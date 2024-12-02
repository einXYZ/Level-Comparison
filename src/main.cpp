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
std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter);

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
		GameLevelManager *gameLevelManager = GameLevelManager::sharedState();
		GJGameLevel *level = this->m_level;
		// GJGameLevel *level = gameLevelManager->getSavedLevel(26681070);
		std::string levelString = ZipUtils::decompressString(level->m_levelString, false, 0);
		log::info("{}", levelString);

		std::vector<std::string> levelStringSplit = splitString(levelString, ";");
		std::string firstElement = levelStringSplit.front();
		levelStringSplit.erase(levelStringSplit.begin());

		for (std::string& objectStr : levelStringSplit) {
			bool isDecoration = false;
			bool dontFade = false;  // 64
			bool dontEnter = false; // 67
			bool customRotationSpeed = false; // 97
			std::string newObjectStr = "";

			std::vector<std::string> splitStrings = splitString(objectStr, ",");
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < splitStrings.size() - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}

			// log::info("Object before: {}", objectStr);

			for (std::vector<std::string>& pair : splitStringsPairs) {
				int propID = std::stoi(pair[0]);

				if (propID == 1) { // id
					if (std::find(objects.begin(), objects.end(), std::stoi(pair[1])) == objects.end()) { // Check if object is decoration
						isDecoration = true;
						objectStr = "";
						break;
					} else {
						newObjectStr += "1," + pair[1] + ",";
						continue;
					}
				}

				if (isDecoration) break;

				switch (propID) {
					case 21: // color 1 to channel 1
						newObjectStr += "21,1,";
						continue;
					case 22: // color 2 to channel 1
						newObjectStr += "22,1,";
						continue;
					case 43: // delete HSV 1
						continue;
					case 44: // delete HSV 2
						continue;
					case 64: // don't fade
						dontFade = true;
						newObjectStr += "64,1,";
						continue;
					case 67: // don't enter
						dontEnter = true;
						newObjectStr += "67,1,";
						continue;
					case 97: // custom rotation speed
						customRotationSpeed = true;
						newObjectStr += "97,69.000000,";
						continue;
					case 98: // turn off disable rotation
						continue;
					case 103: // turn off high detail
						continue;
					default:
						newObjectStr += pair[0] + "," + pair[1] + ",";
						continue;
				}
			}

			if (!dontFade) newObjectStr += "64,1,";
			if (!dontEnter) newObjectStr += "67,1,";
			if (!customRotationSpeed) newObjectStr += "97,69.000000,";

			if (!newObjectStr.empty()) {
				newObjectStr.pop_back(); // Remove the last comma
				objectStr = newObjectStr;
			} else {
				objectStr = "";
			}

			// log::info("Object after: {}", objectStr);
		}

		std::string modifiedLevelString = firstElement + ";" + joinString(levelStringSplit, ";");
		
		FLAlertLayer::create(
			"Level Comparison",
			std::string("Successfully created layout of ") + level->m_levelName.c_str(),
			"OK"
		)->show();

		// log::info("{}", levelString);
		// log::info("{}", modifiedLevelString);


		// create new level
        GJGameLevel* newLevel = gameLevelManager->createNewLevel();
        newLevel->m_levelName = "Test " + level->m_levelName;
        newLevel->m_levelString = modifiedLevelString;
		newLevel->m_levelDesc = ZipUtils::base64URLEncode("Comparison of " + level->m_levelName);
		newLevel->m_songID = level->m_songID;
    }
};

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> splitStrings;
    size_t pos = 0, found;
    while ((found = s.find(delimiter, pos)) != std::string::npos) {
        if (found != pos) { // Avoid adding empty strings
            splitStrings.push_back(s.substr(pos, found - pos));
        }
        pos = found + delimiter.length();
    }
    if (pos < s.size()) {
        splitStrings.push_back(s.substr(pos));
    }
    return splitStrings;
}

std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < elems.size(); ++i) {
        if (i != 0)
            ss << delimiter;
        ss << elems[i];
    }
    return ss.str();
}
