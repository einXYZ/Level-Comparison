#include "utils.hpp"
#include <Geode/Geode.hpp>
#include <sstream>

using namespace geode::prelude;

namespace lc {

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty) {
    std::vector<std::string> splitStrings;
    size_t pos = 0, found;
    while ((found = s.find(delimiter, pos)) != std::string::npos) {
        std::string token = s.substr(pos, found - pos);
        if (!skipEmpty || !token.empty())
            splitStrings.push_back(token);
        pos = found + delimiter.length();
    }
    std::string lastToken = s.substr(pos);
    if (!skipEmpty || !lastToken.empty())
        splitStrings.push_back(lastToken);
    return splitStrings;
}

std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < elems.size(); ++i) {
        if (i != 0) ss << delimiter;
        ss << elems[i];
    }
    return ss.str();
}

int stoi(std::string& str) {
    return utils::numFromString<int>(str).unwrapOr(0);
}

float stof(std::string& str) {
    return utils::numFromString<float>(str).unwrapOr(0);
}

}
