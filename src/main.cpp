#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include "ComparisonMenu.hpp"
#include "ComparisonLogic.hpp"

using namespace geode::prelude;

static void openComparisonMenu(CCNode* self, GJGameLevel* fixedLevel1) {
    ComparisonMenu::create(
        fixedLevel1,
        [self](GJGameLevel* level1, GJGameLevel* level2, ComparisonConfig config) {

            GameLevelManager* glm = GameLevelManager::sharedState();

            std::string modifiedLevelString = createComparison(level1, level2, config);

            GJGameLevel* newLevel = glm->createNewLevel();
            newLevel->m_levelName = "Unnamed comparison";
            newLevel->m_levelString = modifiedLevelString;
            newLevel->m_levelDesc = ZipUtils::base64URLEncode(fmt::format(
                "Comparison of {} by {} {} and {} by {} {}",
                level1->m_levelName.c_str(),
                level1->m_creatorName.c_str(),
                config.level1Buffed ? "(red)" : "(blue)",
                level2->m_levelName.c_str(),
                level2->m_creatorName.c_str(),
                !config.level1Buffed ? "(red)" : "(blue)"
            ));
            newLevel->m_songID = level1->m_songID;
            newLevel->m_audioTrack = level1->m_audioTrack;
            newLevel->m_levelLength = level1->m_levelLength;

            auto scene = EditLevelLayer::scene(newLevel);
            CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
        }
    )->show();
}

class $modify(MakeLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        auto menu = this->getChildByID("left-side-menu");
        if (!menu) {
            log::error("left-side-menu not found");
            return true;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSpriteFrameName(
                "create.png"_spr, .8f,
                CircleBaseColor::Green,
                CircleBaseSize::MediumAlt
            ),
            this,
            menu_selector(MakeLevelInfoLayer::onButton)
        );

        btn->setID("create-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onButton(CCObject*) {
        openComparisonMenu(this, this->m_level);
    }
};

class $modify(MakeEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level))
            return false;

        auto menu = this->getChildByID("folder-menu");
        if (!menu) {
            log::error("folder-menu not found");
            return true;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSpriteFrameName(
                "create.png"_spr, .8f,
                CircleBaseColor::Green,
                CircleBaseSize::Small
            ),
            this,
            menu_selector(MakeEditLevelLayer::onButton)
        );

        btn->setID("create-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onButton(CCObject*) {
        openComparisonMenu(this, this->m_level);
    }
};

class $modify(MakeLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* searchObject) {
        if (!LevelBrowserLayer::init(searchObject))
            return false;

        bool usedFallback = false;
        auto menu = this->getChildByID("my-levels-menu");
        if (!menu) {
            menu = this->getChildByID("saved-menu");
            usedFallback = true;
        }
        if (!menu) {
            log::error("my-levels-menu and saved-menu not found");
            return true;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSpriteFrameName(
                "create.png"_spr, .8f,
                CircleBaseColor::Green,
                usedFallback ? CircleBaseSize::Small : CircleBaseSize::MediumAlt
            ),
            this,
            menu_selector(MakeLevelBrowserLayer::onButton)
        );

        btn->setID("create-button"_spr);
        if (usedFallback)
            btn->setPosition({ -515.f, 90.f });
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onButton(CCObject*) {
        openComparisonMenu(this, nullptr);
    }
};