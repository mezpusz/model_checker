#pragma once

#include "circuit.h"
#include "formula.h"

class bmc {
public:
    bmc(circuit&& c);

    bool run(uint64_t k);

private:
    void create_initial();
    void create_bad(uint64_t k);
    void create_transition(uint64_t k);

    circuit _c;
    cnf _cnf;
};
