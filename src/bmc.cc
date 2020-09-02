#include "bmc.h"

#include <cstdlib>

#include "aiger_parser.h"
#include "dimacs.h"
#include "formula.h"
#include "helper.h"
#include "log.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

bmc::bmc(circuit&& c)
    : _c(c), _a(), _b()
{
    reset();
}

void bmc::reset() {
    _b.clear();
}

void bmc::set_a(const Cnf* a) {
    _a = a;
}

const Cnf& bmc::get_b() {
    return _b;
}

Cnf bmc::create_a(uint64_t k, Cnf* interpolant) {
    auto res = create_initial();
    if (interpolant != nullptr) {
        res = to_cnf_or(res, *interpolant);
    }
    auto ands = create_ands();
    merge(res, ands);
    if (k > 0) {
        merge(res, duplicate(ands, _c.shift()));
        merge(res, create_transition());
    }
    return res;
}

Cnf bmc::create_initial() {
    Cnf res;
    for (const auto& [i, o] : _c.latches) {
        clause cl;
        assert(o%2==0); // maybe smth needs to be changed when negative latch outputs are possible
        cl.insert(negate_literal(o));
        res.insert(cl);
    }
    return res;
}

Cnf bmc::create_ands() {
    Cnf res;
    for (const auto& [i1, i2, o] : _c.ands) {
        conjunction conj1(i1, i2);
        conjunction conj2(o);
        add_equiv(res, conj1, conj2);
    }
    return res;
}

void bmc::create_ands(uint64_t k) {
    auto temp = create_ands();
    const auto shift = _c.shift();
    for (uint64_t i = 2; i <= k; i++) {
        merge(_b, duplicate(temp, shift*i));
    }
}

void bmc::create_bad(uint64_t k) {
    const auto shift = _c.shift();
    clause cl;
    for (const auto& o : _c.outputs) {
        cl.insert(o+k*shift);
    }
    _b.insert(cl);
}

Cnf bmc::create_transition() {
    Cnf res;
    // transitions from 0 to 1
    for (const auto& [i,o] : _c.latches) {
        conjunction conj1(i);
        conjunction conj2(o+_c.shift());
        add_equiv(res, conj1, conj2);
    }
    return res;
}

void bmc::create_transition(uint64_t k) {
    if (k < 2) {
        return;
    }
    auto temp = create_transition();
    const auto shift = _c.shift();

    // transitions from 1 to 2, ..., k-1 to k
    for (uint64_t i = 1; i < k; i++) {
        merge(_b, duplicate(temp, shift*i));
    }
}

bool bmc::run(uint64_t k) {
    reset();
    create_ands(k);
    create_bad(k);
    create_transition(k);

    Solver S;
    _p = new Proof();
    S.proof = _p;
    Cnf c;
    merge(c, *_a);
    merge(c, _b);
#if LOGGING
    circuit_debug(_c);
#endif
    // std::cout << "b: " << _b << std::endl;
    // std::cout << "bmc: " << c << std::endl;
    to_dimacs(c, S);
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
    // printStats(S.stats);
    // checkProof(S.proof);

    return S.okay();
}

Proof* bmc::get_proof() {
    return _p;
}
