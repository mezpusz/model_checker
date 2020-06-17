
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "formula.h"

#include "minisat/Solver.h"

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

cnf create_initial(const circuit& c) {
    cnf res;
    for (const auto& [i, o] : c.latches) {
        clause cl;
        cl.lits.insert(negate_literal(o));
        res.add_clause(cl);
    }
    return res;
}

cnf create_bad(const circuit& c, int k) {
    cnf res;
    clause cl;
    auto shift = c.M*2;
    for (const auto& o : c.outputs) {
        for (int i = 0; i <= k; i++) {
            cl.lits.insert(o + i*shift);
        }
    }
    res.add_clause(cl);
    return res;
}

cnf create_equiv(std::set<int> conj1, std::set<int> conj2) {
    cnf res;
    clause cl;
    for (const auto& l : conj1) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj2) {
        cl.lits.insert(l);
        res.cls.insert(cl);
        cl.lits.erase(l);
    }
    cl.lits.clear();
    for (const auto& l : conj2) {
        cl.lits.insert(negate_literal(l));
    }
    for (const auto& l : conj1) {
        cl.lits.insert(l);
        res.cls.insert(cl);
        cl.lits.erase(l);
    }
    return res;
}

cnf duplicate(const cnf& c, int shift) {
    cnf res;
    for (const auto& cl : c.cls) {
        clause cl_n;
        for (const auto& lit : cl.lits) {
            cl_n.lits.insert(lit + shift);
        }
        res.add_clause(cl_n);
    }
    return res;
}

cnf create_transition(const circuit& c, int k) {
    cnf res;
    if (k == 0) {
        return res;
    }
    int shift = c.M*2;
    for (const auto& [i,o] : c.latches) {
        std::set<int> conj1;
        conj1.insert(o+shift);
        std::set<int> conj2;
        conj2.insert(i);
        res.merge(create_equiv(conj1, conj2));
    }
    for (const auto& [kv, o] : c.ands) {
        std::set<int> conj1;
        conj1.insert(o);
        std::set<int> conj2;
        conj2.insert(kv.first);
        conj2.insert(kv.second);
        res.merge(create_equiv(conj1, conj2));
    }

    auto temp = res;
    for (int i = 1; i < k; i++) {
        res.merge(duplicate(temp, shift*k));
    }

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

    if (k == -1) {
        std::cerr << "Bound was not given as argument" << std::endl;
        return -1;
    }

    circuit c;
    if (!parse_aiger_file(input_file, c)) {
        return -1;
    }

    circuit_debug(c);

    auto cnf = create_initial(c);
    std::cout << "Initial function:" << std::endl;
    cnf_debug(cnf);
    auto bad = create_bad(c, k);
    std::cout << "Bad function:" << std::endl;
    cnf_debug(bad);
    cnf.merge(bad);
    auto tr = create_transition(c, k);
    std::cout << "Transition function:" << std::endl;
    cnf_debug(tr);
    cnf.merge(tr);
    // cnf_debug(cnf);

    Solver S;
    S.proof = new Proof();
    to_dimacs(cnf, S);
    S.solve();
    std::cout << (S.okay() ? "sat" : "unsat") << std::endl;
    if (S.okay()) {
        for (int i = 0; i < S.nVars(); i++) {
            if (S.model[i] != l_Undef) {
                std::cout << ((i==0) ? "" : " ")
                          << ((S.model[i]==l_True) ? "" : "~")
                          << "x"
                          << i+1;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}