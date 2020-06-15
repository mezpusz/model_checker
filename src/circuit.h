#pragma once

#include <set>
#include <map>

struct circuit {
    int M;
    std::set<int> inputs;
    std::map<int, int> latches;
    std::map<std::pair<int, int>, int> ands;
    std::set<int> outputs;
};