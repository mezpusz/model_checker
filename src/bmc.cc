#include "bmc.h"

#include <cstdlib>

#include "aiger_parser.h"
#include "formula.h"

#include "minisat/Solver.h"
#include "minisat/Sort.h"

#if LOGGING
void resolve(vec<Lit>& main, vec<Lit>& other, Var x)
{
    Lit     p;
    bool    ok1 = false, ok2 = false;
    for (int i = 0; i < main.size(); i++) {
        if (var(main[i]) == x){
            ok1 = true, p = main[i];
            main[i] = main.last();
            main.pop();
            break;
        }
    }

    for (int i = 0; i < other.size(); i++) {
        if (var(other[i]) != x) {
            main.push(other[i]);
        } else {
            if (p != ~other[i]) {
                std::cout << "PROOF ERROR! Resolved on variable with SAME polarity in both clauses: x" << (x+1);
            }
            ok2 = true;
        }
    }

    if (!ok1 || !ok2) {
        std::cout << "PROOF ERROR! Resolved on missing variable: x" << (x+1) << std::endl
                  << main << std::endl
                  << other << std::endl;
    }

    sortUnique(main);
}
#endif

bmc::bmc(const circuit& c, bool interpolate)
    : _clauses(),
#if LOGGING
      _orig_clauses(),
#endif
      _vars_b(),
      _c(c),
      _s(),
      _phase_b(false),
      _interpolate(interpolate)
{
}

Cnf bmc::get_interpolant() {
    return _clauses.back();
}

void bmc::root(const vec<Lit>& c) {
    if (!_interpolate) {
        return;
    }
#if LOGGING
    std::cout << _orig_clauses.size() << ": ROOT " << c << std::endl;
    _orig_clauses.push();
    c.copyTo(_orig_clauses.last());
#endif
    Cnf f;
    if (!_phase_b) {
        clause cl;
        for (int i = 0; i < c.size(); i++) {
            if (!_vars_b.count(var(c[i]))) { // var not in B
                continue;
            }
            cl.push_back(index(c[i])-_c.shift()); // we already shift all variables back here
        }
        f.push_back(std::move(cl));
    }
    _clauses.push_back(f);
}

void bmc::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    if (!_interpolate) {
        return;
    }
#if LOGGING
    std::cout << _orig_clauses.size() << ": CHAIN " << cs[0];
    for (int i = 0; i < xs.size(); i++) {
        std::cout << " [" << "x" << xs[i] << "] " << cs[i+1];
    }
    _orig_clauses.push();
    vec<Lit>& c = _orig_clauses.last();
    _orig_clauses[cs[0]].copyTo(c);
    for (int i = 0; i < xs.size(); i++) {
        resolve(c, _orig_clauses[cs[i+1]], xs[i]);
    }
    std::cout << " =>" << c << std::endl;
#endif
    Cnf f = _clauses[cs[0]];
    for (int i = 0; i < xs.size(); i++) {
        if (!_vars_b.count(xs[i])) { // f | g
            f = to_cnf_or(f, _clauses[cs[i+1]]);
        } else { // f & g
            f.insert(f.end(), _clauses[cs[i+1]].begin(), _clauses[cs[i+1]].end());
        }
    }
    _clauses.push_back(f);
}

void bmc::deleted(ClauseId c) {
    if (!_interpolate) {
        return;
    }
    _clauses[c].clear();
#if LOGGING
    std::cout << "deleted " << c << std::endl;
    _orig_clauses[c].clear();
#endif
}

void bmc::create_a(uint64_t k, const Cnf& interpolant) {
    create_initial(interpolant);
    create_ands(0);
    if (k > 0) {
        create_ands(1);
        create_transition(1);
    }
}

void bmc::create_initial(const Cnf& interpolant) {
    for (const auto& [i, o] : _c.latches) {
        for (auto cl : interpolant) {
            cl.push_back(negate_literal(o));
            add_clause(std::move(cl));
        }
    }
}

void bmc::create_ands(uint64_t k) {
    auto shift = k*_c.shift();
    for (const auto& [i1, i2, o] : _c.ands) {
        add_equiv({ i1+shift, i2+shift }, o+shift);
    }
}

void bmc::create_transition(uint64_t k) {
    for (const auto& [i,o] : _c.latches) {
        add_equiv({ i+(k-1)*_c.shift() }, o+k*_c.shift());
    }
}

void bmc::add_equiv(const std::vector<uint64_t>& lhs, uint64_t rhs) {
    clause cl;
    for (const auto& l : lhs) {
        cl.push_back(negate_literal(l));
    }
    cl.push_back(rhs);
    add_clause(cl);
    cl.clear();
    cl.push_back(negate_literal(rhs));
    for (const auto& l : lhs) {
        cl.push_back(l);
        add_clause(cl);
        cl.pop_back();
    }
}

void bmc::add_clause(const clause& cl) {
    vec<Lit> lits;
    int parsed_lit, var;
    for (const auto& lit : cl) {
        if (lit == 0) { // don't add tautologies
            return;
        }
        if (lit == 1) { // don't add false literals
            continue;
        }
        parsed_lit = (lit%2==0) ? lit/2 : -((lit-1)/2);
        var = abs(parsed_lit);
        if (_phase_b) {
            _vars_b.insert(var);
        }
        while (var >= _s->nVars()) {
            _s->newVar();
        }
        lits.push((parsed_lit > 0) ? Lit(var) : ~Lit(var));
    }
    _s->addClause(lits);
}

bool bmc::run(uint64_t k, const Cnf& interpolant) {
    Solver s;
    Proof p(*this);
    s.proof = &p;
    _s = &s;

#if LOGGING
    _orig_clauses.clear();
#endif
    _clauses.clear();
    _vars_b.clear();
    _phase_b = true;
    // ands and transition
    for (uint64_t i = 2; i <= k; i++) {
        create_ands(i);
        create_transition(i);
    }
    // bad
    clause cl;
    for (const auto& o : _c.outputs) {
        cl.push_back(o+k*_c.shift());
    }
    add_clause(cl);

    _phase_b = false;
    create_a(k, interpolant);

    s.solve();

#if LOGGING
    if (s.okay()) {
        for (int i = 0; i < s.nVars(); i++) {
            if (s.model[i] != l_Undef) {
                std::cout << ((i==0) ? "" : " ")
                          << ((s.model[i]==l_True) ? "" : "~")
                          << "x"
                          << i+1;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
#endif

    _s = nullptr;
    return s.okay();
}
