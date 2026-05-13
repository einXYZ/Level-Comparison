#include "ComparisonMenu.hpp"
#include "utils.hpp"

using namespace geode::prelude;
using namespace lc;

ComparisonMenu* ComparisonMenu::create(std::function<void(int, bool, float, bool, bool, bool)> onCreate) {
    auto ret = new ComparisonMenu();
    if (ret && ret->init()) {
        ret->onCreateCallback = onCreate;
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ComparisonMenu::init() {
    if (!Popup::init(360.f, 260.f)) return false;
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto mod = Mod::get();
    targetLevelID = mod->getSavedValue<int>("target-level-id", 0);
    isBuffed = mod->getSavedValue<bool>("is-buffed", false);
    sawRotationSpeed = mod->getSavedValue<float>("saw-rotation-speed", 0.f);
    remapGroups = mod->getSavedValue<bool>("remap-groups", true);
    unhideObjects = mod->getSavedValue<bool>("unhide-invisible", true);
    showModifiers = mod->getSavedValue<bool>("show-modifiers", false);

    auto panel = CCLayerColor::create({ 0, 0, 0, 0 });
    panel->setContentSize({ 360.f, 260.f });
    panel->setID("create-comparison-background"_spr);
    this->m_mainLayer->addChildAtPosition(panel, Anchor::BottomLeft);

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    panel->addChild(menu);

    auto title = CCLabelBMFont::create("Level Comparison", "bigFont.fnt");
    title->setPosition({ 180.f, 240.f });
    title->setScale(0.8f);
    panel->addChild(title);

    auto idLabel = CCLabelBMFont::create("Level ID", "goldFont.fnt");
    idLabel->setPosition({ 80.f, 215.f });
    idLabel->setScale(0.8);
    panel->addChild(idLabel);

    auto idInput = TextInput::create(120.f, "Level ID", "bigFont.fnt");
    idInput->setMaxCharCount(10);
    idInput->setFilter("0123456789");
    idInput->setPosition({ 80.f, 185.f });
    idInput->setEnabled(true);
    idInput->setID("level-id-input"_spr);
    idInput->setString(targetLevelID != 0 ? std::to_string(targetLevelID).c_str() : "");
    idInput->setCallback([this](std::string const& text) {
        targetLevelID = text.empty() ? 0 : lc::stoi(const_cast<std::string&>(text));
    });
    panel->addChild(idInput);

    nerfedToggle = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(ComparisonMenu::onNerfed),
        0.8f
    );
    nerfedToggle->setPosition({ 160.f, 190.f });
    menu->addChild(nerfedToggle);

    auto nerfedLabel = CCLabelBMFont::create("Nerfed", "goldFont.fnt");
    nerfedLabel->setPosition({ 210.f, 190.f });
    nerfedLabel->setScale(0.8);
    panel->addChild(nerfedLabel);

    buffedToggle = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(ComparisonMenu::onBuffed),
        0.8f
    );
    buffedToggle->setPosition({ 270.f, 190.f });
    menu->addChild(buffedToggle);

    auto buffedLabel = CCLabelBMFont::create("Buffed", "goldFont.fnt");
    buffedLabel->setPosition({ 320.f, 190.f });
    buffedLabel->setScale(0.8);
    panel->addChild(buffedLabel);

    auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSprite->setScale(0.5f);

    auto nerfedBuffedInfo = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(ComparisonMenu::onBuffedNerfedInfo)
    );
    nerfedBuffedInfo->setPosition({ 350.f, 205.f });
    menu->addChild(nerfedBuffedInfo);

    buffedToggle->toggle(isBuffed);
    nerfedToggle->toggle(!isBuffed);
    buffedToggle->setClickable(false);
    nerfedToggle->setClickable(false);

    auto speedText = CCLabelBMFont::create("Saw Rotation", "goldFont.fnt");
    speedText->setPosition({ 80.f, 155.f });
    speedText->setScale(0.8);
    panel->addChild(speedText);

    auto sawRotationInfo = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(ComparisonMenu::onSawRotationInfo)
    );
    sawRotationInfo->setPosition({ 155.f, 160.f });
    menu->addChild(sawRotationInfo);

    auto sawSpeedInput = TextInput::create(80.f, "0", "bigFont.fnt");
    sawSpeedInput->setMaxCharCount(3);
    sawSpeedInput->setFilter("0123456789-");
    sawSpeedInput->setPosition({80.f, 125.f});
    sawSpeedInput->setEnabled(true);
    sawSpeedInput->setID("saw-speed-input"_spr);
    sawSpeedInput->setString(std::to_string(static_cast<int>(sawRotationSpeed)).c_str());
    sawSpeedInput->setCallback([this](std::string const& text) {
        if (text.empty() || text == "-") return;
        if (text.find('-', 1) != std::string::npos) return;
        sawRotationSpeed = lc::stof(const_cast<std::string&>(text));
    });
    panel->addChild(sawSpeedInput);

    remapToggle = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(ComparisonMenu::onRemap),
        0.8f
    );
    remapToggle->setPosition({ 160.f, 130.f });
    remapToggle->toggle(remapGroups);
    menu->addChild(remapToggle);

    auto remapLabel = CCLabelBMFont::create("Remap\ngroups", "goldFont.fnt");
    remapLabel->setPosition({ 210.f, 130.f });
    remapLabel->setScale(0.7);
    panel->addChild(remapLabel);

    auto remapInfo = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(ComparisonMenu::onRemapGroupsInfo)
    );
    remapInfo->setPosition({ 240.f, 150.f });
    menu->addChild(remapInfo);

    unhideToggle = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(ComparisonMenu::onUnhide),
        0.8f
    );
    unhideToggle->setPosition({ 270.f, 130.f });
    unhideToggle->toggle(unhideObjects);
    menu->addChild(unhideToggle);

    auto unhideLabel = CCLabelBMFont::create("Unhide\nobjects", "goldFont.fnt");
    unhideLabel->setPosition({ 320.f, 130.f });
    unhideLabel->setScale(0.7);
    panel->addChild(unhideLabel);

    auto unhideInfo = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(ComparisonMenu::onUnhideInfo)
    );
    unhideInfo->setPosition({ 350.f, 150.f });
    menu->addChild(unhideInfo);

    showModifiersToggle = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(ComparisonMenu::onShowModifiers),
        0.8f
    );
    showModifiersToggle->setPosition({ 160.f, 80.f });
    showModifiersToggle->toggle(showModifiers);
    menu->addChild(showModifiersToggle);

    auto showModifiersLabel = CCLabelBMFont::create("Show\nmodifiers", "goldFont.fnt");
    showModifiersLabel->setPosition({ 210.f, 80.f });
    showModifiersLabel->setScale(0.6);
    panel->addChild(showModifiersLabel);

    auto showModifiersInfo = CCMenuItemSpriteExtra::create(
        infoSprite,
        this,
        menu_selector(ComparisonMenu::onShowModifiersInfo)
    );
    showModifiersInfo->setPosition({ 240.f, 100.f });
    menu->addChild(showModifiersInfo);

    auto abortBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Abort"),
        this,
        menu_selector(ComparisonMenu::onAbort)
    );
    abortBtn->setPosition({ 100.f, 20.f });
    menu->addChild(abortBtn);

    auto createBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Create"),
        this,
        menu_selector(ComparisonMenu::onCreate)
    );
    createBtn->setPosition({ 260.f, 20.f });
    menu->addChild(createBtn);

    return true;
}

void ComparisonMenu::onBuffed(CCObject*) {
    if (isBuffed) return;
    isBuffed = true;
    buffedToggle->toggle(true);
    nerfedToggle->toggle(false);
}

void ComparisonMenu::onNerfed(CCObject*) {
    if (!isBuffed) return;
    isBuffed = false;
    buffedToggle->toggle(false);
    nerfedToggle->toggle(true);
}

void ComparisonMenu::onRemap(CCObject*) {
    remapGroups = !remapToggle->m_toggled;
}

void ComparisonMenu::onUnhide(CCObject*) {
    unhideObjects = !unhideToggle->m_toggled;
}

void ComparisonMenu::onShowModifiers(CCObject*) {
    showModifiers = !showModifiersToggle->m_toggled;
}

void ComparisonMenu::onBuffedNerfedInfo(CCObject*) {
    FLAlertLayer::create(
        "Info",
        "Select whether currently opened level is the <cj>nerfed</c> or <cr>buffed</c> version.",
        "OK"
    )->show();
}

void ComparisonMenu::onSawRotationInfo(CCObject*) {
    FLAlertLayer::create(
        "Saw Rotation",
        "Sets the rotation speed of <cy>saw blade objects</c> in the comparison. Set to <cg>0</c> to disable saw rotation.",
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

void ComparisonMenu::onUnhideInfo(CCObject*) {
    FLAlertLayer::create(
        "Unhide Objects",
        "Deletes <cy>alpha triggers</c> and removes the <cy>Hide</c> checkmark from every object. <cg>Recommended</c> to be checked to see all objects.",
        "OK"
    )->show();
}

void ComparisonMenu::onShowModifiersInfo(CCObject*) {
    FLAlertLayer::create(
        "Show Modifiers",
        "Visually displays <cy>modifier blocks</c> such as D-blocks, J-blocks etc.",
        "OK"
    )->show();
}

void ComparisonMenu::onComingSoonInfo(CCObject*) {
    FLAlertLayer::create(
        "Info",
        "This feature is coming soon!",
        "OK"
    )->show();
}

void ComparisonMenu::onAbort(CCObject*) {
    this->removeFromParentAndCleanup(true);
}

void ComparisonMenu::keyBackClicked() {
    onAbort(nullptr);
}

void ComparisonMenu::onCreate(CCObject*) {
    if (targetLevelID == 0) {
        FLAlertLayer::create(
            "Missing Level ID",
            "Please enter a valid <cy>Level ID</c> before creating a comparison.",
            "OK"
        )->show();
        return;
    }

    auto mod = Mod::get();
    mod->setSavedValue("target-level-id", targetLevelID);
    mod->setSavedValue("is-buffed", isBuffed);
    mod->setSavedValue("saw-rotation-speed", sawRotationSpeed);
    mod->setSavedValue("remap-groups", remapGroups);
    mod->setSavedValue("unhide-invisible", unhideObjects);
    mod->setSavedValue("show-modifiers", showModifiers);

    GameLevelManager* glm = GameLevelManager::sharedState();
    GJGameLevel* targetLevel = glm->getSavedLevel(targetLevelID);

    if (targetLevel == nullptr || targetLevel->m_levelNotDownloaded || targetLevel->m_levelString.empty()) {
        glm->downloadLevel(targetLevelID, false, 0);
        FLAlertLayer::create(
            "Target level not found",
            std::string("Try again, check the Level ID or your connection."),
            "OK"
        )->show();
        return;
    }

    if (onCreateCallback) {
        onCreateCallback(
            targetLevelID,
            isBuffed,
            sawRotationSpeed,
            remapGroups,
            unhideObjects,
            showModifiers
        );
    }

    this->removeFromParentAndCleanup(true);
}
