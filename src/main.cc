
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "formula.h"

#include "minisat/Solver.h"

int literal_to_dimacs(int lit) {
    return (lit%2==1) ? lit/2 : -lit/2;
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

cnf create_initial(const circuit& c) {
    cnf res;
    res.num_vars = c.M;
    for (const auto& [i, o] : c.latches) {
        clause cl;
        cl.lits.insert(negate_literal(o));
        res.add_clause(cl);
    }
    return res;
}

cnf create_bad(const circuit& c, int k) {
    cnf res;
    res.num_vars = c.M*(k+1);
    clause cl;
    for (const auto& o : c.outputs) {
        for (int i = 0; i <= k; i++) {
            cl.lits.insert(o + i*c.M*2);
        }
    }
    res.add_clause(cl);
    return res;
}

int main(int argc, char** argv) {
    std::string input_file;
    int k = -1;
    if (argc < 2) {
        std::cerr << "Usage: ./model_checker input_file [k]" << std::endl;
        return -1;
    } else if (argc == 2) {
        input_file = argv[1];
    } else {
        input_file = argv[1];
        k = atoi(argv[2]);
    }

    circuit c;
    if (!parse_aiger_file(input_file, c)) {
        return -1;
    }

    circuit_debug(c);

    auto cnf = create_initial(c);
    cnf.merge(create_bad(c, 0));
    cnf_debug(cnf);

    Solver S;
    S.proof = new Proof();
    to_dimacs(cnf, S);
    S.solve();
    std::cout << (S.okay() ? "sat" : "unsat") << std::endl;

    return 0;
}