
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "dimacs.h"
#include "formula.h"

#include "minisat/Solver.h"

cnf create_equiv(const conjunction& conj1, const conjunction& conj2) {
    cnf res;
    clause cl;
    for (const auto& l : conj1.c) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj2.c) {
        assert(cl.lits.count(l) == 0);
        cl.lits.insert(l);
        res.cls.insert(cl);
        cl.lits.erase(l);
    }
    cl.lits.clear();
    for (const auto& l : conj2.c) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj1.c) {
        assert(cl.lits.count(l) == 0);
        cl.lits.insert(l);
        res.cls.insert(cl);
        cl.lits.erase(l);
    }
    return res;
}

cnf create_initial(const circuit& c) {
    cnf res;
    for (const auto& [i, o] : c.latches) {
        clause cl;
        cl.lits.insert(negate_literal(o));
        res.add_clause(cl);
    }
    for (const auto& [kv, o] : c.ands) {
        conjunction conj1(o);
        conjunction conj2(kv.first, kv.second);
        res.merge(create_equiv(conj1, conj2));
    }
    return res;
}

cnf create_bad(const circuit& c, int k) {
    cnf res;
    clause cl;
    const auto shift = c.shift();
    for (const auto& o : c.outputs) {
        for (int i = 0; i <= k; i++) {
            cl.lits.insert(o + i*shift);
        }
    }
    res.add_clause(cl);
    return res;
}

cnf duplicate(const cnf& c, int shift) {
    cnf res;
    for (const auto& cl : c.cls) {
        clause cl_n;
        for (const auto& lit : cl.lits) {
            cl_n.lits.insert(lit + shift);
        }
        res.add_clause(cl_n);
    }
    return res;
}

cnf create_transition(const circuit& c, int k) {
    cnf res;
    if (k == 0) {
        return res;
    }
    // transitions from 0 to 1
    const auto shift = c.shift();
    for (const auto& [i,o] : c.latches) {
        conjunction conj1(o+shift);
        conjunction conj2(i);
        res.merge(create_equiv(conj1, conj2));
    }
    for (const auto& [kv, o] : c.ands) {
        conjunction conj1(o);
        conjunction conj2(kv.first, kv.second);
        res.merge(create_equiv(conj1, conj2));
    }

    const auto temp = res;
    // transitions from 1 to 2, ..., k-1 to k
    for (int i = 1; i < k; i++) {
        res.merge(duplicate(temp, shift*i));
    }

    return res;
}

bool run_bmc(const circuit& c, int k) {
    // circuit_debug(c);

    auto cnf = create_initial(c);
    // std::cout << "Initial function:" << std::endl;
    // cnf_debug(cnf);
    auto bad = create_bad(c, k);
    // std::cout << "Bad function:" << std::endl;
    // cnf_debug(bad);
    cnf.merge(bad);
    auto tr = create_transition(c, k);
    // std::cout << "Transition function:" << std::endl;
    // cnf_debug(tr);
    cnf.merge(tr);
    // cnf_debug(cnf);

    Solver S;
    S.proof = new Proof();
    to_dimacs(cnf, S);
    S.solve();
    // if (S.okay()) {
    //     for (int i = 0; i < S.nVars(); i++) {
    //         if (S.model[i] != l_Undef) {
    //             std::cout << ((i==0) ? "" : " ")
    //                       << ((S.model[i]==l_True) ? "" : "~")
    //                       << "x"
    //                       << i+1;
    //         }
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;

    return S.okay();
}