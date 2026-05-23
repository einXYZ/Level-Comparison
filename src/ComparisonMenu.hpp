#pragma once
#include "data.hpp"
#include "LevelPickerPopup.hpp"
#include "OptionsPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <functional>

using namespace geode::prelude;

class ComparisonMenu : public geode::Popup {
public:
    static constexpr float c_optionsXOffset = 12.f;
    static constexpr float c_infoXOffset = 0.f;
    static constexpr float c_infoYOffset = 6.f;

    std::function<void(GJGameLevel*, GJGameLevel*, ComparisonConfig)> onCreateCallback;

    GJGameLevel* m_fixedLevel1 = nullptr;
    GJGameLevel* m_pickedLevel1 = nullptr;
    GJGameLevel* m_pickedLevel2 = nullptr;

    CCSprite* m_optSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    CCSprite* m_infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");

    int level1ID = 0;
    int level2ID = 0;
    bool isBuffed = false;
    float sawRotationSpeed = 0.f;
    bool remapGroups = true;
    bool replaceDisappearing = true;

    UnhideOptions unhide = { true, true };
    ModifierOptions modifiers = { false, false, false, false, false };
    ObjectOptions objectOptions = { false, false, false, false, false, false };

    CCMenuItemToggler* unhideToggle = nullptr;
    CCMenuItemToggler* modifiersToggle = nullptr;
    CCMenuItemToggler* objectOptionsToggle = nullptr;
    CCMenuItemToggler* remapToggle = nullptr;
    CCMenuItemToggler* replaceDisappearingToggle = nullptr;
    CCMenuItemSpriteExtra* unhideOptionsBtn = nullptr;
    CCMenuItemSpriteExtra* modifiersOptionsBtn = nullptr;
    CCMenuItemSpriteExtra* objectOptionsBtn = nullptr;

    bool unhideEnabled = true;
    bool modifiersEnabled = false;
    bool objectOptionsEnabled = false;

    static ComparisonMenu* create(
        GJGameLevel* fixedLevel1,
        std::function<void(GJGameLevel*, GJGameLevel*, ComparisonConfig)> onCreate
    );
    bool init() override;

private:
    TextInput* level1Input = nullptr;
    TextInput* level2Input = nullptr;
    std::function<void()> m_applyColors;

    bool* getBoolPtr(const char* key) {
        if (strcmp(key, "is-buffed") == 0) return &isBuffed;
        if (strcmp(key, "unhide-enabled") == 0) return &unhideEnabled;
        if (strcmp(key, "modifiers-enabled") == 0) return &modifiersEnabled;
        if (strcmp(key, "remap-groups") == 0) return &remapGroups;
        if (strcmp(key, "replace-disappearing") == 0) return &replaceDisappearing;
        if (strcmp(key, "object-options-enabled") == 0) return &objectOptionsEnabled;
        if (strcmp(key, "unhide-alpha") == 0) return &unhide.unhideAlpha;
        if (strcmp(key, "unhide-hide") == 0) return &unhide.unhideHide;
        if (strcmp(key, "show-dblocks") == 0) return &modifiers.showDBlocks;
        if (strcmp(key, "show-jblocks") == 0) return &modifiers.showJBlocks;
        if (strcmp(key, "show-sblocks") == 0) return &modifiers.showSBlocks;
        if (strcmp(key, "show-hblocks") == 0) return &modifiers.showHBlocks;
        if (strcmp(key, "show-fblocks") == 0) return &modifiers.showFBlocks;
        if (strcmp(key, "dont-fade") == 0) return &objectOptions.dontFade;
        if (strcmp(key, "dont-enter") == 0) return &objectOptions.dontEnter;
        if (strcmp(key, "no-effects") == 0) return &objectOptions.noEffects;
        if (strcmp(key, "no-glow") == 0) return &objectOptions.noGlow;
        if (strcmp(key, "no-particle") == 0) return &objectOptions.noParticle;
        if (strcmp(key, "no-audio-scale") == 0) return &objectOptions.noAudioScale;
        return nullptr;
    }

    void onToggleBuffedNerfed(CCObject*);
    void onRemap(CCObject*);
    void onReplaceDisappearing(CCObject *);
    void onUnhideToggle(CCObject *);
    void onModifiersToggle(CCObject*);
    void onObjectOptionsToggle(CCObject*);
    void onUnhideOptions(CCObject*);
    void onModifierOptions(CCObject*);
    void onObjectOptions(CCObject*);
    void onSawRotationInfo(CCObject*);
    void onRemapGroupsInfo(CCObject*);
    void onReplaceDisappearingInfo(CCObject *);
    void onUnhideInfo(CCObject *);
    void onModifiersInfo(CCObject*);
    void onObjectOptionsInfo(CCObject*);
    void onPickLevel1(CCObject*);
    void onPickLevel2(CCObject*);
    void onAbort(CCObject*);
    void onCreate(CCObject*);
    void keyBackClicked() override;
};