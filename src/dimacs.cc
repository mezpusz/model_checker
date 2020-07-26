
#include "dimacs.h"

#include <sstream>
#include <iostream>

int literal_to_dimacs(uint64_t lit) {
    return (lit%2==0) ? lit/2 : -(lit/2);
}

void to_dimacs(const cnf& c, Solver& S) {
#if LOGGING
    std::stringstream str;
#endif
    for (const auto& cl : c.cls) {
        vec<Lit> lits;
        int parsed_lit, var;
#if LOGGING
        str << "(";
#endif
        for (const auto& lit : cl.lits) {
            parsed_lit = literal_to_dimacs(lit);
            var = abs(parsed_lit)-1;
#if LOGGING
            str << ((parsed_lit>0)?"-":"") << var << " ";
#endif
            while (var >= S.nVars()) S.newVar();
            lits.push( (parsed_lit > 0) ? Lit(var) : ~Lit(var) );
        }
#if LOGGING
        str << ") ";
#endif
        S.addClause(lits);
    }
#if LOGGING
    std::cout << str.str() << std::endl;
#endif
}
