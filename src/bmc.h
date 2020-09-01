#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"

class bmc {
public:
    bmc(circuit&& c);

    void reset();
    bool run(uint64_t k);
    Proof* get_proof();
    void set_a(const Cnf* a);
    const Cnf& get_b();
    Cnf create_a(uint64_t k);

private:
    Cnf create_initial();
    Cnf create_ands();
    Cnf create_transition();
    void create_ands(uint64_t k);
    void create_bad(uint64_t k);
    void create_transition(uint64_t k);

    circuit _c;
    Proof* _p = nullptr;
    const Cnf* _a;
    Cnf _b;
};
