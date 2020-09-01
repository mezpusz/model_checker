#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <cassert>

using clause = std::set<uint64_t>;

class less {
public:
    bool operator()(const clause& lhs, const clause& rhs) {
        if (std::includes(rhs.begin(), rhs.end(), lhs.begin(), lhs.end())) {
            return false;
        }
        return std::less<clause>()(lhs, rhs);
    }
};

using Cnf = std::set<clause, less>;

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
void merge(Cnf& lhs, const Cnf& rhs);
void add_equiv(Cnf& cnf, const conjunction& conj1, const conjunction& conj2);
Cnf duplicate(const Cnf& cnf, uint64_t shift);

// misc
uint64_t negate_literal(uint64_t lit);
std::ostream& operator<<(std::ostream& out, const clause& cl);
std::ostream& operator<<(std::ostream& out, const Cnf& cnf);
Cnf to_cnf_or(Cnf& lhs, Cnf& rhs);
void clean(Cnf& cnf);
