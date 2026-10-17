#include "TriggerHelper.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
using json = nlohmann::json;
using namespace geode::prelude;

std::string TriggerHelper::buildTriggerString(const TriggerParams& p){
    std::ostringstream s;
    int id=(int)p.type;
    if(p.type==TriggerType::Shader){
        if(p.shaderType=="blur")       id=3006;
        else if(p.shaderType=="pixelate") id=3007;
        else if(p.shaderType=="grayscale") id=3008;
        else if(p.shaderType=="sepia")  id=3009;
        else id=2999;
    }
    s<<"1,"<<id<<",2,"<<(int)p.x<<",3,"<<(int)p.y;
    switch(p.type){
        case TriggerType::Color:
            s<<",23,"<<p.targetChannel<<",7,"<<(int)(p.r*255)
             <<",8,"<<(int)(p.g*255)<<",9,"<<(int)(p.b*255)
             <<",35,"<<p.opacity<<",10,"<<p.duration; break;
        case TriggerType::Move:
            s<<",28,"<<(int)p.moveX<<",29,"<<(int)p.moveY
             <<",51,"<<p.targetGroup<<",10,"<<p.duration; break;
        case TriggerType::Alpha:
            s<<",35,"<<p.alphaVal<<",51,"<<p.targetGroup
             <<",10,"<<p.duration; break;
        case TriggerType::Pulse:
            s<<",51,"<<p.targetGroup
             <<",7,"<<(int)(p.pulseR*255)<<",8,"<<(int)(p.pulseG*255)
             <<",9,"<<(int)(p.pulseB*255)
             <<",46,"<<p.pulseHold<<",47,"<<p.pulseFade; break;
        case TriggerType::Song:
            s<<",35,"<<p.songID<<",36,"<<p.songSpeed
             <<",122,"<<(p.songLoop?1:0); break;
        case TriggerType::Keyframe:
            s<<",51,"<<p.keyframeGroup<<",10,"<<p.duration; break;
        case TriggerType::Shader:
            s<<",35,"<<p.shaderStrength; break;
    }
    s<<";"; return s.str();
}

bool TriggerHelper::placeTrigger(const TriggerParams& p){
    auto* e=LevelEditorLayer::get(); if(!e) return false;
    auto s=buildTriggerString(p);
    auto* o=e->createObjectFromString(s.c_str(),true,false);
    if(!o) return false;
    e->addObject(o,true,false); return true;
}

bool TriggerHelper::setColorChannel(int ch,float r,float g,float b,float a){
    TriggerParams p; p.type=TriggerType::Color;
    p.targetChannel=ch; p.r=r; p.g=g; p.b=b; p.opacity=a;
    p.duration=0; p.x=0; p.y=105;
    return placeTrigger(p);
}

std::vector<TriggerParams> TriggerHelper::parseTriggers(const std::string& jsonStr){
    std::vector<TriggerParams> result;
    try {
        auto j=json::parse(jsonStr);
        for(auto& t:j){
            TriggerParams p;
            std::string type=t.value("type","color");
            p.x=t.value("x",0.f); p.y=t.value("y",105.f);
            if(type=="color"){
                p.type=TriggerType::Color;
                p.targetChannel=t.value("channel",1);
                p.r=t.value("r",1.f); p.g=t.value("g",1.f); p.b=t.value("b",1.f);
                p.opacity=t.value("opacity",1.f); p.duration=t.value("duration",0.f);
            } else if(type=="move"){
                p.type=TriggerType::Move;
                p.moveX=t.value("moveX",0.f); p.moveY=t.value("moveY",0.f);
                p.targetGroup=t.value("targetGroup",0); p.duration=t.value("duration",0.f);
            } else if(type=="alpha"){
                p.type=TriggerType::Alpha;
                p.alphaVal=t.value("alpha",1.f);
                p.targetGroup=t.value("targetGroup",0); p.duration=t.value("duration",0.f);
            } else if(type=="pulse"){
                p.type=TriggerType::Pulse;
                p.targetGroup=t.value("targetGroup",0);
                p.pulseR=t.value("r",1.f); p.pulseG=t.value("g",1.f); p.pulseB=t.value("b",1.f);
                p.pulseHold=t.value("hold",0.3f); p.pulseFade=t.value("fade",0.3f);
            } else if(type=="song"){
                p.type=TriggerType::Song;
                p.songID=t.value("songID",0); p.songSpeed=t.value("speed",1.f);
                p.songLoop=t.value("loop",false);
            } else if(type=="keyframe"){
                p.type=TriggerType::Keyframe;
                p.keyframeGroup=t.value("group",0); p.duration=t.value("duration",1.f);
            } else if(type=="shader"){
                p.type=TriggerType::Shader;
                p.shaderType=t.value("shader","chromatic");
                p.shaderStrength=t.value("strength",1.f);
            }
            result.push_back(p);
        }
    } catch(...){}
    return result;
}

std::string TriggerHelper::getTriggerDocumentation(){ return R"(
=== GD 2.208 TRIGGERS ===
COLOR: {"type":"color","x":0,"y":105,"channel":1,"r":1.0,"g":0.5,"b":0.0,"opacity":1.0,"duration":0.5}
MOVE:  {"type":"move","x":100,"y":105,"moveX":150,"moveY":0,"targetGroup":1,"duration":0.5}
ALPHA: {"type":"alpha","x":200,"y":105,"alpha":0.5,"targetGroup":2,"duration":0.3}
PULSE: {"type":"pulse","x":300,"y":105,"targetGroup":3,"r":1.0,"g":0.2,"b":0.8,"hold":0.3,"fade":0.4}
SONG:  {"type":"song","x":400,"y":105,"songID":805,"speed":1.0,"loop":false}
KEYFRAME: {"type":"keyframe","x":500,"y":105,"group":4,"duration":1.0}
SHADER: {"type":"shader","x":600,"y":105,"shader":"chromatic","strength":0.8}
  shaders: chromatic | blur | pixelate | grayscale | sepia
COLOR CHANNELS: use channel 1-4 for main level colors.
PULSE SYNC: place pulse triggers every beat. 1 beat at normal speed = 30 GD x-units * (128/BPM)
=== END ===
)"; }
