#include "OptionsPopup.hpp"

using namespace geode::prelude;

static constexpr float c_padX = 12.f;
static constexpr float c_padY = 10.f;
static constexpr float c_rowH = 28.f;
static constexpr float c_sectionH = 24.f;
static constexpr float c_btnH = 30.f;
static constexpr float c_titleH = 28.f;

OptionsToggle* OptionsToggle::create(ToggleOption opt, float width) {
    auto ret = new OptionsToggle();
    if (ret && ret->init(opt, width)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool OptionsToggle::init(ToggleOption opt, float width) {
    if (!CCLayer::init()) return false;

    m_value = opt.value;

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    this->addChild(menu);

    m_toggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(OptionsToggle::onToggle), 0.7f
    );
    m_toggle->toggle(*m_value);
    m_toggle->setPosition({ 15.f, c_rowH * 0.5f });
    menu->addChild(m_toggle);

    auto lbl = CCLabelBMFont::create(opt.label.c_str(), "goldFont.fnt");
    lbl->setAnchorPoint({ 0.f, 0.5f });
    lbl->setPosition({ 30.f, c_rowH * 0.5f });
    lbl->setScale(0.6f);
    this->addChild(lbl);

    if (!opt.infoText.empty()) {
        m_infoText = opt.infoText;
        auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        infoSpr->setScale(0.4f);
        auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(OptionsToggle::onInfo));
        infoBtn->setPosition({ 30.f + (lbl->getContentSize().width * 0.6f) + 2.f + infoSpr->getContentSize().width * 0.4f * 0.5f, 2.0f + c_rowH * 0.5f });
        menu->addChild(infoBtn);
    }

    return true;
}

void OptionsToggle::onToggle(CCObject*) {
    *m_value = !m_toggle->m_toggled;
}

void OptionsToggle::onInfo(CCObject*) {
    FLAlertLayer::create("Info", m_infoText.c_str(), "OK")->show();
}


OptionsPopup* OptionsPopup::create(OptionsSection section, std::function<void()> onClose) {
    auto ret = new OptionsPopup();
    if (ret && ret->init(section, onClose)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool OptionsPopup::init(OptionsSection section, std::function<void()> onClose) {
    float contentH = c_titleH + c_sectionH + section.options.size() * c_rowH + c_padY + c_btnH + c_padY;
    float maxLabelW = 0.f;
    for (auto& opt : section.options) {
        auto* lbl = CCLabelBMFont::create(opt.label.c_str(), "goldFont.fnt");
        float w = lbl->getContentSize().width * 0.6f;
        if (!opt.infoText.empty()) w += 8.f + 12.f;
        if (w > maxLabelW) maxLabelW = w;
    }
    float contentW = c_padX + 30.f + maxLabelW + c_padX;
    contentW = std::max(contentW, 160.f);

    if (!Popup::init(contentW, contentH)) return false;
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    m_onClose = onClose;

    auto panel = CCLayerColor::create({ 0, 0, 0, 0 });
    panel->setContentSize({ contentW, contentH });
    this->m_mainLayer->addChildAtPosition(panel, Anchor::BottomLeft);

    auto title = CCLabelBMFont::create("Options", "goldFont.fnt");
    title->setPosition({ contentW / 2.f, contentH - c_titleH * 0.5f });
    panel->addChild(title);

    float y = contentH - c_titleH - c_sectionH;

    auto secLabel = CCLabelBMFont::create(section.title.c_str(), "goldFont.fnt");
    secLabel->setAnchorPoint({ 0.f, 0.5f });
    secLabel->setPosition({ c_padX, y + c_sectionH * 0.5f });
    secLabel->setScale(0.75f);
    panel->addChild(secLabel);

    for (auto& opt : section.options) {
        y -= c_rowH;
        auto row = OptionsToggle::create(opt, contentW);
        row->setPosition({ c_padX, y });
        panel->addChild(row);
    }

    auto closeMenu = CCMenu::create();
    closeMenu->setPosition({ 0, 0 });
    panel->addChild(closeMenu);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Close"),
        this,
        menu_selector(OptionsPopup::onClose)
    );
    closeBtn->setPosition({ contentW / 2.f, c_btnH * 0.5f + c_padY * 0.5f });
    closeMenu->addChild(closeBtn);

    return true;
}

void OptionsPopup::onClose(CCObject*) {
    if (m_onClose) m_onClose();
    this->removeFromParentAndCleanup(true);
}

void OptionsPopup::keyBackClicked() {
    onClose(nullptr);
}