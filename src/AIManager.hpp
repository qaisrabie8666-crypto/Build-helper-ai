#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <string>
#include <vector>
#include <map>
using namespace geode::prelude;
enum class AIProvider { OpenRouter, Gemini, Claude, DeepSeek, Ollama };
struct AIMessage { std::string role, content; };
struct GDObject {
    int id=1; float x=0,y=0,rotation=0,scale=1; int zOrder=0,color1=1,color2=2,groupID=0;
    bool isTrigger=false;
    std::map<std::string,std::string> triggerProps;
};
struct DesignVariant {
    std::string name,description,colorTheme;
    int songID=805; float suggestedBPM=128;
    std::vector<GDObject> objects;
};
class AIManager {
public:
    static AIManager* get();
    void sendMessage(const std::vector<AIMessage>& h, const std::string& sys,
        std::function<void(std::string)> ok, std::function<void(std::string)> err);
    void sendWithSearch(const std::vector<AIMessage>& h, const std::string& sys,
        const std::string& q,
        std::function<void(std::string)> ok, std::function<void(std::string)> err);
    void requestDesigns(const std::string& desc,
        std::function<void(std::vector<DesignVariant>)> ok,
        std::function<void(std::string)> err);
    AIProvider getCurrentProvider();
    std::string getProviderName();
    bool hasWebSearchSupport();
private:
    void sendOpenAICompat(const std::string& url, const std::string& model,
        const std::string& key, const std::vector<AIMessage>& h, const std::string& sys,
        std::function<void(std::string)> ok, std::function<void(std::string)> err);
    void sendGemini(const std::vector<AIMessage>&,const std::string&,std::function<void(std::string)>,std::function<void(std::string)>);
    void sendClaude(const std::vector<AIMessage>&,const std::string&,std::function<void(std::string)>,std::function<void(std::string)>);
    void sendOpenRouter(const std::vector<AIMessage>&,const std::string&,std::function<void(std::string)>,std::function<void(std::string)>);
    void sendDeepSeek(const std::vector<AIMessage>&,const std::string&,std::function<void(std::string)>,std::function<void(std::string)>);
    void sendOllama(const std::vector<AIMessage>&,const std::string&,std::function<void(std::string)>,std::function<void(std::string)>);
    std::string parseOpenAI(const std::string& b);
    std::string parseGemini(const std::string& b);
    std::string parseClaude(const std::string& b);
    std::vector<DesignVariant> parseDesigns(const std::string& j);
};
