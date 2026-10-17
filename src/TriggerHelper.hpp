#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include <map>
using namespace geode::prelude;
enum class TriggerType { Color=899, Move=901, Alpha=1007, Pulse=1006, Song=1268, Keyframe=3033, Shader=2999 };
struct TriggerParams {
    TriggerType type = TriggerType::Color;
    float x=0,y=105;
    int targetChannel=1, targetGroup=0;
    float r=1,g=1,b=1,opacity=1,duration=0;
    float moveX=0,moveY=0;
    float alphaVal=1;
    int songID=0; float songSpeed=1; bool songLoop=false;
    float pulseR=1,pulseG=1,pulseB=1,pulseHold=0.3f,pulseFade=0.3f;
    int keyframeGroup=0;
    std::string shaderType="chromatic"; float shaderStrength=1;
};
class TriggerHelper {
public:
    static bool placeTrigger(const TriggerParams& p);
    static bool setColorChannel(int ch, float r, float g, float b, float a=1);
    static std::string buildTriggerString(const TriggerParams& p);
    static std::vector<TriggerParams> parseTriggers(const std::string& json);
    static std::string getTriggerDocumentation();
};
