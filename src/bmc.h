
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "dimacs.h"
#include "formula.h"

cnf create_equiv(const conjunction& conj1, const conjunction& conj2);
cnf create_initial(const circuit& c);
cnf create_bad(const circuit& c, int k);
cnf duplicate(const cnf& c, int shift);
cnf create_transition(const circuit& c, int k);
bool run_bmc(const circuit& c, int k);