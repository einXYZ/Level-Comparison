#pragma once
#include "data.hpp"
#include <Geode/Geode.hpp>
#include <functional>
#include <vector>
#include <string>

using namespace geode::prelude;

struct ToggleOption {
    std::string label;
    bool* value;
    std::string infoText;
};

struct OptionsSection {
    std::string title;
    std::vector<ToggleOption> options;
};

class OptionsToggle : public CCLayer {
public:
    bool* m_value = nullptr;
    CCMenuItemToggler* m_toggle = nullptr;
    std::string m_infoText;

    static OptionsToggle* create(ToggleOption opt, float width);
    bool init(ToggleOption opt, float width);

private:
    void onToggle(CCObject*);
    void onInfo(CCObject*);
};

class OptionsPopup : public geode::Popup {
public:
    static OptionsPopup* create(OptionsSection section, std::function<void()> onClose);
    bool init(OptionsSection section, std::function<void()> onClose);

private:
    std::function<void()> m_onClose;

    void onClose(CCObject*);
    void keyBackClicked() override;
};