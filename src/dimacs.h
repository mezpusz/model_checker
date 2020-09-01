#pragma once

#include "formula.h"

#include "minisat/Solver.h"

int literal_to_dimacs(uint64_t lit);
void to_dimacs(const Cnf& cnf, Solver& S);
