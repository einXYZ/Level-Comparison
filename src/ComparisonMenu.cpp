#include "ComparisonMenu.hpp"
#include "utils.hpp"

#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;
using namespace lc;

ComparisonMenu* ComparisonMenu::create(
    GJGameLevel* fixedLevel1,
    std::function<void(GJGameLevel*, GJGameLevel*, ComparisonConfig)> onCreate
) {
    auto ret = new ComparisonMenu();
    ret->m_fixedLevel1 = fixedLevel1;
    if (ret && ret->init()) {
        ret->onCreateCallback = onCreate;
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ComparisonMenu::init() {
    if (!Popup::init(380.f, 280.f)) return false;
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    m_optSpr->setScale(0.6f);
    m_infoSpr->setScale(0.5f);

    auto mod = Mod::get();
    level1ID = mod->getSavedValue<int>("source-level-id", 0);
    level2ID = mod->getSavedValue<int>("target-level-id", 0);
    sawRotationSpeed = mod->getSavedValue<float>("saw-rotation-speed", 0.f);
    for (auto& b : savedBools) *getBoolPtr(b.key) = mod->getSavedValue<bool>(b.key, b.defaultValue);

    bool level1IsEditor = mod->getSavedValue<bool>("source-level-editor", false);
    bool level2IsEditor = mod->getSavedValue<bool>("target-level-editor", false);

    auto findEditorLevel = [](int editorID) -> GJGameLevel* {
        for (auto lvl : CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels)) {
            if (EditorIDs::getID(lvl, false) == editorID) return lvl;
        }
        return nullptr;
    };

    auto panel = CCLayerColor::create({ 0, 0, 0, 0 });
    panel->setContentSize({ 380.f, 280.f });
    panel->setID("create-comparison-background"_spr);
    this->m_mainLayer->addChildAtPosition(panel, Anchor::BottomLeft);

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    panel->addChild(menu);

    auto title = CCLabelBMFont::create("Level Comparison", "goldFont.fnt");
    title->setPosition({ 190.f, 265.f });
    panel->addChild(title);

    auto level1Label = CCLabelBMFont::create("Level 1", "goldFont.fnt");
    level1Label->setPosition({ 80.f, 240.f });
    level1Label->setScale(0.8f);
    panel->addChild(level1Label);

    level1Input = TextInput::create(120.f, "Level ID", "bigFont.fnt");
    level1Input->setPosition({ 80.f, 215.f });

    if (m_fixedLevel1) {
        bool isEditor = m_fixedLevel1->m_levelType == GJLevelType::Editor;
        level1Input->setString(isEditor
            ? fmt::format("({})", EditorIDs::getID(m_fixedLevel1)).c_str()
            : std::to_string(m_fixedLevel1->m_levelID).c_str()
        );
        level1Input->setEnabled(false);
    } else {
        level1Input->setMaxCharCount(10);
        level1Input->setFilter("0123456789");
        level1Input->setEnabled(true);
        level1Input->setCallback([this](std::string const& text) {
            level1ID = text.empty() ? 0 : lc::stoi(const_cast<std::string&>(text));
            m_pickedLevel1 = nullptr;
        });

        if (level1IsEditor && level1ID != 0) {
            m_pickedLevel1 = findEditorLevel(level1ID);
            level1Input->setEnabled(false);
            level1Input->setString(fmt::format("({})", level1ID).c_str());
        } else 
            level1Input->setString(level1ID != 0 ? std::to_string(level1ID).c_str() : "");

        auto pickSpr1 = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        pickSpr1->setScale(0.5f);
        auto pickBtn1 = CCMenuItemSpriteExtra::create(pickSpr1, this, menu_selector(ComparisonMenu::onPickLevel1));
        pickBtn1->setPosition({ 152.f, 215.f });
        menu->addChild(pickBtn1);
    }
    panel->addChild(level1Input);

    auto level2Label = CCLabelBMFont::create("Level 2", "goldFont.fnt");
    level2Label->setPosition({ 80.f, 190.f });
    level2Label->setScale(0.8f);
    panel->addChild(level2Label);

    level2Input = TextInput::create(120.f, "Level ID", "bigFont.fnt");
    level2Input->setMaxCharCount(10);
    level2Input->setFilter("0123456789");
    level2Input->setPosition({ 80.f, 165.f });
    level2Input->setID("level-id-input"_spr);
    level2Input->setCallback([this](std::string const& text) {
        level2ID = text.empty() ? 0 : lc::stoi(const_cast<std::string&>(text));
        m_pickedLevel2 = nullptr;
    });

    if (level2IsEditor && level2ID != 0) {
        m_pickedLevel2 = findEditorLevel(level2ID);
        level2Input->setEnabled(false);
        level2Input->setString(fmt::format("({})", level2ID).c_str());
    } else {
        level2Input->setEnabled(true);
        level2Input->setString(level2ID != 0 ? std::to_string(level2ID).c_str() : "");
    }
    panel->addChild(level2Input);

    m_applyColors = [this]() {
        ccColor3B buffedColor = { 255, 80, 80 };
        ccColor3B nerfedColor = { 80, 160, 255 };
        level1Input->getInputNode()->setLabelPlaceholderColor(isBuffed ? buffedColor : nerfedColor);
        level1Input->getInputNode()->setLabelNormalColor(isBuffed ? buffedColor : nerfedColor);
        level2Input->getInputNode()->setLabelPlaceholderColor(isBuffed ? nerfedColor : buffedColor);
        level2Input->getInputNode()->setLabelNormalColor(isBuffed ? nerfedColor : buffedColor);
    };
    m_applyColors();

    auto pickSpr2 = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    pickSpr2->setScale(0.5f);
    auto pickBtn2 = CCMenuItemSpriteExtra::create(pickSpr2, this, menu_selector(ComparisonMenu::onPickLevel2));
    pickBtn2->setPosition({ 152.f, 165.f });
    menu->addChild(pickBtn2);

    auto flipSpr = CircleButtonSprite::createWithSpriteFrameName("edit_flipYBtn_001.png", 0.8f, CircleBaseColor::Green, CircleBaseSize::Small);
    flipSpr->setScale(0.5f);
    auto buffedNerfedBtn = CCMenuItemSpriteExtra::create(flipSpr, this, menu_selector(ComparisonMenu::onToggleBuffedNerfed));
    buffedNerfedBtn->setPosition({ 152.f, 190.f });
    menu->addChild(buffedNerfedBtn);

    auto speedText = CCLabelBMFont::create("Saw Rotation", "goldFont.fnt");
    speedText->setPosition({ 80.f, 135.f });
    speedText->setScale(0.8f);
    panel->addChild(speedText);

    auto sawRotationInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onSawRotationInfo));
    float sawLabelRight = 80.f + (speedText->getContentSize().width * 0.8f * 0.5f);
    sawRotationInfo->setPosition({ sawLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, speedText->getPositionY() + c_infoYOffset });
    menu->addChild(sawRotationInfo);

    auto sawSpeedInput = TextInput::create(80.f, "0", "bigFont.fnt");
    sawSpeedInput->setMaxCharCount(3);
    sawSpeedInput->setFilter("0123456789-");
    sawSpeedInput->setPosition({ 80.f, 107.f });
    sawSpeedInput->setEnabled(true);
    sawSpeedInput->setID("saw-speed-input"_spr);
    sawSpeedInput->setString(std::to_string(static_cast<int>(sawRotationSpeed)).c_str());
    sawSpeedInput->setCallback([this](std::string const& text) {
        if (text.empty() || text == "-") return;
        if (text.find('-', 1) != std::string::npos) return;
        sawRotationSpeed = lc::stof(const_cast<std::string&>(text));
    });
    panel->addChild(sawSpeedInput);

    unhideToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ComparisonMenu::onUnhideToggle), 0.8f
    );
    unhideToggle->setPosition({ 190.f, 220.f });
    unhideToggle->toggle(unhideEnabled);
    menu->addChild(unhideToggle);

    auto unhideLabel = CCLabelBMFont::create("Unhide", "goldFont.fnt");
    unhideLabel->setPosition({ 208.f, 220.f });
    unhideLabel->setScale(0.7f);
    unhideLabel->setAnchorPoint({ 0.f, 0.5f });
    panel->addChild(unhideLabel);

    unhideOptionsBtn = CCMenuItemSpriteExtra::create(m_optSpr, this, menu_selector(ComparisonMenu::onUnhideOptions));
    float unhideLabelRight = 208.f + (unhideLabel->getContentSize().width * 0.7f);
    unhideOptionsBtn->setPosition({ unhideLabelRight + c_optionsXOffset + m_optSpr->getContentSize().width * 0.6f * 0.5f, 220.f });
    unhideOptionsBtn->setVisible(unhideEnabled);
    menu->addChild(unhideOptionsBtn);

    auto unhideInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onUnhideInfo));
    unhideInfo->setPosition({ unhideLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, unhideLabel->getPositionY() + c_infoYOffset });
    menu->addChild(unhideInfo);

    modifiersToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ComparisonMenu::onModifiersToggle), 0.8f
    );
    modifiersToggle->setPosition({ 190.f, 183.f });
    modifiersToggle->toggle(modifiersEnabled);
    menu->addChild(modifiersToggle);

    auto modifiersLabel = CCLabelBMFont::create("Modifiers", "goldFont.fnt");
    modifiersLabel->setPosition({ 208.f, 183.f });
    modifiersLabel->setScale(0.7f);
    modifiersLabel->setAnchorPoint({ 0.f, 0.5f });
    panel->addChild(modifiersLabel);

    modifiersOptionsBtn = CCMenuItemSpriteExtra::create(m_optSpr, this, menu_selector(ComparisonMenu::onModifierOptions));
    float modifiersLabelRight = 208.f + (modifiersLabel->getContentSize().width * 0.7f);
    modifiersOptionsBtn->setPosition({ modifiersLabelRight + c_optionsXOffset + m_optSpr->getContentSize().width * 0.6f * 0.5f, 183.f });
    modifiersOptionsBtn->setVisible(modifiersEnabled);
    menu->addChild(modifiersOptionsBtn);

    auto modifiersInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onModifiersInfo));
    modifiersInfo->setPosition({ modifiersLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, modifiersLabel->getPositionY() + c_infoYOffset });
    menu->addChild(modifiersInfo);

    objectOptionsToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ComparisonMenu::onObjectOptionsToggle), 0.8f
    );
    objectOptionsToggle->setPosition({ 190.f, 146.f });
    objectOptionsToggle->toggle(objectOptionsEnabled);
    menu->addChild(objectOptionsToggle);

    auto objectOptionsLabel = CCLabelBMFont::create("Obj. Options", "goldFont.fnt");
    objectOptionsLabel->setPosition({ 208.f, 146.f });
    objectOptionsLabel->setScale(0.7f);
    objectOptionsLabel->setAnchorPoint({ 0.f, 0.5f });
    panel->addChild(objectOptionsLabel);

    objectOptionsBtn = CCMenuItemSpriteExtra::create(m_optSpr, this, menu_selector(ComparisonMenu::onObjectOptions));
    float objectOptionsLabelRight = 208.f + (objectOptionsLabel->getContentSize().width * 0.7f);
    objectOptionsBtn->setPosition({ objectOptionsLabelRight + c_optionsXOffset + m_optSpr->getContentSize().width * 0.6f * 0.5f, 146.f });
    objectOptionsBtn->setVisible(objectOptionsEnabled);
    menu->addChild(objectOptionsBtn);

    auto objectOptionsInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onObjectOptionsInfo));
    objectOptionsInfo->setPosition({ objectOptionsLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, objectOptionsLabel->getPositionY() + c_infoYOffset });
    menu->addChild(objectOptionsInfo);

    remapToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ComparisonMenu::onRemap), 0.8f
    );
    remapToggle->setPosition({ 190.f, 109.f });
    remapToggle->toggle(remapGroups);
    menu->addChild(remapToggle);

    auto remapLabel = CCLabelBMFont::create("Remap groups", "goldFont.fnt");
    remapLabel->setPosition({ 208.f, 109.f });
    remapLabel->setScale(0.7f);
    remapLabel->setAnchorPoint({ 0.f, 0.5f });
    panel->addChild(remapLabel);

    auto remapInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onRemapGroupsInfo));
    float remapLabelRight = 208.f + (remapLabel->getContentSize().width * 0.7f);
    remapInfo->setPosition({ remapLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, remapLabel->getPositionY() + c_infoYOffset });
    menu->addChild(remapInfo);

    replaceDisappearingToggle = CCMenuItemToggler::createWithStandardSprites(
    this, menu_selector(ComparisonMenu::onReplaceDisappearing), 0.8f
    );
    replaceDisappearingToggle->setPosition({ 190.f, 72.f });
    replaceDisappearingToggle->toggle(replaceDisappearing);
    menu->addChild(replaceDisappearingToggle);

    auto replaceDisappearingLabel = CCLabelBMFont::create("Replace Disappearing", "goldFont.fnt");
    replaceDisappearingLabel->setPosition({ 208.f, 72.f });
    replaceDisappearingLabel->setScale(0.55f);
    replaceDisappearingLabel->setAnchorPoint({ 0.f, 0.5f });
    panel->addChild(replaceDisappearingLabel);

    auto replaceDisappearingInfo = CCMenuItemSpriteExtra::create(m_infoSpr, this, menu_selector(ComparisonMenu::onReplaceDisappearingInfo));
    float replaceDisappearingLabelRight = 208.f + (replaceDisappearingLabel->getContentSize().width * 0.55f);
    replaceDisappearingInfo->setPosition({ replaceDisappearingLabelRight + c_infoXOffset + m_infoSpr->getContentSize().width * 0.5f * 0.5f, replaceDisappearingLabel->getPositionY() + c_infoYOffset });
    menu->addChild(replaceDisappearingInfo);

    auto abortBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Abort"), this, menu_selector(ComparisonMenu::onAbort)
    );
    abortBtn->setPosition({ 100.f, 20.f });
    menu->addChild(abortBtn);

    auto createBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Create"), this, menu_selector(ComparisonMenu::onCreate)
    );
    createBtn->setPosition({ 280.f, 20.f });
    menu->addChild(createBtn);

    return true;
}

void ComparisonMenu::onToggleBuffedNerfed(CCObject*) {
    isBuffed = !isBuffed;
    if (m_applyColors) m_applyColors();
}

void ComparisonMenu::onUnhideToggle(CCObject*) {
    unhideEnabled = !unhideToggle->m_toggled;
    unhideOptionsBtn->setVisible(unhideEnabled);
}

void ComparisonMenu::onModifiersToggle(CCObject*) {
    modifiersEnabled = !modifiersToggle->m_toggled;
    modifiersOptionsBtn->setVisible(modifiersEnabled);
}

void ComparisonMenu::onObjectOptionsToggle(CCObject*) {
    objectOptionsEnabled = !objectOptionsToggle->m_toggled;
    objectOptionsBtn->setVisible(objectOptionsEnabled);
}

void ComparisonMenu::onRemap(CCObject*) {
    remapGroups = !remapToggle->m_toggled;
}

void ComparisonMenu::onReplaceDisappearing(CCObject*) {
    replaceDisappearing = !replaceDisappearingToggle->m_toggled;
}

void ComparisonMenu::onUnhideOptions(CCObject*) {
    OptionsPopup::create({
        "Unhide", {
            { "Alpha triggers", &unhide.unhideAlpha, "Deletes all <cy>alpha triggers</c>." },
            { "Hide checkbox", &unhide.unhideHide, "Removes the <cy>Hide</c> option from every object." },
        },
    }, nullptr)->show();
}

void ComparisonMenu::onModifierOptions(CCObject*) {
    OptionsPopup::create({
        "Show Modifiers", {
            { "D-Blocks", &modifiers.showDBlocks, "" },
            { "J-Blocks", &modifiers.showJBlocks, "" },
            { "S-Blocks", &modifiers.showSBlocks, "" },
            { "H-Blocks", &modifiers.showHBlocks, "" },
            { "F-Blocks", &modifiers.showFBlocks, "" },
        },
    }, nullptr)->show();
}

void ComparisonMenu::onObjectOptions(CCObject*) {
    OptionsPopup::create({
        "Object Options", {
            { "Dont Fade", &objectOptions.dontFade, "Applies <cy>Dont Fade</c> on all objects." },
            { "Dont Enter", &objectOptions.dontEnter, "Applies <cy>Dont Enter</c> on all objects." },
            { "No Effects", &objectOptions.noEffects, "Removes all <cy>portal effects</c>." },
            { "No Glow", &objectOptions.noGlow, "Removes <cy>glow</c> from all objects." },
            { "No Particle", &objectOptions.noParticle,"Removes <cy>particle effects</c> from portals and orbs." },
            { "No Audio Scale", &objectOptions.noAudioScale, "Disables <cy>orb pulsing</c>." },
        },
    }, nullptr)->show();
}

void ComparisonMenu::onSawRotationInfo(CCObject*) {
    FLAlertLayer::create(
        "Saw Rotation",
        "Sets the rotation speed of <cy>saw blade objects</c>. Set to <cg>0</c> to disable saw rotation.",
        "OK"
    )->show();
}

void ComparisonMenu::onUnhideInfo(CCObject*) {
    FLAlertLayer::create(
        "Unhide",
        "Makes <cy>invisible objects</c> from alpha triggers and hide option visible again.",
        "OK"
    )->show();
}

void ComparisonMenu::onModifiersInfo(CCObject*) {
    FLAlertLayer::create(
        "Modifiers",
        "Visually displays <cy>modifier blocks</c> such as D-, J-, S-, H-, and F-Blocks.",
        "OK"
    )->show();
}

void ComparisonMenu::onObjectOptionsInfo(CCObject*) {
    FLAlertLayer::create(
        "Object Options",
        "Applies <cy>object options</c> to all objects.",
        "OK"
    )->show();
}

void ComparisonMenu::onRemapGroupsInfo(CCObject*) {
    FLAlertLayer::create(
        "Remap Groups",
        "<cy>Common Group IDs</c> used by both levels will be remapped so they don't interfere with each other. <cg>Recommended</c> to be always checked.",
        "OK"
    )->show();
}

void ComparisonMenu::onReplaceDisappearingInfo(CCObject*) {
    FLAlertLayer::create(
        "Replace Disappearing",
        "Replaces <cy>disappearing objects</c> with their visible counterparts.",
        "OK"
    )->show();
}

void ComparisonMenu::onPickLevel1(CCObject*) {
    LevelPickerPopup::create([this](GJGameLevel* level) {
        m_pickedLevel1 = level;
        level1ID = level->m_levelID;
        bool isEditor = level->m_levelType == GJLevelType::Editor;
        if (level1Input) {
            level1Input->setEnabled(!isEditor);
            level1Input->setString(isEditor
                ? fmt::format("({})", EditorIDs::getID(level)).c_str()
                : std::to_string(level1ID).c_str()
            );
        }
        if (!isEditor && (level->m_levelNotDownloaded || level->m_levelString.empty()))
            GameLevelManager::sharedState()->downloadLevel(level1ID, false, 0);
    })->show();
}

void ComparisonMenu::onPickLevel2(CCObject*) {
    LevelPickerPopup::create([this](GJGameLevel* level) {
        m_pickedLevel2 = level;
        level2ID = level->m_levelID;
        bool isEditor = level->m_levelType == GJLevelType::Editor;
        if (level2Input) {
            level2Input->setEnabled(!isEditor);
            level2Input->setString(isEditor
                ? fmt::format("({})", EditorIDs::getID(level)).c_str()
                : std::to_string(level2ID).c_str()
            );
        }
        if (!isEditor && (level->m_levelNotDownloaded || level->m_levelString.empty()))
            GameLevelManager::sharedState()->downloadLevel(level2ID, false, 0);
    })->show();
}

void ComparisonMenu::onAbort(CCObject*) {
    this->removeFromParentAndCleanup(true);
}

void ComparisonMenu::keyBackClicked() {
    onAbort(nullptr);
}

void ComparisonMenu::onCreate(CCObject*) {
    GameLevelManager* glm = GameLevelManager::sharedState();

    auto tryRefresh = [&](GJGameLevel* lvl, int id) -> GJGameLevel* {
        if (lvl && !lvl->m_levelString.empty()) return lvl;
        if (auto f = glm->getSavedLevel(id); f && !f->m_levelString.empty()) return f;
        return lvl;
    };

    GJGameLevel* level1 = m_fixedLevel1;
    if (!level1) level1 = m_pickedLevel1;
    if (!level1 && level1ID != 0) level1 = glm->getSavedLevel(level1ID);
    if (level1 && !m_fixedLevel1) level1 = tryRefresh(level1, level1->m_levelID);

    GJGameLevel* level2 = m_pickedLevel2;
    if (!level2 && level2ID != 0) level2 = glm->getSavedLevel(level2ID);
    if (level2) level2 = tryRefresh(level2, level2->m_levelID);

    if (!level1) {
        FLAlertLayer::create(
            "Invalid Level 1",
            "Please enter a valid <cy>Level ID</c> or pick a level for <cg>Level 1</c>.",
            "OK"
        )->show();
        return;
    }

    if (!level2) {
        FLAlertLayer::create(
            "Invalid Level 2",
            "Please enter a valid <cy>Level ID</c> or pick a level for <cg>Level 2</c>.",
            "OK"
        )->show();
        return;
    }

    if (!m_fixedLevel1 && level1->m_levelType != GJLevelType::Editor && (level1->m_levelNotDownloaded || level1->m_levelString.empty())) {
        glm->downloadLevel(level1ID, false, 0);
        FLAlertLayer::create(
            "Level 1 not downloaded",
            "Try again, check the Level ID or your connection.",
            "OK"
        )->show();
        return;
    }

    if (level2->m_levelType != GJLevelType::Editor && (level2->m_levelNotDownloaded || level2->m_levelString.empty())) {
        glm->downloadLevel(level2ID, false, 0);
        FLAlertLayer::create(
            "Level 2 not downloaded",
            "Try again, check the Level ID or your connection.",
            "OK"
        )->show();
        return;
    }

    auto mod = Mod::get();
    mod->setSavedValue("source-level-id", level1->m_levelType == GJLevelType::Editor ? EditorIDs::getID(level1) : level1ID);
    mod->setSavedValue("source-level-editor", level1->m_levelType == GJLevelType::Editor);
    mod->setSavedValue("target-level-id", level2->m_levelType == GJLevelType::Editor ? EditorIDs::getID(level2) : level2ID);
    mod->setSavedValue("target-level-editor", level2->m_levelType == GJLevelType::Editor);
    mod->setSavedValue("saw-rotation-speed", sawRotationSpeed);
    for (auto& b : savedBools) mod->setSavedValue(b.key, *getBoolPtr(b.key));

    ComparisonConfig config;
    config.level1Buffed = isBuffed;
    config.sawSpeed = sawRotationSpeed;
    config.remapGroups = remapGroups;
    config.replaceDisappearing = replaceDisappearing;
    config.unhide = unhideEnabled ? unhide : UnhideOptions{ false, false };
    config.modifiers = modifiersEnabled ? modifiers : ModifierOptions{ false, false, false, false, false };
    config.objectOptions = objectOptionsEnabled ? objectOptions : ObjectOptions{ false, false, false, false, false, false };

    if (onCreateCallback)
        onCreateCallback(level1, level2, config);

    this->removeFromParentAndCleanup(true);
}