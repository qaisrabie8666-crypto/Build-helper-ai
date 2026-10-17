#include "AIManager.hpp"
#include "WebSearch.hpp"
#include "TriggerHelper.hpp"
#include <Geode/utils/web.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace geode::prelude;
AIManager* AIManager::get(){ static AIManager i; return &i; }
AIProvider AIManager::getCurrentProvider(){
    auto s=Mod::get()->getSettingValue<std::string>("ai-provider");
    if(s=="gemini") return AIProvider::Gemini;
    if(s=="claude") return AIProvider::Claude;
    if(s=="deepseek") return AIProvider::DeepSeek;
    if(s=="ollama") return AIProvider::Ollama;
    return AIProvider::OpenRouter;
}
std::string AIManager::getProviderName(){
    return Mod::get()->getSettingValue<std::string>("ai-provider");
}
bool AIManager::hasWebSearchSupport(){
    return !Mod::get()->getSettingValue<std::string>("tavily-key").empty();
}
void AIManager::sendMessage(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    switch(getCurrentProvider()){
        case AIProvider::Gemini:     sendGemini(h,sys,ok,err);     break;
        case AIProvider::Claude:     sendClaude(h,sys,ok,err);     break;
        case AIProvider::DeepSeek:   sendDeepSeek(h,sys,ok,err);   break;
        case AIProvider::Ollama:     sendOllama(h,sys,ok,err);     break;
        case AIProvider::OpenRouter: sendOpenRouter(h,sys,ok,err); break;
    }
}
void AIManager::sendWithSearch(const std::vector<AIMessage>& h,const std::string& sys,
    const std::string& q,std::function<void(std::string)> ok,std::function<void(std::string)> err){
    if(!WebSearch::get()->isAvailable()){sendMessage(h,sys,ok,err);return;}
    WebSearch::get()->search(q,[this,h,sys,ok,err](std::vector<SearchResult> res){
        Loader::get()->queueInMainThread([this,h,sys,ok,err,res](){
            sendMessage(h,sys+WebSearch::formatForPrompt(res),ok,err);
        });
    },[this,h,sys,ok,err](std::string){sendMessage(h,sys,ok,err);});
}
void AIManager::requestDesigns(const std::string& desc,
    std::function<void(std::vector<DesignVariant>)> ok,std::function<void(std::string)> err){
    std::string sys=TriggerHelper::getTriggerDocumentation()+
        "\nOutput EXACTLY 3 design variants as JSON array. No prose.\n"
        "Schema: [{\"name\":\"\",\"description\":\"\",\"colorTheme\":\"\",\"songID\":805,\"suggestedBPM\":128,"
        "\"colorChannels\":[{\"channel\":1,\"r\":1.0,\"g\":0.0,\"b\":0.0}],"
        "\"objects\":[{\"id\":1,\"x\":0,\"y\":0,\"rotation\":0,\"scale\":1.0,\"zOrder\":0,\"color1\":1,\"color2\":2,\"groupID\":0}],"
        "\"triggers\":[{\"type\":\"color\",\"x\":0,\"y\":105,\"channel\":3,\"r\":0.1,\"g\":0.1,\"b\":0.3,\"opacity\":1,\"duration\":0}]}]\n"
        "Make 3 DISTINCT styles. 20+ objects each. Include shaders and pulse triggers.";
    std::vector<AIMessage> msgs={{"user","Design a GD level: "+desc}};
    sendWithSearch(msgs,sys,"Geometry Dash level design ideas "+desc,
        [this,ok,err](std::string r){
            Loader::get()->queueInMainThread([this,r,ok,err](){
                auto v=parseDesigns(r);
                if(v.empty()) err("Failed to parse designs");
                else ok(v);
            });
        },err);
}
std::vector<DesignVariant> AIManager::parseDesigns(const std::string& raw){
    std::vector<DesignVariant> variants;
    auto s=raw.find('['),e=raw.rfind(']');
    if(s==std::string::npos||e==std::string::npos) return variants;
    try{
        auto j=json::parse(raw.substr(s,e-s+1));
        for(auto& v:j){
            DesignVariant dv;
            dv.name=v.value("name","Variant");
            dv.description=v.value("description","");
            dv.colorTheme=v.value("colorTheme","");
            dv.songID=v.value("songID",805);
            dv.suggestedBPM=v.value("suggestedBPM",128.f);
            if(v.contains("objects"))
                for(auto& o:v["objects"]){
                    GDObject obj;
                    obj.id=o.value("id",1); obj.x=o.value("x",0.f);
                    obj.y=o.value("y",0.f); obj.rotation=o.value("rotation",0.f);
                    obj.scale=o.value("scale",1.f); obj.zOrder=o.value("zOrder",0);
                    obj.color1=o.value("color1",1); obj.color2=o.value("color2",2);
                    dv.objects.push_back(obj);
                }
            if(v.contains("triggers")){
                GDObject m; m.isTrigger=true;
                m.triggerProps["__json__"]=v["triggers"].dump();
                dv.objects.push_back(m);
            }
            if(v.contains("colorChannels"))
                for(auto& cc:v["colorChannels"]){
                    GDObject m; m.isTrigger=true;
                    m.triggerProps["__cc__"]=cc.dump();
                    dv.objects.push_back(m);
                }
            variants.push_back(dv);
        }
    }catch(...){}
    return variants;
}
void AIManager::sendOpenAICompat(const std::string& url,const std::string& model,
    const std::string& key,const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    json body; body["model"]=model; body["messages"]=json::array();
    if(!sys.empty()) body["messages"].push_back({{"role","system"},{"content",sys}});
    for(auto& m:h) body["messages"].push_back({{"role",m.role},{"content",m.content}});
    auto req=web::WebRequest(); req.bodyJSON(body);
    if(!key.empty()) req.header("Authorization","Bearer "+key);
    auto task=req.post(url);
    task.listen([ok,err,this](web::WebResponse* res){
        if(!res||!res->ok()){err("HTTP "+std::to_string(res?res->code():0));return;}
        try{ok(parseOpenAI(res->string().unwrapOr("")));}catch(...){err("Parse error");}
    });
}
std::string AIManager::parseOpenAI(const std::string& b){
    return json::parse(b)["choices"][0]["message"]["content"].get<std::string>();
}
void AIManager::sendOpenRouter(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    auto key=Mod::get()->getSettingValue<std::string>("api-key");
    auto model=Mod::get()->getSettingValue<std::string>("openrouter-model");
    json body; body["model"]=model; body["messages"]=json::array();
    if(!sys.empty()) body["messages"].push_back({{"role","system"},{"content",sys}});
    for(auto& m:h) body["messages"].push_back({{"role",m.role},{"content",m.content}});
    auto req=web::WebRequest(); req.bodyJSON(body);
    req.header("Authorization","Bearer "+key);
    req.header("HTTP-Referer","https://github.com/qais/build-helper-ai");
    req.header("X-Title","GD Build Helper AI");
    auto task=req.post("https://openrouter.ai/api/v1/chat/completions");
    task.listen([ok,err,this](web::WebResponse* res){
        if(!res||!res->ok()){err("HTTP "+std::to_string(res?res->code():0));return;}
        try{ok(parseOpenAI(res->string().unwrapOr("")));}catch(...){err("Parse error");}
    });
}
void AIManager::sendDeepSeek(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    sendOpenAICompat("https://api.deepseek.com/v1/chat/completions","deepseek-chat",
        Mod::get()->getSettingValue<std::string>("api-key"),h,sys,ok,err);
}
void AIManager::sendOllama(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    sendOpenAICompat(
        Mod::get()->getSettingValue<std::string>("ollama-url")+"/api/chat",
        Mod::get()->getSettingValue<std::string>("ollama-model"),
        "",h,sys,ok,err);
}
void AIManager::sendGemini(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    auto key=Mod::get()->getSettingValue<std::string>("api-key");
    json body;
    if(!sys.empty()) body["system_instruction"]["parts"][0]["text"]=sys;
    body["contents"]=json::array();
    for(auto& m:h) body["contents"].push_back({
        {"role",m.role=="assistant"?"model":"user"},
        {"parts",{{{"text",m.content}}}}});
    auto req=web::WebRequest(); req.bodyJSON(body);
    auto task=req.post("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key="+key);
    task.listen([ok,err,this](web::WebResponse* res){
        if(!res||!res->ok()){err("HTTP "+std::to_string(res?res->code():0));return;}
        try{ok(parseGemini(res->string().unwrapOr("")));}catch(...){err("Parse error");}
    });
}
std::string AIManager::parseGemini(const std::string& b){
    return json::parse(b)["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
}
void AIManager::sendClaude(const std::vector<AIMessage>& h,const std::string& sys,
    std::function<void(std::string)> ok,std::function<void(std::string)> err){
    auto key=Mod::get()->getSettingValue<std::string>("api-key");
    json body; body["model"]="claude-3-5-sonnet-20241022";
    body["max_tokens"]=4096; body["system"]=sys; body["messages"]=json::array();
    for(auto& m:h) body["messages"].push_back({{"role",m.role},{"content",m.content}});
    auto req=web::WebRequest(); req.bodyJSON(body);
    req.header("x-api-key",key); req.header("anthropic-version","2023-06-01");
    auto task=req.post("https://api.anthropic.com/v1/messages");
    task.listen([ok,err,this](web::WebResponse* res){
        if(!res||!res->ok()){err("HTTP "+std::to_string(res?res->code():0));return;}
        try{ok(parseClaude(res->string().unwrapOr("")));}catch(...){err("Parse error");}
    });
}
std::string AIManager::parseClaude(const std::string& b){
    return json::parse(b)["content"][0]["text"].get<std::string>();
}
