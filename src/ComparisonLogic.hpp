#pragma once
#include "data.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config);
std::string replaceDisappearing(const std::string& objStr);