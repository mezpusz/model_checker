#pragma once

#include <set>
#include <string>

struct clause {
    std::set<uint64_t> lits;

    bool operator<(const clause& other) const;
};

struct conjunction {
    std::set<uint64_t> c;

    conjunction(uint64_t one) : c() {
        c.insert(one);
    }
    conjunction(uint64_t one, uint64_t two) : c() {
        c.insert(one);
        c.insert(two);
    }
};

struct cnf {
    std::set<clause> cls;

    void add_clause(const clause& cl);
    void merge(const cnf& other);
    void add_equiv(const conjunction& conj1, const conjunction& conj2);
    cnf duplicate(uint64_t shift) const;
};

std::string literal_to_string(uint64_t n);
uint64_t negate_literal(uint64_t lit);
void cnf_debug(const cnf& f);