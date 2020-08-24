#pragma once

#include "circuit.h"
#include "formula.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

struct InterpolantCreator : public ProofTraverser {
    InterpolantCreator(std::set<uint64_t> v_a, std::set<uint64_t> v_b, formula* a)
        : var_a(v_a), var_b(v_b), a(a)
    {}

    void root(const vec<Lit>& c) override;
    void chain(const vec<ClauseId>& cs, const vec<Var>& xs) override;

    std::set<uint64_t> var_a;
    std::set<uint64_t> var_b;
    formula* a;

    std::vector<formula*> clauses;
};

formula* create_interpolant(formula* a, formula* b, Proof* p);
bool interpolation(circuit&& c);
