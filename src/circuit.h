#pragma once

#include <set>
#include <map>
#include <sstream>
#include <iostream>

struct circuit {
    uint64_t M;
    std::set<uint64_t> inputs;
    std::set<std::pair<uint64_t, uint64_t>> latches;
    std::set<std::tuple<uint64_t, uint64_t, uint64_t>> ands;
    std::set<uint64_t> outputs;

    uint64_t shift() const { return 2*M; }
};

void circuit_debug(const circuit& c);
