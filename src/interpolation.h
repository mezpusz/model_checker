#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

struct InterpolantCreator : public ProofTraverser {
    InterpolantCreator(std::set<uint64_t> v_a, std::set<uint64_t> v_b, Cnf a)
        : var_a(v_a), var_b(v_b), a(a)
    {}

    void root(const vec<Lit>& c) override;
    void chain(const vec<ClauseId>& cs, const vec<Var>& xs) override;

    std::set<uint64_t> var_a;
    std::set<uint64_t> var_b;
    Cnf a;

    std::vector<Cnf> clauses;
};

Cnf create_interpolant(const Cnf& a, const Cnf& b, Proof* p);
bool interpolation(circuit&& c);
