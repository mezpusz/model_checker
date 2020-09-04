#pragma once

#include "circuit.h"
#include "formula.h"

#include <iostream>
#include <set>

#include "minisat/Solver.h"

#define LOGGING 0

class bmc : public ProofTraverser {
public:
    bmc(const circuit& c, bool interpolate);

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

    void add_equiv(const std::vector<uint64_t>& lhs, uint64_t rhs);
    void add_clause(const clause& cl);

    std::vector<Cnf> _clauses;
#if LOGGING
    vec<vec<Lit>> _orig_clauses;
#endif
    std::set<uint64_t> _vars_b;
    circuit _c;
    Solver* _s;
    bool _phase_b;
    bool _interpolate;
};
