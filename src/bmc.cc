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
    _a = new junction_formula;
    _b = new junction_formula;
    to_junction_formula(_a)->conn = connective::AND;
    to_junction_formula(_b)->conn = connective::AND;
}

void bmc::set_a(formula* a) {
    _a = a;
}

formula* bmc::get_b() {
    return _b;
}

formula* bmc::create_initial() {
    auto res = new junction_formula;
    res->conn = connective::AND;
    for (const auto& [i, o] : _c.latches) {
        auto cl = new junction_formula;
        cl->conn = connective::OR;
        assert(o%2==0); // maybe smth needs to be changed when negative latch outputs are possible
        literal l;
        l.var = o;
        cl->subformulas.push_back(negate_literal(&l));
        add_clause(res, cl);
    }
    auto a = create_ands();
    merge(res, a);
    return res;
}

formula* bmc::create_ands() {
    auto res = new junction_formula;
    res->conn = connective::AND;
    for (const auto& [i1, i2, o] : _c.ands) {
        literal li1, li2, lo;
        li1.var = i1;
        li2.var = i2;
        lo.var = o;
        conjunction conj1(&li1, &li2);
        conjunction conj2(&lo);
        add_equiv(res, conj1, conj2);
    }
    return res;
}

void bmc::create_ands(uint64_t k) {
    // if (k == 0) {
    //     return;
    // }
    auto temp = create_ands();
    const auto shift = _c.shift();
    for (uint64_t i = 2; i <= k; i++) {
        merge(_b, duplicate(temp, shift*i));
    }
}

void bmc::create_bad(uint64_t k) {
    auto cl = new junction_formula;
    cl->conn = connective::OR;
    const auto shift = _c.shift();
    for (const auto& o : _c.outputs) {
        for (uint64_t i = 0; i <= k; i++) {
            auto lit = new literal;
            lit->var = o + i*shift;
            cl->subformulas.push_back(lit);
        }
    }
    add_clause(_b, cl);
}

formula* bmc::create_transition() {
    auto temp = new junction_formula;
    temp->conn = connective::AND;
    // transitions from 0 to 1
    const auto shift = _c.shift();
    for (const auto& [i,o] : _c.latches) {
        literal li, lo;
        li.var = i;
        lo.var = o+shift;
        conjunction conj1(&li);
        conjunction conj2(&lo);
        add_equiv(temp, conj1, conj2);
    }
    return temp;
}

void bmc::create_transition(uint64_t k) {
    if (k < 2) {
        return;
    }
    auto temp = create_transition();
    const auto shift = _c.shift();

    // merge(_b, temp);
    // transitions from 1 to 2, ..., k-1 to k
    for (uint64_t i = 1; i < k; i++) {
        merge(_b, duplicate(temp, shift*i));
    }
}

bool bmc::run(uint64_t k) {
    create_ands(k);
    create_bad(k);
    create_transition(k);
#if LOGGING
    circuit_debug(_c);
    cnf_debug(_cnf);
#endif

    Solver S;
    _p = new Proof();
    S.proof = _p;
    auto c = new junction_formula;
    c->conn = connective::AND;
    merge(c, _a);
    merge(c, _b);
    // std::cout << "formula: " << formula_to_string(c) << std::endl;
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
