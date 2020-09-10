#pragma once

#include "circuit.h"
#include "formula.h"

#include <iostream>
#include <set>
#include <map>

#include "minisat/Solver.h"

#define LOGGING 0

class bmc : public ProofTraverser {
public:
    bmc(const circuit& c);

    bool run(uint64_t k, const Cnf& interpolant);
    Cnf get_interpolant();

    void root(const vec<Lit>& c) override;
    void chain(const vec<ClauseId>& cs, const vec<Var>& xs) override;
    void deleted(ClauseId c) override;

private:
    void create_a(uint64_t k, const Cnf& interpolant);
    void create_initial(const Cnf& interpolant);
    void create_ands(uint64_t k);
    void create_transition(uint64_t k);

    void add_equiv(const std::vector<lit>& lhs, lit rhs);
    void add_clause(const clause& cl);

    std::map<size_t, Cnf> _clauses;
    uint64_t _num_clauses;
#if LOGGING
    vec<vec<Lit>> _orig_clauses;
#endif
    std::set<uint64_t> _vars_b;
    circuit _c;
    Solver* _s;
    bool _phase_b;
};
