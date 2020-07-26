#include "formula.h"

#include <cassert>
#include <iostream>
#include <sstream>

std::string literal_to_string(uint64_t n) {
    if (n == 0) {
        return "T";
    }
    if (n == 1) {
        return "F";
    }
    std::stringstream str;
    str << ((n%2==1) ? "~" : "") << "x" << n/2;
    return str.str();
}

bool clause::operator<(const clause& other) const {
    return lits < other.lits;
}

uint64_t negate_literal(uint64_t lit) {
    return (lit%2 == 0) ? (lit + 1) : (lit - 1);
}

void cnf::add_clause(const clause& cl) {
    cls.insert(cl);
}

void cnf::merge(const cnf& other) {
    cls.insert(other.cls.begin(), other.cls.end());
    // for (const auto& cl : other.cls) {
    //     cls.insert(cl);
    // }
}

void cnf::add_equiv(const conjunction& conj1, const conjunction& conj2) {
    clause cl;
    for (const auto& l : conj1.c) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj2.c) {
        assert(cl.lits.count(l) == 0);
        cl.lits.insert(l);
        cls.insert(cl);
        cl.lits.erase(l);
    }
    cl.lits.clear();
    for (const auto& l : conj2.c) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj1.c) {
        assert(cl.lits.count(l) == 0);
        cl.lits.insert(l);
        cls.insert(cl);
        cl.lits.erase(l);
    }
}

cnf cnf::duplicate(uint64_t shift) const {
    cnf res;
    for (const auto& cl : cls) {
        clause cl_n;
        for (const auto& lit : cl.lits) {
            cl_n.lits.insert(lit + shift);
        }
        res.add_clause(cl_n);
    }
    return res;
}

void cnf_debug(const cnf& c) {
    std::stringstream str;
    for (auto it = c.cls.begin(); it != c.cls.end();) {
        str << "(";
        for (auto it2 = it->lits.begin(); it2 != it->lits.end();) {
            str << literal_to_string(*it2);
            it2++;
            if (it2 != it->lits.end()) {
                str << " \\/ ";
            }
        }
        str << ")";
        it++;
        if (it != c.cls.end()) {
            str << " /\\ ";
        }
    }
    std::cout << str.str() << std::endl;
}
