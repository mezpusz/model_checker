#include "formula.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <functional>

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

uint64_t negate_literal(uint64_t lit) {
    return (lit%2 == 0) ? (lit + 1) : (lit - 1);
}

std::ostream& operator<<(std::ostream& out, const clause& cl) {
    out << "(";
    for (auto it = cl.begin(); it != cl.end();) {
        out << literal_to_string(*it);
        if (++it != cl.end()) {
            out << " | ";
        }
    }
    out << ")";
    return out;
}

std::ostream& operator<<(std::ostream& out, const Cnf& cnf) {
    out << "(";
    for (auto it = cnf.begin(); it != cnf.end();) {
        out << *it;
        if (++it != cnf.end()) {
            out << " & ";
        }
    }
    out << ")";
    return out;
}

Cnf to_cnf_or(const Cnf& lhs, const Cnf& rhs) {
    Cnf res;
    for (const auto& cl1 : lhs) {
        // bool t = false;
        // for (const auto& lit : cl1) {
        //     if (cl1.count(negate_literal(lit))) {
        //         t = true;
        //         break;
        //     }
        // }
        // if (t) {
        //     continue;
        // }
        for (const auto& cl2 : rhs) {
            // if (lhs.size() > 1 && rhs.size() > 1 &&
            //     (std::includes(cl1.begin(), cl1.end(), cl2.begin(), cl2.end())
            //     || std::includes(cl2.begin(), cl2.end(), cl1.begin(), cl1.end()))) {
            //     break;
            // }
            auto cl = cl1;
            cl.insert(cl2.begin(), cl2.end());
            res.insert(std::move(cl));
        }
    }
    return res;
}

void clean(Cnf& cnf) {
    Cnf res;
    bool okay;
    for (auto it = cnf.begin(); it != cnf.end(); it++) {
        okay = true;
        for (const auto& lit : *it) {
            if (it->count(negate_literal(lit))) {
                okay = false;
                break;
            }
        }
        if (!okay) {
            break;
        }
        for (auto it2 = cnf.begin(); it2 != cnf.end(); it2++) {
            if (it != it2 && std::includes(it2->begin(), it2->end(), it->begin(), it->end())) {
                okay = false;
                break;
            }
        }
        if (okay) {
            res.insert(*it);
        }
    }
    cnf = res;
}
