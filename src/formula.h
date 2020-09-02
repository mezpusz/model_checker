#pragma once

#include <set>
#include <string>
#include <vector>
#include <cassert>

using clause = std::set<uint64_t>;
using Cnf = std::set<clause>;

std::string literal_to_string(uint64_t n);

struct conjunction {
    std::vector<uint64_t> c;

    conjunction(uint64_t one) : c() {
        c.push_back(one);
    }
    conjunction(uint64_t one, uint64_t two) : c() {
        c.push_back(one);
        c.push_back(two);
    }
};

// cnf
Cnf duplicate(const Cnf& cnf, int shift);

// misc
uint64_t negate_literal(uint64_t lit);
std::ostream& operator<<(std::ostream& out, const clause& cl);
std::ostream& operator<<(std::ostream& out, const Cnf& cnf);
Cnf to_cnf_or(Cnf& lhs, Cnf& rhs);
void clean(Cnf& cnf);
