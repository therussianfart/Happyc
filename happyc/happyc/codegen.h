#pragma once
#include "parser.h"
#include <string>

void generateCpp(const std::vector<NodePtr>& ast, const std::string& outPath);
