#pragma once

#include <set>
#include <string>

struct clause {
    std::set<int> lits;

    bool operator<(const clause& other) const;
};

struct cnf {
    std::set<clause> cls;

    void add_clause(const clause& cl);
};

std::string literal_to_string(int n);
int negate_literal(int lit);
void cnf_debug(const cnf& f);