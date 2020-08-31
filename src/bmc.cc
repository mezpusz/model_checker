#include "bmc.h"

#include <cstdlib>

#include "aiger_parser.h"
#include "dimacs.h"
#include "formula.h"
#include "helper.h"
#include "log.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

bmc::bmc(circuit&& c, formula_store* store)
    : _c(c), _a(), _b(), _store(store)
{
    reset();
}

void bmc::reset() {
    _a = _store->create(connective::AND);
    _b = _store->create(connective::AND);
}

void bmc::set_a(formula* a) {
    _a = a;
}

formula* bmc::get_b() {
    return _b;
}

formula* bmc::create_initial() {
    formula* res = _store->create(connective::AND);
    for (const auto& [i, o] : _c.latches) {
        assert(o%2==0); // maybe smth needs to be changed when negative latch outputs are possible
        auto l = _store->create(o);
        formula_set sf;
        sf.insert(negate_literal(l, _store));
        add_clause(res, _store->create(connective::OR, std::move(sf)), _store);
    }
    auto a = create_ands();
    add_clause(res, a, _store);
    return res;
}

formula* bmc::create_ands() {
    formula* res = _store->create(connective::AND);
    for (const auto& [i1, i2, o] : _c.ands) {
        conjunction conj1(_store->create(i1), _store->create(i2));
        conjunction conj2(_store->create(o));
        add_equiv(res, conj1, conj2, _store);
    }
    return res;
}

void bmc::create_ands(uint64_t k) {
    auto temp = create_ands();
    const auto shift = _c.shift();
    for (uint64_t i = 2; i <= k; i++) {
        merge(_b, duplicate(temp, shift*i, _store), _store);
    }
}

void bmc::create_bad(uint64_t k) {
    const auto shift = _c.shift();
    formula_set sf;
    for (const auto& o : _c.outputs) {
        for (uint64_t i = 0; i <= k; i++) {
            sf.insert(_store->create(o + i*shift));
        }
    }
    add_clause(_b, _store->create(connective::OR, std::move(sf)), _store);
}

formula* bmc::create_transition() {
    formula* temp = _store->create(connective::AND);
    // transitions from 0 to 1
    const auto shift = _c.shift();
    for (const auto& [i,o] : _c.latches) {
        conjunction conj1(_store->create(i));
        conjunction conj2(_store->create(o+shift));
        add_equiv(temp, conj1, conj2, _store);
    }
    return temp;
}

void bmc::create_transition(uint64_t k) {
    if (k < 2) {
        return;
    }
    auto temp = create_transition();
    const auto shift = _c.shift();

    // transitions from 1 to 2, ..., k-1 to k
    for (uint64_t i = 1; i < k; i++) {
        merge(_b, duplicate(temp, shift*i, _store), _store);
    }
}

bool bmc::run(uint64_t k) {
    create_ands(k);
    create_bad(k);
    create_transition(k);

    Solver S;
    _p = new Proof();
    S.proof = _p;
    formula* c = _store->create(connective::AND);
    _a->manual_refcount = true;
    _b->manual_refcount = true;
    merge(c, _a, _store);
    merge(c, _b, _store);
#if LOGGING
    circuit_debug(_c);
    cnf_debug(c);
#endif
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
