#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "ComparisonMenu.hpp"
#include "ComparisonLogic.hpp"

using namespace geode::prelude;

class $modify(MakeLevelLayoutLayer, LevelInfoLayer) {
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
            menu_selector(MakeLevelLayoutLayer::onButton)
        );

        btn->setID("create-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onButton(CCObject*) {
        ComparisonMenu::create(
            [this](int levelID, bool isBuffed, float sawSpeed, bool remapGroups, bool unhideObjects, bool showModifiers) {

                GameLevelManager* glm = GameLevelManager::sharedState();

                GJGameLevel* level1 = this->m_level;
                GJGameLevel* level2 = glm->getSavedLevel(levelID);

                ComparisonConfig config;
                config.isBuffed = isBuffed;
                config.sawSpeed = sawSpeed;
                config.remapGroups = remapGroups;
                config.unhideObjects = unhideObjects;
                config.showModifiers = showModifiers;

                std::string modifiedLevelString = createComparison(level1, level2, config);

                GJGameLevel* newLevel = glm->createNewLevel();
                newLevel->m_levelName = "Unnamed comparison";
                newLevel->m_levelString = modifiedLevelString;
                newLevel->m_levelDesc = ZipUtils::base64URLEncode(fmt::format(
                    "Comparison of {} by {} {} and {} by {} {}",
                    level1->m_levelName.c_str(),
                    level1->m_creatorName.c_str(),
                    config.isBuffed ? "(red)" : "(blue)",
                    level2->m_levelName.c_str(),
                    level2->m_creatorName.c_str(),
                    !config.isBuffed ? "(red)" : "(blue)"
                ));
                newLevel->m_songID = level1->m_songID;
                newLevel->m_audioTrack = level1->m_audioTrack;
                newLevel->m_levelLength = level1->m_levelLength;

                auto scene = EditLevelLayer::scene(newLevel);
                CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
            }
        )->show();
    }
};
