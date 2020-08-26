
#include "dimacs.h"

#include "log.h"

#include <sstream>

int literal_to_dimacs(uint64_t lit) {
    return (lit%2==0) ? lit/2 : -((lit-1)/2);
}

void to_dimacs(formula* cnf, Solver& S) {
#if LOGGING
    std::stringstream str;
#endif
    auto cnf_j = to_conjunction(cnf);
    for (const auto& cl : *cnf_j) {
        vec<Lit> lits;
        int parsed_lit, var;
#if LOGGING
        str << "(";
#endif
        auto cl_j = to_disjunction(cl);
        for (const auto& lit : *cl_j) {
            auto lit_l = to_literal(lit);
            parsed_lit = literal_to_dimacs(lit_l->var());
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
