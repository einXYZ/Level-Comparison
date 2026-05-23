#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <functional>
#include <vector>

using namespace geode::prelude;

class LevelPickerCell : public CCLayerColor {
public:
    GJGameLevel* m_level = nullptr;
    std::function<void(GJGameLevel*)> m_onSelect;

    static LevelPickerCell* create(GJGameLevel* level, float width, std::function<void(GJGameLevel*)> onSelect);
    bool init(GJGameLevel* level, float width, std::function<void(GJGameLevel*)> onSelect);

private:
    void onSelectBtn(CCObject*);
};

class LevelPickerPopup : public geode::Popup {
public:
    std::function<void(GJGameLevel*)> m_onSelect;

    static LevelPickerPopup* create(std::function<void(GJGameLevel*)> onSelect);
    bool init() override;

private:
    bool m_showingLocal = true;
    std::string m_searchQuery = "";
    ScrollLayer* m_scrollLayer = nullptr;
    CCMenuItemSpriteExtra* m_tabLocal = nullptr;
    CCMenuItemSpriteExtra* m_tabSaved = nullptr;

    void loadList();
    std::vector<GJGameLevel*> getFilteredLevels();
    void onTabLocal(CCObject*);
    void onTabSaved(CCObject*);
    void keyBackClicked() override;
};