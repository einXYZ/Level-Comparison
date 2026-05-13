#pragma once
#include "data.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <functional>

using namespace geode::prelude;

class ComparisonMenu : public geode::Popup {
public:
    std::function<void(int, bool, float, bool, bool, bool)> onCreateCallback;

    int targetLevelID = 0;
    bool isBuffed = false;
    float sawRotationSpeed = 0.f;
    bool remapGroups = true;
    bool unhideObjects = true;
    bool showModifiers = false;

    CCLabelBMFont* speedLabel = nullptr;
    CCMenuItemToggler* buffedToggle = nullptr;
    CCMenuItemToggler* nerfedToggle = nullptr;
    CCMenuItemToggler* remapToggle = nullptr;
    CCMenuItemToggler* unhideToggle = nullptr;
    CCMenuItemToggler* showModifiersToggle = nullptr;

    static ComparisonMenu* create(std::function<void(int, bool, float, bool, bool, bool)> onCreate);
    bool init() override;

private:
    void onBuffed(CCObject*);
    void onNerfed(CCObject*);
    void onRemap(CCObject*);
    void onUnhide(CCObject*);
    void onShowModifiers(CCObject*);
    void onBuffedNerfedInfo(CCObject*);
    void onSawRotationInfo(CCObject*);
    void onRemapGroupsInfo(CCObject*);
    void onUnhideInfo(CCObject*);
    void onShowModifiersInfo(CCObject*);
    void onComingSoonInfo(CCObject*);
    void onAbort(CCObject*);
    void onCreate(CCObject*);
    void keyBackClicked() override;
};
