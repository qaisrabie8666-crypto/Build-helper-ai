#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "BuildHelperLayer.hpp"
using namespace geode::prelude;
class $modify(MyEditorUI,EditorUI){
    bool init(LevelEditorLayer* lel){
        if(!EditorUI::init(lel)) return false;
        auto W=CCDirector::get()->getWinSize();
        auto base=CCSprite::createWithSpriteFrameName("GJ_editModeBtn_001.png");
        base->setScale(0.70f);
        auto badge=CCLabelBMFont::create("AI","chatFont.fnt");
        badge->setScale(0.40f); badge->setColor({60,210,255});
        badge->setPosition({
            base->getContentSize().width*0.74f,
            base->getContentSize().height*0.74f});
        base->addChild(badge,5);
        auto btn=CCMenuItemSpriteExtra::create(
            base,this,menu_selector(MyEditorUI::onOpenHelper));
        auto m=CCMenu::create(btn,nullptr);
        m->setPosition({W.width-32.f,W.height-32.f});
        addChild(m,10);
        return true;
    }
    void onOpenHelper(CCObject*){
        auto l=BuildHelperLayer::create();
        CCDirector::get()->getRunningScene()->addChild(l,200);
    }
};
$execute{ log::info("Build Helper AI v1.0 loaded"); }
