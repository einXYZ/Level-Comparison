#pragma once
#include <string>
#include <vector>

namespace lc {
    std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty);
    std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter);
    int stoi(std::string& str);
    float stof(std::string& str);
}
