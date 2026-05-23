#include "ComparisonLogic.hpp"
#include "data.hpp"
#include "utils.hpp"
#include <Geode/Geode.hpp>
#include <map>
#include <set>

using namespace geode::prelude;
using namespace lc;

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config) {
	std::vector<GJGameLevel*> levels = { level1, level2 };
	bool first = !config.level1Buffed;
	std::vector<std::string> levelStringSplit1, levelStringSplit2;
	std::string firstElement;
	const std::string kS38 = "kS38,1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1000_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1001_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1009_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1002_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1013_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1014_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1004_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1007_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1003_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1012_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1010_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1011_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1_5_1_7_1.000000_15_1.000000_9_3_10_180.000000a1.000000a1.000000a1a1_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_2_5_1_7_1.000000_15_1.000000_9_3_10_0.000000a1.000000a1.000000a1a1_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_4_5_1_7_1.000000_15_1.000000_18_0_8_1|,";

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
	if (!config.unhide.unhideAlpha) effectiveObjects.push_back(1007);

	std::set<int> enabledModifiers;
	if (config.modifiers.showDBlocks) enabledModifiers.insert(1755);
	if (config.modifiers.showJBlocks) enabledModifiers.insert(1813);
	if (config.modifiers.showSBlocks) enabledModifiers.insert(1829);
	if (config.modifiers.showHBlocks) enabledModifiers.insert(1859);
	if (config.modifiers.showFBlocks) enabledModifiers.insert(2866);
	for (int id : enabledModifiers) effectiveObjects.push_back(id);

	int levelIndex = 0;

	for (GJGameLevel *level : levels) {
		bool isSecondLevel = (levelIndex == 1);
		std::string levelString = ZipUtils::decompressString(level->m_levelString, false, 0);
		std::vector<std::string> levelStringSplit = splitString(levelString, ";", true);
		firstElement = levelStringSplit.front();
		levelStringSplit.erase(levelStringSplit.begin());

		for (std::string& objectStr : levelStringSplit) {
			bool isDecoration = false;
			bool isDisappearing = false;
			bool hasLayer1 = false;
			bool hasColor1 = false;
			bool hasColor2 = false;
			bool hasLayer2 = false;
			bool dontFade = false;
			bool dontEnter = false;
			bool noGlow = false;
			bool customRotationSpeed = false;
			bool disableRotation = false;
			bool noParticle = false;
			bool noEffects = false;
			bool noAudioScale = false;
			std::string newObjectStr = "";

			std::vector<std::string> splitStrings = splitString(objectStr, ",", true);
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < static_cast<int>(splitStrings.size()) - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}

			for (std::vector<std::string>& pair : splitStringsPairs) {
				int propID = stoi(pair[0]);

				if (propID == 1) {
					if (std::find(effectiveObjects.begin(), effectiveObjects.end(), stoi(pair[1])) == effectiveObjects.end()) {
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

						if (config.replaceDisappearing) {
							for (int id : disappearingObjects) {
								if (id == objID) {
									isDisappearing = true;
									break;
								}
							}
						}

						newObjectStr += "1," + pair[1] + ",";
						continue;
					}
				}

				if (isDecoration) break;

				switch (propID) {
					case 19:
						continue;
					case 20:
						hasLayer1 = true;
						first ? newObjectStr += "20,1," : newObjectStr += "20,2,";
						continue;
					case 21:
						hasColor1 = true;
						first ? newObjectStr += "21,1," : newObjectStr += "21,2,";
						continue;
					case 22:
						hasColor2 = true;
						first ? newObjectStr += "22,1," : newObjectStr += "22,2,";
						continue;
					case 43:
						continue;
					case 44:
						continue;
					case 61:
						hasLayer2 = true;
						first ? newObjectStr += "61,1," : newObjectStr += "61,2,";
						continue;
					case 64:
						dontFade = true;
						if (config.objectOptions.dontFade) newObjectStr += "64,1,";
						continue;
					case 67:
						dontEnter = true;
						if (config.objectOptions.dontEnter) newObjectStr += "67,1,";
						continue;
					case 96:
						noGlow = true;
						if (config.objectOptions.noGlow) newObjectStr += "96,1,";
						continue;
					case 97:
						customRotationSpeed = true;
						if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,";
						continue;
					case 98:
						disableRotation = true;
						if (config.sawSpeed == 0) newObjectStr += "98,1,";
						continue;
					case 103:
						continue;
					case 116:
						noEffects = true;
						if (config.objectOptions.noEffects) newObjectStr += "116,1,";
						continue;
					case 121:
						newObjectStr = "";
						isDecoration = true;
						objectStr = "";
						break;
					case 135:
						if (!config.unhide.unhideHide)
							newObjectStr += pair[0] + "," + pair[1] + ",";
						continue;
					case 372:
						noAudioScale = true;
						if (config.objectOptions.noAudioScale) newObjectStr += "372,1,";
						continue;
					case 507:
						noParticle = true;
						if (config.objectOptions.noParticle) newObjectStr += "507,1,";
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
			if (!dontFade) { if (config.objectOptions.dontFade) newObjectStr += "64,1,"; }
			if (!dontEnter) { if (config.objectOptions.dontEnter) newObjectStr += "67,1,"; }
			if (!noGlow) { if (config.objectOptions.noGlow) newObjectStr += "96,1,"; }
			if (!customRotationSpeed) { if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,"; }
			if (!disableRotation) { if (config.sawSpeed == 0) newObjectStr += "98,1,"; }
			if (!noEffects) { if (config.objectOptions.noEffects) newObjectStr += "116,1,"; }
			if (!noParticle) { if (config.objectOptions.noParticle) newObjectStr += "507,1,"; }
			if (!noAudioScale) { if (config.objectOptions.noAudioScale) newObjectStr += "372,1,"; }

			if (!newObjectStr.empty()) {
				newObjectStr.pop_back();
				if (isDisappearing)
					newObjectStr = replaceDisappearing(newObjectStr);
				objectStr = newObjectStr;
			} else {
				objectStr = "";
			}
		}

		levelIndex++;
		first ? levelStringSplit1 = levelStringSplit : levelStringSplit2 = levelStringSplit;
		first = !first;
	}

	auto parseProps = [&](const std::string& obj) -> std::map<int, std::string> {
		std::map<int, std::string> props;
		std::vector<std::string> tokens = splitString(obj, ",", true);
		for (int i = 0; i + 1 < static_cast<int>(tokens.size()); i += 2)
			props[stoi(tokens[i])] = tokens[i + 1];
		return props;
	};

	auto getFloat = [](std::map<int, std::string>& p, int k) -> float {
		auto it = p.find(k);
		return it == p.end() ? 0.f : lc::stof(it->second);
	};

	struct PortalEntry { size_t idx; std::map<int, std::string> props; };
	std::vector<PortalEntry> portals1, portals2;

	for (size_t i = 0; i < levelStringSplit1.size(); i++) {
		if (levelStringSplit1[i].empty()) continue;
		auto p = parseProps(levelStringSplit1[i]);
		if (!p.count(1) || std::find(portals.begin(), portals.end(), stoi(p[1])) == portals.end()) continue;
		portals1.push_back({ i, p });
	}

	for (size_t i = 0; i < levelStringSplit2.size(); i++) {
		if (levelStringSplit2[i].empty()) continue;
		auto p = parseProps(levelStringSplit2[i]);
		if (!p.count(1) || std::find(portals.begin(), portals.end(), stoi(p[1])) == portals.end()) continue;
		portals2.push_back({ i, p });
	}

	for (PortalEntry& e2 : portals2) {
		for (PortalEntry& e1 : portals1) {
			if (stoi(e1.props[1]) != stoi(e2.props[1])) continue;
			bool match = true;
			for (int k : portalCompareKeys)
				if (std::abs(getFloat(e1.props, k) - getFloat(e2.props, k)) > 0.001f) { match = false; break; }
			if (!match) continue;

			std::string newPortalString;
			std::vector<std::string> tokens = splitString(levelStringSplit1[e1.idx], ",", true);
			for (int i = 0; i + 1 < static_cast<int>(tokens.size()); i += 2) {
				int k = stoi(tokens[i]);
				if (k == 21 || k == 22) continue;
				newPortalString += tokens[i] + "," + tokens[i + 1] + ",";
			}
			if (!newPortalString.empty()) newPortalString.pop_back();
			levelStringSplit1[e1.idx] = newPortalString;
			levelStringSplit2[e2.idx] = "";
			break;
		}
	}

	if (!enabledModifiers.empty()) {
		static const auto keyValueIfSet = [](int key, const std::string& val) {
			return val.empty() ? "" : "," + std::to_string(key) + "," + val;
		};

		auto replaceModifiers = [&](std::vector<std::string>& split) {
			for (std::string& obj : split) {
				if (obj.empty()) continue;

				std::vector<std::string> tokens = splitString(obj, ",", true);
				int objID = 0;
				std::string x, y, h, v, r, layer, c1, scale, scaleX, scaleY, warpX, warpY;

				for (int i = 0; i + 1 < static_cast<int>(tokens.size()); i += 2) {
					switch (stoi(tokens[i])) {
						case 1: objID = stoi(tokens[i + 1]); break;
						case 2: x = tokens[i + 1]; break;
						case 3: y = tokens[i + 1]; break;
						case 4: h = tokens[i + 1]; break;
						case 5: v = tokens[i + 1]; break;
						case 6: r = tokens[i + 1]; break;
						case 20: layer = tokens[i + 1]; break;
						case 21: c1 = tokens[i + 1]; break;
						case 32: scale = tokens[i + 1]; break;
						case 128: scaleX = tokens[i + 1]; break;
						case 129: scaleY = tokens[i + 1]; break;
						case 131: warpY = tokens[i + 1]; break;
						case 132: warpX = tokens[i + 1]; break;
					}
				}

				if (!enabledModifiers.count(objID)) continue;

				float baseScale = scale.empty() ? 1.f : lc::stof(scale);
				std::string halfScale = fmt::format("{:.6f}", baseScale * 0.5f);
				auto halved = [](const std::string& s) {
					return s.empty() ? "" : fmt::format("{:.6f}", lc::stof(const_cast<std::string&>(s)) * 0.5f);
				};

				std::string base = ",2," + x + ",3," + y + ",20," + layer + ",21," + c1;
				std::string flip = keyValueIfSet(4, h) + keyValueIfSet(5, v) + keyValueIfSet(6, r);
				std::string warp = keyValueIfSet(128, scaleX) + keyValueIfSet(129, scaleY) + keyValueIfSet(131, warpY) + keyValueIfSet(132, warpX);
				std::string txtWarp = keyValueIfSet(128, halved(scaleX)) + keyValueIfSet(129, halved(scaleY)) + keyValueIfSet(131, warpY) + keyValueIfSet(132, warpX);

				std::string box = "1,467" + base + ",22," + c1 + ",64,1,67,1,96,1,98,1,507,1,121,1" + keyValueIfSet(32, scale) + flip + warp;
				std::string txt = "1,914" + base + ",31," + modifierLetters.at(objID) + ",32," + halfScale + ",64,1,67,1,96,1,121,1" + flip + txtWarp;

				obj = obj + ";" + box + ";" + txt;
			}
		};

		replaceModifiers(levelStringSplit1);
		replaceModifiers(levelStringSplit2);
	}

	std::vector<std::string> firstElementSplit = splitString(firstElement, ",", false);
	std::vector<std::vector<std::string>> firstElementPairs;
	for (int i = 0; i < static_cast<int>(firstElementSplit.size()) - 1; i += 2) {
		firstElementPairs.push_back({ firstElementSplit[i], firstElementSplit[i + 1] });
	}

	std::string newFirstElement;
	for (std::vector<std::string>& pair : firstElementPairs) {
		if (pair[0] == "kA6") {
			newFirstElement += "kA6,13,";
		}
		else if (pair[0] == "kA7") {
			newFirstElement += "kA7,1,";
		}
		else if (pair[0] == "kA17") {
			newFirstElement += "kA17,1,";
		}
		else if (pair[0] == "kA18") {
			newFirstElement += "kA18,0,";
		}
		else if (pair[0] == "kA25") {
			newFirstElement += "kA25,0,";
		}
		else if (pair[0] == "kS38") {
			newFirstElement += kS38;
		}
		else if (pair[0] == "kS39") {
			newFirstElement += "kS39,0,";
		}
		else if (pair[0].size() >= 3 && pair[0].substr(0, 2) == "kS") {
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

	std::string modifiedLevelString = newFirstElement + ";" + joinString(levelStringSplit1, ";") + ";" + joinString(levelStringSplit2, ";");
	return modifiedLevelString;
}

std::string replaceDisappearing(const std::string& objStr) {
    std::vector<std::string> tokens = splitString(objStr, ",", true);

    int objID = 0;
    for (int i = 0; i + 1 < (int)tokens.size(); i += 2) {
        if (stoi(tokens[i]) == 1) { objID = stoi(tokens[i + 1]); break; }
    }

    int newID = 0;
    bool blackBlending = false;

    switch (objID) {
        case 144: newID = 216; blackBlending = true; break;
        case 205: newID = 217; blackBlending = true; break;
        case 145: newID = 218; blackBlending = true; break;
        case 459: newID = 458; blackBlending = true; break;
        case 147: newID = 215; blackBlending = true; break;
        case 204: newID = 219; blackBlending = true; break;
        case 146: newID = 467; break;
        case 206: newID = 661; break;
        case 673: newID = 1338; break;
        case 674: newID = 1339; break;
        case 1341: newID = 1338; break;
        case 1342: newID = 1339; break;
        case 1344: newID = 1338; break;
        case 1345: newID = 1339; break;
        case 740: newID = 186; break;
        case 741: newID = 187; break;
        case 742: newID = 188; break;
        default: return objStr;
    }

    std::string result;
    bool hasColor2 = false;

    for (int i = 0; i + 1 < (int)tokens.size(); i += 2) {
        int k = stoi(tokens[i]);
        if (k == 1) {
            result += "1," + std::to_string(newID) + ",";
        } else if (k == 22) {
            hasColor2 = true;
            if (blackBlending)
                result += "22,4,";
            else
                result += "22," + tokens[i + 1] + ",";
        } else {
            result += tokens[i] + "," + tokens[i + 1] + ",";
        }
    }

    if (!hasColor2 && blackBlending)
        result += "22,4,";

    if (!result.empty()) result.pop_back();
    return result;
}