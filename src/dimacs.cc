
#include "dimacs.h"

int literal_to_dimacs(int lit) {
    return (lit%2==0) ? lit/2 : -lit/2;
}

void to_dimacs(const cnf& c, Solver& S) {
    for (const auto& cl : c.cls) {
        vec<Lit> lits;
        int parsed_lit, var;
        for (const auto& lit : cl.lits) {
            parsed_lit = literal_to_dimacs(lit);
            var = abs(parsed_lit)-1;
            while (var >= S.nVars()) S.newVar();
            lits.push( (parsed_lit > 0) ? Lit(var) : ~Lit(var) );
        }
        S.addClause(lits);
    }
}
