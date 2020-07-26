#include "bmc.h"

#include <cstdlib>

#include "aiger_parser.h"
#include "dimacs.h"
#include "formula.h"
#include "log.h"

#include "minisat/Solver.h"

bmc::bmc(circuit&& c)
    : _c(c), _cnf()
{}

void bmc::create_initial() {
    for (const auto& [i, o] : _c.latches) {
        clause cl;
        cl.lits.insert(negate_literal(o));
        _cnf.add_clause(cl);
    }
    for (const auto& [i1, i2, o] : _c.ands) {
        conjunction conj1(o);
        conjunction conj2(i1, i2);
        _cnf.add_equiv(conj1, conj2);
    }
}

void bmc::create_bad(uint64_t k) {
    clause cl;
    const auto shift = _c.shift();
    for (const auto& o : _c.outputs) {
        for (uint64_t i = 0; i <= k; i++) {
            cl.lits.insert(o + i*shift);
        }
    }
    _cnf.add_clause(cl);
}

void bmc::create_transition(uint64_t k) {
    if (k == 0) {
        return;
    }
    cnf temp;
    // transitions from 0 to 1
    const auto shift = _c.shift();
    for (const auto& [i,o] : _c.latches) {
        conjunction conj1(o+shift);
        conjunction conj2(i);
        temp.add_equiv(conj1, conj2);
    }
    for (const auto& [i1, i2, o] : _c.ands) {
        conjunction conj1(o+shift);
        conjunction conj2(i1+shift, i2+shift);
        temp.add_equiv(conj1, conj2);
    }

    _cnf.merge(temp);
    // transitions from 1 to 2, ..., k-1 to k
    for (uint64_t i = 1; i < k; i++) {
        _cnf.merge(temp.duplicate(shift*i));
    }
}

bool bmc::run(uint64_t k) {
    create_initial();
    create_bad(k);
    create_transition(k);
#if LOGGING
    // circuit_debug(_c);
    // cnf_debug(_cnf);
#endif

    Solver S;
    S.proof = new Proof();
    to_dimacs(_cnf, S);
    S.solve();
#if LOGGING
    if (S.okay()) {
        for (uint64_t i = 0; i < S.nVars(); i++) {
            if (S.model[i] != l_Undef) {
                std::cout << ((i==0) ? "" : " ")
                          << ((S.model[i]==l_True) ? "" : "~")
                          << "x"
                          << i+1;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
#endif

    return S.okay();
}
