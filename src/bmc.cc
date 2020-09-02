#include "bmc.h"

#include <cstdlib>

#include "aiger_parser.h"
#include "formula.h"
#include "helper.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

bmc::bmc(const circuit& c)
    : _c(c),
      _a(),
      _clauses_a(),
      _vars_b()
{
}

const std::set<uint64_t>& bmc::get_vars_b() {
    return _vars_b;
}

const std::set<clause>& bmc::get_clauses_a() {
    return _clauses_a;
}

void bmc::create_a(uint64_t k, const Cnf& interpolant) {
    create_initial(interpolant);
    create_ands(0, true);
    if (k > 0) {
        create_ands(1, true);
        create_transition(0, true);
    }
}

void bmc::create_initial(const Cnf& interpolant) {
    for (const auto& [i, o] : _c.latches) {
        for (auto cl : interpolant) {
            assert(o%2==0); // maybe smth needs to be changed when negative latch outputs are possible
            cl.insert(negate_literal(o));
            add_clause(cl, true);
        }
    }
}

void bmc::create_ands(uint64_t k, bool for_a) {
    auto shift = k*_c.shift();
    for (const auto& [i1, i2, o] : _c.ands) {
        add_equiv({ i1+shift, i2+shift }, o+shift, for_a);
    }
}

void bmc::create_ands(uint64_t k) {
    for (uint64_t i = 2; i <= k; i++) {
        create_ands(i, false);
    }
}

void bmc::create_bad(uint64_t k) {
    clause cl;
    for (const auto& o : _c.outputs) {
        cl.insert(o+k*_c.shift());
    }
    add_clause(cl, false);
}

void bmc::create_transition(uint64_t k, bool for_a) {
    for (const auto& [i,o] : _c.latches) {
        add_equiv({ i+k*_c.shift() }, o+(k+1)*_c.shift(), for_a);
    }
}

void bmc::create_transition(uint64_t k) {
    if (k < 2) {
        return;
    }
    for (uint64_t i = 1; i < k; i++) {
        create_transition(i, false);
    }
}

void bmc::add_equiv(const std::vector<uint64_t>& lhs, uint64_t rhs, bool for_a) {
    clause cl;
    for (const auto& l : lhs) {
        cl.insert(negate_literal(l));
    }
    cl.insert(rhs);
    add_clause(cl, for_a);
    cl.clear();
    cl.insert(negate_literal(rhs));
    for (const auto& l : lhs) {
        assert(cl.count(l)==0);
        cl.insert(l);
        add_clause(cl, for_a);
        cl.erase(l);
    }
}

void bmc::add_clause(const clause& cl, bool for_a) {
    if (for_a) {
        _clauses_a.insert(cl);
    }
    vec<Lit> lits;
    int parsed_lit, var;
    for (const auto& lit : cl) {
        parsed_lit = (lit%2==0) ? lit/2 : -((lit-1)/2);
        var = abs(parsed_lit);
        if (!for_a) {
            _vars_b.insert(var);
        }
        while (var >= _s->nVars())
        {
            _s->newVar();
        }
        lits.push((parsed_lit > 0) ? Lit(var) : ~Lit(var));
    }
    _s->addClause(lits);
}

bool bmc::run(uint64_t k, const Cnf& interpolant) {
    Solver s;
    s.proof = new Proof();
    _p = s.proof;
    _s = &s;

    create_a(k, interpolant);
    create_ands(k);
    create_bad(k);
    create_transition(k);

    s.solve();

#if LOGGING
    circuit_debug(_c);
#endif
#if LOGGING
    if (_s.okay()) {
        for (uint64_t i = 0; i < _s.nVars(); i++) {
            if (_s.model[i] != l_Undef) {
                std::cout << ((i==0) ? "" : " ")
                          << ((_s.model[i]==l_True) ? "" : "~")
                          << "x"
                          << i+1;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
#endif
    // printStats(_s.stats);
    // checkProof(_s.proof);

    _s = nullptr;
    return s.okay();
}

Proof* bmc::get_proof() {
    return _p;
}
