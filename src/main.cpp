#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include "objects.hpp"
#include "data.hpp"


using namespace geode::prelude;
using namespace cocos2d;
using namespace cocos2d::extension;

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config);
std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty);
std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter);
int stoi(std::string& str);
int stof(std::string& str);

class ComparisonMenu : public FLAlertLayer, public TextInputDelegate {
public:
	std::function<void(
		int targetLevelID,
		bool isBuffed,
		float sawRotationSpeed
	)> onCreateCallback;

    int targetLevelID = 0;
    bool isBuffed = false;
    float sawRotationSpeed = 0.f;

    CCLabelBMFont* speedLabel = nullptr;
    CCMenuItemToggler* buffedToggle = nullptr;
    CCMenuItemToggler* nerfedToggle = nullptr;
	CCMenuItemToggler* remapToggle = nullptr;
	CCTextInputNode* levelIDNode = nullptr;
	CCTextInputNode* sawSpeedNode = nullptr;


    static ComparisonMenu* create(std::function<void(int, bool, float)> onCreate) {
		auto ret = new ComparisonMenu();
		if (ret && ret->init()) {
			ret->onCreateCallback = onCreate;
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}


    bool init() override {
        if (!FLAlertLayer::init(75)) return false;
		this->setTouchEnabled(true);
		this->setKeypadEnabled(true);
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto mod = Mod::get();
        targetLevelID = mod->getSavedValue<int>("target-level-id", 0);
        isBuffed = mod->getSavedValue<bool>("is-buffed", false);
        sawRotationSpeed = mod->getSavedValue<float>("saw-rotation-speed", 0.f);

        auto panel = CCScale9Sprite::create("GJ_square01.png", {0.0f, 0.0f, 80.0f, 80.0f});
        panel->setContentSize({ 360.f, 260.f });
        panel->setPosition(winSize / 2);
		panel->setID("create-comparison-background"_spr);
        this->m_mainLayer->addChild(panel);

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        panel->addChild(menu);

        auto title = CCLabelBMFont::create("Level Comparison", "bigFont.fnt");
        title->setPosition({ 180.f, 240.f });
		title->setScale(0.8f);
        panel->addChild(title);

        // level id input
        auto idLabel = CCLabelBMFont::create("Level ID", "goldFont.fnt");
        idLabel->setPosition({ 80.f, 215.f });
		idLabel->setScale(0.8);
        panel->addChild(idLabel);

        auto idInput = TextInput::create(120.f, "0", "bigFont.fnt");
		idInput->setMaxCharCount(10);
		idInput->setFilter("0123456789");
		idInput->setPosition({ 80.f, 185.f });
		idInput->setEnabled(true);
		idInput->setID("level-id-input"_spr);
        idInput->setString(std::to_string(targetLevelID).c_str());
        idInput->setDelegate(this);
        panel->addChild(idInput);
		
		levelIDNode = idInput->getInputNode();
		levelIDNode->setDelegate(this);

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

        // set initial toggle state
        buffedToggle->toggle(isBuffed);
        nerfedToggle->toggle(!isBuffed);
		buffedToggle->setClickable(false);
		nerfedToggle->setClickable(false);


        // saw speed
        auto speedText = CCLabelBMFont::create("Saw Rotation", "goldFont.fnt");
        speedText->setPosition({ 80.f, 155.f });
		speedText->setScale(0.8);
        panel->addChild(speedText);

        auto sawSpeedInput = TextInput::create(80.f, "0", "bigFont.fnt");
		sawSpeedInput->setMaxCharCount(3);
		sawSpeedInput->setFilter("0123456789-");
		sawSpeedInput->setPosition({80.f, 125.f});
		sawSpeedInput->setEnabled(true);
		sawSpeedInput->setID("saw-speed-input"_spr);
        sawSpeedInput->setString(std::to_string(static_cast<int>(sawRotationSpeed)).c_str());
        sawSpeedInput->setDelegate(this);
        panel->addChild(sawSpeedInput);
	
		sawSpeedNode = sawSpeedInput->getInputNode();
		sawSpeedNode->setDelegate(this);

		// remap groups
		remapToggle = CCMenuItemToggler::createWithStandardSprites(
			this,
			menu_selector(ComparisonMenu::onNerfed),
			0.8f
		);
        /* remapToggle->setPosition({ 160.f, 130.f });
		remapToggle->setEnabled(false);
        menu->addChild(remapToggle);

        auto remapLabel = CCLabelBMFont::create("Remap groups", "goldFont.fnt");
        remapLabel->setPosition({ 240.f, 130.f });
		remapLabel->setScale(0.7);
        panel->addChild(remapLabel);

		auto remapInfo = CCMenuItemSpriteExtra::create(
			infoSprite,
			this,
			menu_selector(ComparisonMenu::onComingSoonInfo)
		);

		remapInfo->setPosition({ 300.f, 145.f });
		menu->addChild(remapInfo); */

        // buttons
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

        // log::info("Menu loaded | ID={} | Buffed={} | Speed={}", targetLevelID, isBuffed, sawRotationSpeed);
        return true;
    }

    // callbacks
	void onBuffed(CCObject*) {
		if (isBuffed) return;

		isBuffed = true;
		buffedToggle->toggle(true);
		nerfedToggle->toggle(false);
	}

	void onNerfed(CCObject*) {
		if (!isBuffed) return;

		isBuffed = false;
		buffedToggle->toggle(false);
		nerfedToggle->toggle(true);
	}

	void onBuffedNerfedInfo(CCObject*) {
		FLAlertLayer::create(
			"Info",
			"Select whether currently opened level is the <cj>nerfed</c> or <cr>buffed</c> version.",
			"OK"
		)->show();
	}

	void onComingSoonInfo(CCObject*) {
		FLAlertLayer::create(
			"Info",
			"This feature is coming soon!",
			"OK"
		)->show();
	}

    void onAbort(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

	void keyBackClicked() override {
		onAbort(nullptr);
	}

    void onCreate(CCObject*) {
		auto mod = Mod::get();
		mod->setSavedValue("target-level-id", targetLevelID);
		mod->setSavedValue("is-buffed", isBuffed);
		mod->setSavedValue("saw-rotation-speed", sawRotationSpeed);

		// log::info("ID={} | Buffed={} | Speed={}", targetLevelID, isBuffed, sawRotationSpeed);

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
				sawRotationSpeed
			);
		}

		this->removeFromParentAndCleanup(true);
	}


	void textChanged(CCTextInputNode* input) override {
		std::string text = input->getString();

		if (input == levelIDNode) {
			targetLevelID = text.empty() ? 0 : stoi(text);
		}
		else if (input == sawSpeedNode) {
			try {
				sawRotationSpeed = (text.empty() || text == "-") ? 0.f : stof(text);
			} catch (...) {
				sawRotationSpeed = 0.f;
			}
		}
	}
};



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

		log::info("Button added to left-side-menu");

		return true;
	}


    void onButton(CCObject*) {
		auto scene = CCDirector::sharedDirector()->getRunningScene();
		ComparisonMenu::create(
			[this](int levelID, bool isBuffed, float sawSpeed) {

				GameLevelManager* glm = GameLevelManager::sharedState();

				GJGameLevel* level1 = this->m_level;
				GJGameLevel* level2 = glm->getSavedLevel(levelID);

				ComparisonConfig config;
				config.isBuffed = isBuffed;
				config.sawSpeed = sawSpeed;

				std::string modifiedLevelString = createComparison(
					level1,
					level2,
					config
				);
				FLAlertLayer::create(
					"Level Comparison",
					fmt::format("Created comparison of {} and {}",
						level1->m_levelName.c_str(),
						level2->m_levelName.c_str()),
					"OK"
				)->show();
				
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
			}
		)->show();
    }
};

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config) {
	std::vector<GJGameLevel*> levels = { level1, level2 };
	bool first = !config.isBuffed;
	std::vector<std::string> levelStringSplit1, levelStringSplit2;
	std::string firstElement;

	for (GJGameLevel *level : levels) {
		std::string levelString = ZipUtils::decompressString(level->m_levelString, false, 0);
		std::vector<std::string> levelStringSplit = splitString(levelString, ";", true);
		firstElement = levelStringSplit.front();
		levelStringSplit.erase(levelStringSplit.begin());

		for (std::string& objectStr : levelStringSplit) {
			bool isDecoration = false;
			bool hasLayer1 = false; // 20
			bool hasColor1 = false; // 21
			bool hasColor2 = false; // 22
			bool hasLayer2 = false; // 61
			bool dontFade = false;  // 64
			bool dontEnter = false; // 67
			bool noGlow = false;    // 96
			bool customRotationSpeed = false; // 97
			bool disableRotation = false; // 98
			bool noParticle = false; // 507
			std::string newObjectStr = "";

			std::vector<std::string> splitStrings = splitString(objectStr, ",", true);
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < static_cast<int>(splitStrings.size()) - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}

			// log::info("Object before: {}", objectStr);

			for (std::vector<std::string>& pair : splitStringsPairs) {
				int propID = stoi(pair[0]);

				if (propID == 1) { // id
					if (pair[1] == "351") log::info("{}", splitStringsPairs);
					
					if (std::find(objects.begin(), objects.end(), stoi(pair[1])) == objects.end()) { // Check if object is decoration
						isDecoration = true;
						objectStr = "";
						break;
					} else {
						int objID = stoi(pair[1]);

						for (const auto& objPair : blackObjects) {
							if (objID == objPair[0]) {
								objID = objPair[1];
								pair[1] = std::to_string(objID);
								break;
							}
						}

						newObjectStr += "1," + pair[1] + ",";
						continue;
					}
				}

				if (isDecoration) break;

				switch (propID) {
					case 19: // delete 1.9 color
						continue;
					case 20: // editor layer 1
						hasLayer1 = true;
						first ? newObjectStr += "20,1," : newObjectStr += "20,2,";
						continue;
					case 21: // color 1
						hasColor1 = true;
						first ? newObjectStr += "21,1," : newObjectStr += "21,2,";
						continue;
					case 22: // color 2
						hasColor2 = true;
						first ? newObjectStr += "22,1," : newObjectStr += "22,2,";
						continue;
					case 43: // delete HSV 1
						continue;
					case 44: // delete HSV 2
						continue;
					case 61: // editor layer 2
						hasLayer2 = true;
						first ? newObjectStr += "61,1," : newObjectStr += "61,2,";
						continue;
					case 64: // don't fade
						dontFade = true;
						newObjectStr += "64,1,";
						continue;
					case 67: // don't enter
						dontEnter = true;
						newObjectStr += "67,1,";
						continue;
					case 96: // no glow
						noGlow = true;
						newObjectStr += "96,1,";
						continue;
					case 97: // custom rotation speed
						customRotationSpeed = true;
						if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,";
						continue;
					case 98: // turn off disable rotation
						disableRotation = true;
						if (config.sawSpeed == 0) newObjectStr += "98,1,";
						continue;
					case 103: // turn off high detail
						continue;
					case 135: // turn off hide
						continue;
					case 507: // no particle
						noParticle = true;
						newObjectStr += "507,1,";
						continue; 
					default:
						newObjectStr += pair[0] + "," + pair[1] + ",";
						continue;
				}
			}


			if (!hasLayer1) { first ? newObjectStr += "20,1," : newObjectStr += "20,2,"; }
			if (!hasColor1) { first ? newObjectStr += "21,1," : newObjectStr += "21,2,"; }
			if (!hasColor2) { first ? newObjectStr += "22,1," : newObjectStr += "22,2,"; }
			if (!hasLayer2) { first ? newObjectStr += "61,1," : newObjectStr += "61,2,"; }
			if (!dontFade) newObjectStr += "64,1,";
			if (!dontEnter) newObjectStr += "67,1,";
			if (!noGlow) newObjectStr += "96,1,";
			if (!customRotationSpeed) { if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,"; }
			if (!disableRotation) { if (config.sawSpeed == 0) newObjectStr += "98,1,"; }
			if (!noParticle) newObjectStr += "507,1,";

			if (!newObjectStr.empty()) {
				newObjectStr.pop_back(); // Remove the last comma
				objectStr = newObjectStr;
			} else {
				objectStr = "";
			}

			// log::info("Object after: {}", objectStr);
		}

		first ? levelStringSplit1 = levelStringSplit : levelStringSplit2 = levelStringSplit;
		first = !first;
	}

	std::vector<std::string> firstElementSplit = splitString(firstElement, ",", false);
	std::vector<std::vector<std::string>> firstElementPairs;
	for (int i = 0; i < static_cast<int>(firstElementSplit.size()) - 1; i += 2) {
		firstElementPairs.push_back({ firstElementSplit[i], firstElementSplit[i + 1] });
	}
	// log::info("{}", firstElement);
	// log::info("{}", firstElementPairs);

	std::string newFirstElement;
	for (std::vector<std::string>& pair : firstElementPairs) {
		if (pair[0] == "kA6") { // background texture
			newFirstElement += "kA6,13,";
		}
		else if (pair[0] == "kA7") { // ground texture
			newFirstElement += "kA7,1,";
		}
		else if (pair[0] == "kA17") { // ground line
			newFirstElement += "kA17,1,";
		}
		else if (pair[0] == "kA18") { // font
			newFirstElement += "kA18,6,";
		}
		else if (pair[0] == "kA25") { // middleground texture
			newFirstElement += "kA25,0,";
		}
		else if (pair[0] == "kS38") { // colors
			newFirstElement += "kS38,1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1000_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1001_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1009_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1002_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1013_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1014_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1004_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1007_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1003_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1012_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1010_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1011_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1_5_1_7_1.000000_15_1.000000_9_3_10_180.000000a1.000000a1.000000a1a1_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_2_5_1_7_1.000000_15_1.000000_9_3_10_0.000000a1.000000a1.000000a1a1_18_0_8_1|,";
		}
		else if (pair[0] == "kS39") { // color page
			newFirstElement += "kS39,0,";
		}
		else {
			newFirstElement += pair[0] + "," + pair[1] + ",";
		}
	}
	
	std::string modifiedLevelString = newFirstElement + ";" + joinString(levelStringSplit1, ";") + joinString(levelStringSplit2, ";");
	return modifiedLevelString;
}

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty) {
    std::vector<std::string> splitStrings;
    size_t pos = 0, found;
    while ((found = s.find(delimiter, pos)) != std::string::npos) {
        std::string token = s.substr(pos, found - pos);
        if (!skipEmpty || !token.empty()) {
            splitStrings.push_back(token);
        }
        pos = found + delimiter.length();
    }
    // Handle the last token
    std::string lastToken = s.substr(pos);
    if (!skipEmpty || !lastToken.empty()) {
        splitStrings.push_back(lastToken);
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

int stoi(std::string& str) {
	return utils::numFromString<int>(str).unwrapOr(0);
}

int stof(std::string& str) {
	return utils::numFromString<float>(str).unwrapOr(0);
}