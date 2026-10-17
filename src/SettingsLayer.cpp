#include <Geode/Geode.hpp>
using namespace geode::prelude;
class SettingsLayer : public CCLayer {
    CCTextInputNode *m_key,*m_tav,*m_model,*m_ollama;
protected:
    bool init(){
        if(!CCLayer::init()) return false;
        auto W=CCDirector::get()->getWinSize();
        addChild(CCLayerColor::create({0,0,0,155}));
        auto bg=CCScale9Sprite::create("GJ_square04.png");
        bg->setContentSize({340,300}); bg->setPosition(W/2); addChild(bg);
        auto t=CCLabelBMFont::create("AI Settings","goldFont.fnt");
        t->setScale(0.56f); t->setPosition({W.width/2,W.height/2+130}); addChild(t);
        auto mk=[&](const char* lbl,const char* hint,float yo)->CCTextInputNode*{
            auto l=CCLabelBMFont::create(lbl,"bigFont.fnt");
            l->setScale(0.34f); l->setPosition({W.width/2-128,W.height/2+yo+14}); addChild(l);
            auto ti=CCTextInputNode::create(258,26,hint,"chatFont.fnt");
            ti->setMaxLabelScale(0.46f); ti->setPosition({W.width/2,W.height/2+yo}); addChild(ti);
            return ti;
        };
        auto g=[](const char* k){return Mod::get()->getSettingValue<std::string>(k);};
        m_key=mk("API Key:","Enter key...",80);    m_key->setString(g("api-key").c_str());
        m_tav=mk("Tavily Key:","tavily.com",42);   m_tav->setString(g("tavily-key").c_str());
        m_model=mk("OR Model:","openai/gpt-4o",4); m_model->setString(g("openrouter-model").c_str());
        m_ollama=mk("Ollama URL:","192.168.x:11434",-34); m_ollama->setString(g("ollama-url").c_str());
        auto mkb=[&](const char* l,SEL_MenuHandler s,float x){
            auto b=CCScale9Sprite::create("GJ_button_01.png"); b->setContentSize({100,28});
            auto lb=CCLabelBMFont::create(l,"bigFont.fnt"); lb->setScale(0.40f);
            lb->setPosition({50,14}); b->addChild(lb);
            auto btn=CCMenuItemSpriteExtra::create(b,this,s); btn->setPositionX(x); return btn;
        };
        auto m=CCMenu::create(
            mkb("Save",menu_selector(SettingsLayer::onSave),-55),
            mkb("Close",menu_selector(SettingsLayer::onClose),55),nullptr);
        m->setPosition({W.width/2,W.height/2-118}); addChild(m);
        return true;
    }
    void onSave(CCObject*){
        auto s=[](const char* k,CCTextInputNode* n){
            Mod::get()->setSettingValue<std::string>(k,std::string(n->getString()));};
        s("api-key",m_key); s("tavily-key",m_tav);
        s("openrouter-model",m_model); s("ollama-url",m_ollama);
        removeFromParent();
    }
    void onClose(CCObject*){removeFromParent();}
public:
    static SettingsLayer* create(){
        auto r=new SettingsLayer();
        if(r&&r->init()){r->autorelease();return r;}
        CC_SAFE_DELETE(r);return nullptr;
    }
};
