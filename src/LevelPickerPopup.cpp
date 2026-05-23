#include "LevelPickerPopup.hpp"
#include <algorithm>

using namespace geode::prelude;

static const float c_cellH = 90.f;
static const float c_listW = 356.f;
static const float c_listH = 165.f;
static const float c_maxCells = 50;

LevelPickerCell* LevelPickerCell::create(GJGameLevel* level, float width, std::function<void(GJGameLevel*)> onSelect) {
    auto ret = new LevelPickerCell();
    if (ret && ret->init(level, width, onSelect)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LevelPickerCell::init(GJGameLevel* level, float width, std::function<void(GJGameLevel*)> onSelect) {
    if (!CCLayerColor::initWithColor({ 0, 0, 0, 0 }, width, c_cellH)) return false;

    m_level = level;
    m_onSelect = onSelect;

    auto levelCell = LevelCell::create(c_listW, c_cellH);
    levelCell->loadFromLevel(level);
    levelCell->setContentSize({ width, c_cellH });
    levelCell->setPosition({ 0.f, 0.f });
    if (auto btn = levelCell->getChildByIDRecursive("view-button"))
        btn->setVisible(false);
    this->addChild(levelCell, 0);

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    this->addChild(menu, 1);

    auto spr = ButtonSprite::create("Select");
    spr->setScale(0.85f);
    auto selectBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(LevelPickerCell::onSelectBtn)
    );
    selectBtn->setPosition({ width - 58.f, c_cellH / 2.f });
    menu->addChild(selectBtn);

    return true;
}

void LevelPickerCell::onSelectBtn(CCObject*) {
    if (m_onSelect) m_onSelect(m_level);
}


LevelPickerPopup* LevelPickerPopup::create(std::function<void(GJGameLevel*)> onSelect) {
    auto ret = new LevelPickerPopup();
    if (ret && ret->init()) {
        ret->m_onSelect = onSelect;
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LevelPickerPopup::init() {
    if (!Popup::init(380.f, 280.f)) return false;
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    auto panel = CCLayerColor::create({ 0, 0, 0, 0 });
    panel->setContentSize({ 380.f, 280.f });
    this->m_mainLayer->addChildAtPosition(panel, Anchor::BottomLeft);

    auto title = CCLabelBMFont::create("Choose Level", "bigFont.fnt");
    title->setPosition({ 190.f, 262.f });
    title->setScale(0.7f);
    panel->addChild(title);

    auto searchInput = TextInput::create(240.f, "Search", "bigFont.fnt");
    searchInput->setPosition({ 190.f, 230.f });
    searchInput->setCallback([this](const std::string& text) {
        m_searchQuery = text;
        loadList();
    });
    panel->addChild(searchInput);

    auto menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    panel->addChild(menu);

    m_tabLocal = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Local"),
        this,
        menu_selector(LevelPickerPopup::onTabLocal)
    );
    m_tabLocal->setPosition({ 110.f, 190.f });
    menu->addChild(m_tabLocal);

    m_tabSaved = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Saved"),
        this,
        menu_selector(LevelPickerPopup::onTabSaved)
    );
    m_tabSaved->setPosition({ 270.f, 190.f });
    menu->addChild(m_tabSaved);

    m_scrollLayer = ScrollLayer::create({ c_listW, c_listH });
    m_scrollLayer->setPosition({ 12.f, 7.f });
    this->m_mainLayer->addChild(m_scrollLayer);

    loadList();

    return true;
}

std::vector<GJGameLevel*> LevelPickerPopup::getFilteredLevels() {
    std::vector<GJGameLevel*> levels;

    if (m_showingLocal) {
        for (auto level : CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels))
            levels.push_back(level);
    } else {
        auto arr = GameLevelManager::sharedState()->getSavedLevels(false, 0);
        if (arr) {
            for (auto level : CCArrayExt<GJGameLevel*>(arr))
                levels.push_back(level);
        }
    }

    if (m_searchQuery.empty()) {
        if (levels.size() > c_maxCells) levels.resize(c_maxCells);
        return levels;
    }

    std::string query = m_searchQuery;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    std::vector<GJGameLevel*> filtered;
    for (GJGameLevel* level : levels) {
        std::string name = level->m_levelName;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(query) != std::string::npos) filtered.push_back(level);
    }
    if (filtered.size() > c_maxCells) filtered.resize(c_maxCells);
    return filtered;
}

void LevelPickerPopup::loadList() {
    m_scrollLayer->m_contentLayer->removeAllChildren();

    auto levels = getFilteredLevels();
    float totalHeight = std::max(static_cast<float>(levels.size()) * c_cellH, c_listH);
    m_scrollLayer->m_contentLayer->setContentSize({ c_listW, totalHeight });

    float y = totalHeight;
    int index = 0;
    for (GJGameLevel* level : levels) {
        y -= c_cellH;
        auto cell = LevelPickerCell::create(level, c_listW, [this](GJGameLevel* selected) {
            if (m_onSelect) m_onSelect(selected);
            this->removeFromParentAndCleanup(true);
        });
        cell->setPosition({ 0.f, y });
        if (index % 2 == 0) {
            cell->setColor({ 0, 0, 0 });
            cell->setOpacity(40);
        } else {
            cell->setColor({ 255, 255, 255 });
            cell->setOpacity(8);
        }
        m_scrollLayer->m_contentLayer->addChild(cell);
        index++;
    }

    m_scrollLayer->moveToTop();
}

void LevelPickerPopup::onTabLocal(CCObject*) {
    m_showingLocal = true;
    loadList();
}

void LevelPickerPopup::onTabSaved(CCObject*) {
    m_showingLocal = false;
    loadList();
}

void LevelPickerPopup::keyBackClicked() {
    this->removeFromParentAndCleanup(true);
}