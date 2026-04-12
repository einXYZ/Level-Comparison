struct ComparisonConfig {
    bool isBuffed;
    float sawSpeed;
    bool remapGroups;
    bool unhideObjects;
    bool showModifiers;
};

std::vector<int> groupKeys = {
    33, 57, 274, 108, 51, 71, 76, 395, 442, 516, 517, 518, 519, 448, 455, 457
};

static const std::map<int, std::string> modifierLetters = {
		{1755, "RA=="}, {1813, "Sg=="}, {1829, "Uw=="}, {1859, "SA=="}, {2866, "Rg=="}
	};