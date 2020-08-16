#pragma once

#include "minisat/Solver.h"
#include "minisat/Sort.h"

void resolve(vec<Lit>& main, vec<Lit>& other, Var x);

struct Checker : public ProofTraverser {
    vec<vec<Lit> >  clauses;

    void root (const vec<Lit>& c);
    void chain (const vec<ClauseId>& cs, const vec<Var>& xs);
    void deleted(ClauseId c);
};

void checkProof(Proof* proof, ClauseId goal = ClauseId_NULL);