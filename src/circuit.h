#pragma once

#include <vector>
#include <tuple>

struct circuit {
    uint64_t M;
    std::vector<uint64_t> inputs;
    std::vector<std::pair<uint64_t, uint64_t>> latches;
    std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> ands;
    std::vector<uint64_t> outputs;

    uint64_t shift() const { return 2*M; }
};
