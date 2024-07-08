#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include "objects.hpp"

using namespace geode::prelude;

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter);


class $modify(MakeLevelLayoutLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;
        
        auto menu = this->getChildByID("left-side-menu");
        if (menu) {
            auto btn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
                this, menu_selector(MakeLevelLayoutLayer::onButton)
            );
            btn->setID("export-button"_spr);
            menu->addChild(btn);
            menu->updateLayout();
        }

        return true;
    }

    void onButton(CCObject*) {
		std::string levelName = this->m_level->m_levelName;
        std::string levelString = this->m_level->m_levelString;
		std::string levelStringNormal = ZipUtils::decompressString(levelString, false, 0);

		std::vector<std::string> levelStringSplit = splitString(levelStringNormal, ";");
		std::string firstElement = levelStringSplit.front();
		levelStringSplit.erase(levelStringSplit.begin());

		for (std::string& str : levelStringSplit) {
			// log::debug("{}", str);
			std::vector<std::string> splitStrings = splitString(str, ",");
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < splitStrings.size() - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}
			for (std::vector<std::string>& pair : splitStringsPairs) {
				// log::debug("Pair: {}, {}", pair[0], pair[1]);
				if (pair[0] == "1") {
					if (std::find(objects.begin(), objects.end(), std::stoi(pair[1])) == objects.end()) {
						str = "";
					}
				}
			}
		}

		levelStringSplit.erase(std::remove(levelStringSplit.begin(), levelStringSplit.end(), ""), levelStringSplit.end());

		std::string levelStringFinal = firstElement;

		for (std::string& str : levelStringSplit) {
			levelStringFinal = levelStringFinal + ";" + str;
		}
		levelStringFinal = levelStringFinal + ";";

		std::string levelStringCompressed = ZipUtils::compressString(levelStringFinal, false, 0);
		
		FLAlertLayer::create(
			"Level Comparison",
			std::string("Successfully created layout of ") + levelName.c_str(),
			"OK"
		)->show();
		// log::info("{}", levelStringNormal);
		// log::info("{}", levelStringFinal);


		// create new level
        GameLevelManager *gameLevelManager = GameLevelManager::sharedState();
        GJGameLevel* newLevel = gameLevelManager->createNewLevel();
        newLevel->m_levelName = "TestNewName";
        newLevel->m_levelString = levelStringFinal;
    }

};

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter) {
	std::vector<std::string> splitStrings;
	size_t pos = 0;
	std::string token;
	std::string str = s;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		token = str.substr(0, pos);
		splitStrings.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	splitStrings.push_back(str);
	return splitStrings;
}
