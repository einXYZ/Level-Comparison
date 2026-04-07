#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
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

class ComparisonMenu : public geode::Popup {
public:
	std::function<void(
		int targetLevelID,
		bool isBuffed,
		float sawRotationSpeed,
		bool remapGroups,
		bool unhideInvisible
	)> onCreateCallback;

    int targetLevelID = 0;
    bool isBuffed = false;
    float sawRotationSpeed = 0.f;
	bool remapGroups = true;
	bool unhideInvisible = true;

    CCLabelBMFont* speedLabel = nullptr;
    CCMenuItemToggler* buffedToggle = nullptr;
    CCMenuItemToggler* nerfedToggle = nullptr;
	CCMenuItemToggler* remapToggle = nullptr;
	CCMenuItemToggler* unhideToggle = nullptr;


    static ComparisonMenu* create(std::function<void(int, bool, float, bool, bool)> onCreate) {
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
        if (!Popup::init(360.f, 260.f)) return false;
		this->setTouchEnabled(true);
		this->setKeypadEnabled(true);
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto mod = Mod::get();
        targetLevelID = mod->getSavedValue<int>("target-level-id", 0);
        isBuffed = mod->getSavedValue<bool>("is-buffed", false);
        sawRotationSpeed = mod->getSavedValue<float>("saw-rotation-speed", 0.f);
		remapGroups = mod->getSavedValue<bool>("remap-groups", true);
		unhideInvisible = mod->getSavedValue<bool>("unhide-invisible", true);

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

        // level id input
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
			targetLevelID = text.empty() ? 0 : stoi(const_cast<std::string&>(text));
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


        // saw speed
        auto speedText = CCLabelBMFont::create("Saw Rotation", "goldFont.fnt");
        speedText->setPosition({ 80.f, 155.f });
		speedText->setScale(0.8);
        panel->addChild(speedText);

		infoSprite->setScale(0.5f);

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
			sawRotationSpeed = stof(const_cast<std::string&>(text));
		});
        panel->addChild(sawSpeedInput);
		
		// remap groups
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

		// unhide invisible
		unhideToggle = CCMenuItemToggler::createWithStandardSprites(
			this,
			menu_selector(ComparisonMenu::onUnhide),
			0.8f
		);
		unhideToggle->setPosition({ 270.f, 130.f });
		unhideToggle->toggle(unhideInvisible);
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

	void onRemap(CCObject*) {
		remapGroups = !remapToggle->m_toggled;
		log::info("{}", remapGroups);
	}

	void onUnhide(CCObject*) {
		unhideInvisible = !unhideToggle->m_toggled;
	}

	void onBuffedNerfedInfo(CCObject*) {
		FLAlertLayer::create(
			"Info",
			"Select whether currently opened level is the <cj>nerfed</c> or <cr>buffed</c> version.",
			"OK"
		)->show();
	}

	void onSawRotationInfo(CCObject*) {
		FLAlertLayer::create(
			"Saw Rotation",
			"Sets the rotation speed of <cy>saw blade objects</c> in the comparison. Set to <cg>0</c> to disable saw rotation.",
			"OK"
		)->show();
	}

	void onRemapGroupsInfo(CCObject*) {
		FLAlertLayer::create(
			"Remap Groups",
			"<cy>Common Group IDs</c> used by both levels will be remapped so they don't interfere each other. <cg>Recommended</c> to be always checked.",
			"OK"
		)->show();
	}

	void onUnhideInfo(CCObject*) {
		FLAlertLayer::create(
			"Unhide Objects",
			"Deletes <cy>alpha triggers</c> and removes the <cy>Hide</c> checkmark from every object. <cg>Recommended</c> to be checked to see all objects.",
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
		mod->setSavedValue("unhide-invisible", unhideInvisible);

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
				sawRotationSpeed,
				remapGroups,
				unhideInvisible
			);
		}

		this->removeFromParentAndCleanup(true);
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
			[this](int levelID, bool isBuffed, float sawSpeed, bool remapGroups, bool unhideInvisible) {

				GameLevelManager* glm = GameLevelManager::sharedState();

				GJGameLevel* level1 = this->m_level;
				GJGameLevel* level2 = glm->getSavedLevel(levelID);

				ComparisonConfig config;
				config.isBuffed = isBuffed;
				config.sawSpeed = sawSpeed;
				config.remapGroups = remapGroups;
				config.unhideInvisible = unhideInvisible;

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
				newLevel->m_audioTrack = level1->m_audioTrack;
				newLevel->m_levelLength = level1->m_levelLength;
			}
		)->show();
    }
};

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config) {
	std::vector<GJGameLevel*> levels = { level1, level2 };
	bool first = !config.isBuffed;
	std::vector<std::string> levelStringSplit1, levelStringSplit2;
	std::string firstElement;
	const std::string kS38 = "kS38,1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1000_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1001_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1009_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1002_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1013_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1014_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1004_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1007_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1003_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1012_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1010_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1011_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1_5_1_7_1.000000_15_1.000000_9_3_10_180.000000a1.000000a1.000000a1a1_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_2_5_1_7_1.000000_15_1.000000_9_3_10_0.000000a1.000000a1.000000a1a1_18_0_8_1|,";
 
	std::map<int, int> groupRemap;
	if (config.remapGroups) {
		std::set<int> usedInLevel1;
		std::set<int> usedInLevel2;
 
		auto collectGroups = [&](GJGameLevel* lvl, std::set<int>& outSet) {
			std::string ls = ZipUtils::decompressString(lvl->m_levelString, false, 0);
			std::vector<std::string> parts = splitString(ls, ";", true);
			parts.erase(parts.begin());
			for (std::string& obj : parts) {
				std::vector<std::string> tokens = splitString(obj, ",", true);
				for (int i = 0; i < static_cast<int>(tokens.size()) - 1; i += 2) {
					int key = stoi(tokens[i]);
					if (std::find(groupKeys.begin(), groupKeys.end(), key) != groupKeys.end()) {
						if (key == 57 || key == 274 || key == 442) {
							for (std::string& idStr : splitString(tokens[i + 1], ".", true)) {
								int val = stoi(idStr);
								if (val > 0) outSet.insert(val);
							}
						} else {
							int val = stoi(tokens[i + 1]);
							if (val > 0) outSet.insert(val);
						}
					}
				}
			}
		};
 
		collectGroups(levels[0], usedInLevel1);
		collectGroups(levels[1], usedInLevel2);
 
		int nextFree = 1;
		for (int id : usedInLevel2) {
			while (usedInLevel1.count(nextFree)) nextFree++;
			groupRemap[id] = nextFree++;
		}
	}
	std::vector<int> effectiveObjects = objects;
	if (!config.unhideInvisible) effectiveObjects.push_back(1007);
 
	int levelIndex = 0;
 
	for (GJGameLevel *level : levels) {
		bool isSecondLevel = (levelIndex == 1);
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
					if (std::find(effectiveObjects.begin(), effectiveObjects.end(), stoi(pair[1])) == effectiveObjects.end()) { // Check if object is decoration
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
						if (!config.unhideInvisible) {
							newObjectStr += pair[0] + "," + pair[1] + ",";
						}
						continue;
					case 507: // no particle
						noParticle = true;
						newObjectStr += "507,1,";
						continue;
					case 33: case 51: case 71: case 76: case 108:
					case 395: case 448: case 455: case 457:
					case 516: case 517: case 518: case 519:
						if (config.remapGroups && isSecondLevel && !groupRemap.empty()) {
							int val = stoi(pair[1]);
							if (val > 0 && groupRemap.count(val)) {
								newObjectStr += pair[0] + "," + std::to_string(groupRemap[val]) + ",";
								continue;
							}
						}
						newObjectStr += pair[0] + "," + pair[1] + ",";
						continue;
					case 57: case 274: case 442:
						if (config.remapGroups && isSecondLevel && !groupRemap.empty()) {
							std::vector<std::string> ids = splitString(pair[1], ".", true);
							std::string remapped;
							for (std::string& idStr : ids) {
								int val = stoi(idStr);
								if (val > 0 && groupRemap.count(val))
									remapped += std::to_string(groupRemap[val]) + ".";
								else
									remapped += idStr + ".";
							}
							if (!remapped.empty()) remapped.pop_back();
							newObjectStr += pair[0] + "," + remapped + ",";
							continue;
						}
						newObjectStr += pair[0] + "," + pair[1] + ",";
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
 
		levelIndex++;
		first ? levelStringSplit1 = levelStringSplit : levelStringSplit2 = levelStringSplit;
		first = !first;
	}
 
	std::vector<std::string> firstElementSplit = splitString(firstElement, ",", false);
	std::vector<std::vector<std::string>> firstElementPairs;
	for (int i = 0; i < static_cast<int>(firstElementSplit.size()) - 1; i += 2) {
		firstElementPairs.push_back({ firstElementSplit[i], firstElementSplit[i + 1] });
	}
	// log::info("firstElement: {}", firstElement);
	// log::info("firstElementPairs: {}", firstElementPairs);
 
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
			newFirstElement += kS38;
		}
		else if (pair[0] == "kS39") { // color page
			newFirstElement += "kS39,0,";
		}
		else if (pair[0].size() >= 3 && pair[0].substr(0, 2) == "kS") { // (pre) 1.9 color channels
			int n = stoi(pair[0].substr(2));
			if (!((n >= 1 && n <= 20) || (n >= 29 && n <= 37))) {
				newFirstElement += pair[0] + "," + pair[1] + ",";
			}
		}
		else {
			newFirstElement += pair[0] + "," + pair[1] + ",";
		}
	}
 
	if (newFirstElement.find("kS38,") == std::string::npos) {
		newFirstElement += kS38;
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