#pragma once
#include <Geode/Geode.hpp>
#include "AIManager.hpp"
using namespace geode::prelude;
enum class HelperMode { Chat, Plan, Build };
class BuildHelperLayer : public CCLayer {
protected:
    bool init() override;
    CCMenu* m_modeMenu=nullptr;
    CCTextInputNode* m_inputField=nullptr;
    CCNode* m_chatContainer=nullptr;
    CCScrollLayerExt* m_chatScroll=nullptr;
    CCLabelBMFont* m_statusLabel=nullptr;
    CCMenuItemSpriteExtra* m_sendBtn=nullptr;
    CCNode* m_buildPanel=nullptr;
    CCTextInputNode *m_levelIdInput=nullptr,*m_decorInput=nullptr,*m_songInput=nullptr;
    float m_chatYOffset=10.f;
    HelperMode m_mode=HelperMode::Chat;
    std::vector<AIMessage> m_history;
    bool m_loading=false;
    std::string m_activePlan;
    void setupUI(); void setupModeBar(); void setupChatArea();
    void setupInputRow(); void setupBuildPanel();
    void onModeChat(CCObject*); void onModePlan(CCObject*); void onModeBuild(CCObject*);
    void onSend(CCObject*); void onPickDesign(CCObject*);
    void onExecuteBuild(CCObject*); void onSettings(CCObject*); void onClose(CCObject*);
    void sendToAI(const std::string& msg);
    void handleAIResponse(const std::string& r);
    void applyBuildResponse(const std::string& json);
    void applyDesignVariant(const DesignVariant& v);
    void addBubble(const std::string& text, bool isUser);
    void setLoading(bool v);
    void setStatus(const std::string& t, ccColor3B c={150,220,150});
    std::string buildSystemPrompt();
public:
    static BuildHelperLayer* create();
};
