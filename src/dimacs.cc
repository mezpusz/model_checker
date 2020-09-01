
#include "dimacs.h"

#include "log.h"

#include <sstream>

int literal_to_dimacs(uint64_t lit) {
    return (lit%2==0) ? lit/2 : -((lit-1)/2);
}

void to_dimacs(const Cnf& cnf, Solver& S) {
#if LOGGING
    std::stringstream str;
#endif
    for (const auto& cl : cnf) {
        vec<Lit> lits;
        int parsed_lit, var;
#if LOGGING
        str << "(";
#endif
        for (const auto& lit : cl) {
            parsed_lit = literal_to_dimacs(lit);
            var = abs(parsed_lit);
#if LOGGING
            str << ((parsed_lit>0)?"":"-") << var << " ";
#endif
            while (var >= S.nVars())
            {
                S.newVar();
            }
            lits.push((parsed_lit > 0) ? Lit(var) : ~Lit(var));
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
