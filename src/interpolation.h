#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

struct InterpolantCreator : public ProofTraverser {
    InterpolantCreator(const std::set<uint64_t>& v_b, const std::set<clause>& clauses_a)
        : var_b(v_b), clauses_a(clauses_a)
    {}

    void root(const vec<Lit>& c) override;
    void chain(const vec<ClauseId>& cs, const vec<Var>& xs) override;

    std::set<uint64_t> var_b;
    const std::set<clause>& clauses_a;

    std::vector<Cnf> clauses;
};

Cnf create_interpolant(const std::set<uint64_t>& v_b, const std::set<clause>& clauses_a, Proof* p);
bool interpolation(circuit c);
