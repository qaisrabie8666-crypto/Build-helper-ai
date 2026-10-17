#include "WebSearch.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace geode::prelude;
WebSearch* WebSearch::get() { static WebSearch i; return &i; }
bool WebSearch::isAvailable() {
    return !Mod::get()->getSettingValue<std::string>("tavily-key").empty();
}
void WebSearch::search(const std::string& q,
    std::function<void(std::vector<SearchResult>)> ok,
    std::function<void(std::string)> err)
{
    auto key = Mod::get()->getSettingValue<std::string>("tavily-key");
    if (key.empty()) { err("No Tavily key"); return; }
    json body;
    body["api_key"]=""; body["query"]=q;
    body["search_depth"]="basic"; body["max_results"]=5;
    body["include_answer"]=true;
    body["api_key"]=key;
    auto req = web::WebRequest();
    req.bodyJSON(body);
    auto task = req.post("https://api.tavily.com/search");
    task.listen([ok,err](web::WebResponse* res){
        if(!res||!res->ok()){err("HTTP "+std::to_string(res?res->code():0));return;}
        try {
            auto j=json::parse(res->string().unwrapOr(""));
            std::vector<SearchResult> results;
            if(j.contains("answer")&&!j["answer"].is_null())
                results.push_back({"Direct Answer","",j["answer"].get<std::string>()});
            for(auto& r:j["results"]){
                SearchResult sr;
                sr.title=r.value("title",""); sr.url=r.value("url","");
                sr.content=r.value("content","");
                if(sr.content.size()>300) sr.content=sr.content.substr(0,300)+"...";
                results.push_back(sr);
            }
            ok(results);
        } catch(...){err("Parse error");}
    });
}
std::string WebSearch::formatForPrompt(const std::vector<SearchResult>& results){
    if(results.empty()) return "";
    std::string out="\n\n--- WEB SEARCH RESULTS ---\n";
    for(auto& r:results){
        if(!r.title.empty()) out+="• "+r.title+"\n";
        out+=r.content+"\n\n";
    }
    return out+"--- END ---\n";
}
