#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"

class bmc {
public:
    bmc(circuit&& c, formula_store* store);

    void reset();
    bool run(uint64_t k);
    Proof* get_proof();
    void set_a(formula* a);
    formula* get_b();

    formula* create_initial();
    formula* create_ands();
    formula* create_transition();

private:
    void create_ands(uint64_t k);
    void create_bad(uint64_t k);
    void create_transition(uint64_t k);

    circuit _c;
    Proof* _p = nullptr;
    formula* _a;
    formula* _b;
    formula_store* _store;
};
