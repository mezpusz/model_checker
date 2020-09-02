#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"

class bmc {
public:
    bmc(const circuit& c);
    bmc& operator=(bmc& b) = default;

    bool run(uint64_t k, const Cnf& interpolant);
    Proof* get_proof();
    const std::set<uint64_t>& get_vars_b();
    const std::set<clause>& get_clauses_a();

private:
    void create_a(uint64_t k, const Cnf& interpolant);
    void create_initial(const Cnf& interpolant);
    void create_ands(uint64_t k, bool for_a);
    void create_ands(uint64_t k);
    void create_bad(uint64_t k);
    void create_transition(uint64_t k, bool for_a);
    void create_transition(uint64_t k);

    void add_equiv(const std::vector<uint64_t>& lhs, uint64_t rhs, bool for_a);
    void add_clause(const clause& cnf, bool for_a);

    circuit _c;
    Solver* _s;
    Proof* _p;
    const Cnf* _a;
    std::set<clause> _clauses_a;
    std::set<uint64_t> _vars_b;
};
