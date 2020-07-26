#pragma once

#include "formula.h"

#include "minisat/Solver.h"

int literal_to_dimacs(int lit);
void to_dimacs(const cnf& c, Solver& S);
