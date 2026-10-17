#pragma once
#include <Geode/Geode.hpp>
#include "AIManager.hpp"
using namespace geode::prelude;
class DesignPickerLayer : public CCLayer {
protected:
    bool init(const std::vector<DesignVariant>& v, std::function<void(DesignVariant)> cb);
    std::vector<DesignVariant> m_variants;
    std::function<void(DesignVariant)> m_onPick;
    void buildCard(const DesignVariant& v, int i, float y);
    void onPick(CCObject*); void onClose(CCObject*);
public:
    static DesignPickerLayer* create(const std::vector<DesignVariant>& v,
        std::function<void(DesignVariant)> cb);
};
