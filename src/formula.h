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

inline void remove_tautologies(Cnf& cnf) {
    // we assume here that no empty clause is present
    // and all literals inside clauses are sorted
    for (auto& cl : cnf) {
        // remove duplicates
        cl.erase(unique(cl.begin(), cl.end()), cl.end());
        // eliminate tautologies (true literals are handled in add_clause)
        for (size_t j = 0; j < cl.size()-1; j++) {
            if (cl[j] % 2 == 0 && cl[j] == negate_literal(cl[j+1])) {
                cl.clear();
                break;
            }
        }
    }
    if (!cnf.empty()) {
        // remove marked clauses
        cnf.erase(std::remove_if(cnf.begin(), cnf.end(), [](const clause& cl) { return cl.empty(); }), cnf.end());
        // finally sort clauses
        sort(cnf.begin(), cnf.end());
        cnf.shrink_to_fit();
    }
}

inline void remove_subsumed(Cnf& cnf) {
    for (size_t i = 0; i < cnf.size(); i++) {
        if (!cnf[i].empty()) {
            for (size_t j = 0; j < cnf.size(); j++) {
                if (i != j && !cnf[j].empty() && includes(cnf[i].begin(), cnf[i].end(), cnf[j].begin(), cnf[j].end())) {
                    cnf[i].clear();
                    break;
                }
            }
        }
    }
    if (!cnf.empty()) {
        // remove marked clauses
        cnf.erase(std::remove_if(cnf.begin(), cnf.end(), [](const clause& cl) { return cl.empty(); }), cnf.end());
        // no sort is needed since clauses were sorted already
        cnf.shrink_to_fit();
    }
}

inline void to_cnf_or(Cnf& lhs, const Cnf& rhs) {
    // one is empty clause, return other
    if (lhs.size() == 1 && lhs[0].empty()) {
        lhs = rhs;
    }
    if (rhs.size() == 1 && rhs[0].empty()) {
        return;
    }
    // otherwise merge sorted together to form full cross-product of two CNFs
    // (note that empty CNF is also handled here) 
    Cnf res;
    for (const auto& cl1 : lhs) {
        for (const auto& cl2 : rhs) {
            res.emplace_back();
            merge(cl1.begin(), cl1.end(), cl2.begin(), cl2.end(), back_inserter(res.back()));
        }
    }
    // since new clauses are added, remove any tautologies and subsumed from these
    remove_tautologies(res);
    remove_subsumed(res);
    lhs = res;
}

inline void to_cnf_and(Cnf& lhs, const Cnf& rhs) {
    // if one is empty clause, return it
    if (lhs.size() == 1 && lhs[0].empty()) {
        return;
    }
    if (rhs.size() == 1 && rhs[0].empty()) {
        lhs = rhs;
    }
    // if rhs is empty, just return
    if (rhs.empty()) {
        return;
    }
    // otherwise merge clauses sorted together (note that empty CNF is also handled here)
    Cnf res;
    merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), back_inserter(res));
    // no new clauses, so we only need to check subsumed
    remove_subsumed(res);
    lhs = res;
}
