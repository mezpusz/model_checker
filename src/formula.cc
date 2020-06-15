#include "formula.h"

#include <iostream>
#include <sstream>

std::string literal_to_string(int n) {
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

int negate_literal(int lit) {
    return (lit%2 == 0) ? (lit + 1) : (lit - 1);
}

void cnf::add_clause(const clause& cl) {
    cls.insert(cl);
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
