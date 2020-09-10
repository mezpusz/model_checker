#pragma once

#include <vector>
#include <functional>
#include <sstream>

#include "minisat/SolverTypes.h"

using namespace std;

using lit = uint64_t;
using clause = vector<lit>;
using Cnf = vector<clause>;

inline string lit_to_string(lit n) {
    if (n == 0) {
        return "T";
    }
    if (n == 1) {
        return "F";
    }
    stringstream str;
    str << ((n%2==1) ? "~" : "") << "x" << n/2;
    return str.str();
}

inline lit negate_literal(lit lit) {
    return (lit%2 == 0) ? (lit + 1) : (lit - 1);
}

inline ostream& operator<<(ostream& out, const clause& cl) {
    out << "(";
    for (auto it = cl.begin(); it != cl.end();) {
        out << lit_to_string(*it);
        if (++it != cl.end()) {
            out << " | ";
        }
    }
    out << ")";
    return out;
}

inline ostream& operator<<(ostream& out, const Cnf& cnf) {
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

inline ostream& operator<<(ostream& out, const Lit& lit) {
    out << (sign(lit) ? "~" : "") << "x" << var(lit);
    return out;
}

inline ostream& operator<<(ostream& out, const vec<Lit>& lits) {
    out << "(";
    for (int i = 0; i < lits.size(); i++) {
        out << lits[i];
        if (i+1 < lits.size()) {
            out << " & ";
        }
    }
    out << ")";
    return out;
}

inline void clean(Cnf& cnf, bool sort = true) {
    for (size_t i = 0; i < cnf.size();) {
        // remove duplicates
        if (sort) {
            std::sort(cnf[i].begin(), cnf[i].end());
        }
        cnf[i].erase(
            std::unique(cnf[i].begin(), cnf[i].end()),
            cnf[i].end());
        if (!cnf[i].empty()) {
            bool t = false;
            // eliminate tautologies (true literals are handled in add_clause)
            for (size_t j = 0; j < cnf[i].size()-1; j++){
                if (cnf[i][j] == negate_literal(cnf[i][j+1])) {
                    t = true;
                    break;
                }
            }
            if (t) {
                cnf[i] = cnf.back();
                cnf.pop_back();
                continue;
            }
        }
        i++;
    }
    // check for subsumed clauses
    for (size_t i = 0; i < cnf.size();) {
        bool s = false;
        for (size_t j = 0; j < cnf.size(); j++) {
            if (i != j && std::includes(cnf[i].begin(), cnf[i].end(), cnf[j].begin(), cnf[j].end())) {
                s = true;
                break;
            }
        }
        if (s) {
            cnf[i] = cnf.back();
            cnf.pop_back();
            continue;
        }
        i++;
    }
    std::sort(cnf.begin(), cnf.end());
    cnf.erase(std::unique(cnf.begin(), cnf.end()), cnf.end());
    cnf.shrink_to_fit();
}

inline Cnf to_cnf_or(const Cnf& lhs, const Cnf& rhs) {
    Cnf res;
    for (const auto& cl1 : lhs) {
        for (const auto& cl2 : rhs) {
            res.emplace_back();
            std::merge(cl1.begin(), cl1.end(), cl2.begin(), cl2.end(), std::back_inserter(res.back()));
        }
    }
    clean(res, false);
    return res;
}
