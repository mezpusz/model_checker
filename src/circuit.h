#pragma once

#include <set>
#include <map>
#include <sstream>
#include <iostream>

struct circuit {
    int M;
    std::set<int> inputs;
    std::map<int, int> latches;
    std::map<std::pair<int, int>, int> ands;
    std::set<int> outputs;
};

void circuit_debug(const circuit& c);