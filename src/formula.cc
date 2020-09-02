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

uint64_t negate_literal(uint64_t lit) {
    return (lit%2 == 0) ? (lit + 1) : (lit - 1);
}

void merge(Cnf& lhs, const Cnf& rhs) {
    // if (lhs.count(cl) || rhs.count(cl)) {
    //     lhs.clear();
    //     lhs.emplace();
    //     return;
    // }
    lhs.insert(rhs.begin(), rhs.end());
}

void add_equiv(Cnf& cnf, const conjunction& conj1, const conjunction& conj2) {
    clause cl;
    for (const auto& l : conj1.c) {
        cl.insert(negate_literal(l));
    }
    for (const auto& l : conj2.c) {
        assert(cl.count(l)==0);
        cl.insert(l);
        cnf.insert(cl);
        cl.erase(l);
    }
    cl.clear();
    for (const auto& l : conj2.c) {
        cl.insert(negate_literal(l));
    }
    for (const auto& l : conj1.c) {
        assert(cl.count(l)==0);
        cl.insert(l);
        cnf.insert(cl);
        cl.erase(l);
    }
}

Cnf duplicate(const Cnf& cnf, int shift) {
    Cnf res;
    for (const auto& cl : cnf) {
        clause res_cl;
        for (const auto& lit : cl) {
            res_cl.insert(lit + shift);
        }
        res.insert(std::move(res_cl));
    }
    return res;
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

Cnf to_cnf_or(Cnf& lhs, Cnf& rhs) {
    Cnf res;
    for (const auto& cl1 : lhs) {
        // bool t = false;
        // for (const auto& lit : cl1.first) {
        //     if (cl1.first.count(negate_literal(lit))) {
        //         t = true;
        //         break;
        //     }
        // }
        // if (t) {
        //     continue;
        // }
        for (const auto& cl2 : rhs) {
            //     if (lhs.size() > 1 && rhs.size() > 1 &&
            //         (std::includes(cl1.first.begin(), cl1.first.end(), cl2.first.begin(), cl2.first.end())
            //         || std::includes(cl2.first.begin(), cl2.first.end(), cl1.first.begin(), cl1.first.end()))) {
            //         break;
            //     }
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
                // std::cout << *it << " contains literal and its negation" << std::endl;
                okay = false;
                break;
            }
        }
        if (!okay) {
            break;
        }
        for (auto it2 = cnf.begin(); it2 != cnf.end(); it2++) {
            if (it != it2 && std::includes(it2->begin(), it2->end(), it->begin(), it->end())) {
                // std::cout << *it2 << " includes " << *it << std::endl;
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
