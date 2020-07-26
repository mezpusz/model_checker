#pragma once

#include <set>
#include <string>

struct clause {
    std::set<int> lits;

    bool operator<(const clause& other) const;
};

struct conjunction {
    std::set<int> c;

    conjunction(int one) : c() {
        c.insert(one);
    }
    conjunction(int one, int two) : c() {
        c.insert(one);
        c.insert(two);
    }
};

struct cnf {
    std::set<clause> cls;

    void add_clause(const clause& cl);
    void merge(const cnf& other);
};

std::string literal_to_string(int n);
int negate_literal(int lit);
void cnf_debug(const cnf& f);