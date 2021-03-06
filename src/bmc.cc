#include "bmc.h"

#include "minisat/Sort.h"

using namespace std;

template<class T>
void copyTo(const vec<T>& v, std::vector<T>& other) {
    other.resize(v.size());
    for (int i = 0; i < v.size(); i++) {
        other[i] = v[i];
    }
}

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
                cout << "PROOF ERROR! Resolved on variable with SAME polarity in both clauses: x" << (x+1);
            }
            ok2 = true;
        }
    }

    if (!ok1 || !ok2) {
        cout << "PROOF ERROR! Resolved on missing variable: x" << (x+1) << endl
                  << main << endl
                  << other << endl;
    }

    sortUnique(main);
}
#endif

bmc::bmc(const circuit& c)
    : _clauses(),
      _chains(),
      _num_clauses(0),
#if LOGGING
      _orig_clauses(),
#endif
      _vars_b(),
      _c(c),
      _s(),
      _phase_b(false)
{
}

Cnf bmc::get_interpolant() {
    return calc_chain(_num_clauses-1);
}

void bmc::root(const vec<Lit>& c) {
#if LOGGING
    cout << _orig_clauses.size() << ": ROOT " << c << endl;
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
        f.push_back(move(cl));
    }
    if (!f.empty()) {
        _clauses.insert(make_pair(_num_clauses, f));
    }
    _num_clauses++;
}

void bmc::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
#if LOGGING
    cout << _orig_clauses.size() << ": CHAIN " << cs[0];
    for (int i = 0; i < xs.size(); i++) {
        cout << " [" << "x" << xs[i] << "] " << cs[i+1];
    }
    _orig_clauses.push();
    vec<Lit>& c = _orig_clauses.last();
    _orig_clauses[cs[0]].copyTo(c);
    for (int i = 0; i < xs.size(); i++) {
        resolve(c, _orig_clauses[cs[i+1]], xs[i]);
    }
    cout << " =>" << c << endl;
#endif
    if (_phase_b) {
        _num_clauses++;
        return;
    }
    auto res = _chains.emplace(make_pair(_num_clauses++, make_pair(vector<ClauseId>(), vector<Var>())));
    copyTo(xs, res.first->second.second);
    copyTo(cs, res.first->second.first);
}

Cnf bmc::calc_chain(ClauseId id) {
    auto rIt = _clauses.find(id);
    if (rIt != _clauses.end()) {
        return rIt->second;
    }
    auto cIt = _chains.find(id);
    Cnf f;
    if (cIt == _chains.end()) {
        return f;
    }
    const auto& cs = cIt->second.first;
    const auto& xs = cIt->second.second;
    f = calc_chain(cs[0]);
    for (size_t i = 0; i < xs.size(); i++) {
        auto g = calc_chain(cs[i+1]);
        if (!_vars_b.count(xs[i])) { // f | g
            to_cnf_or(f, g);
        } else { // f & g
            to_cnf_and(f, g);
        }
    }
    if (!f.empty()) {
        _clauses.insert(make_pair(id, f));
    }
    _chains.erase(id);
    return f;
}

void bmc::deleted(ClauseId c) {
    _clauses.erase(c);
    _chains.erase(c);
#if LOGGING
    cout << "DELETED " << c << endl;
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
    for (const auto& kv : _c.latches) {
        for (auto cl : interpolant) {
            cl.push_back(negate_literal(kv.second));
            add_clause(move(cl));
        }
    }
}

// we shift everything except true and false
inline uint64_t sh(uint64_t v, uint64_t shift) {
    if (v == 0 || v == 1) {
        return v;
    }
    return v+shift;
}

void bmc::create_ands(uint64_t k) {
    auto shift = k*_c.shift();
    for (const auto& [i1, i2, o] : _c.ands) {
        add_equiv({ sh(i1,shift), sh(i2,shift) }, sh(o,shift));
    }
}

void bmc::create_transition(uint64_t k) {
    for (const auto& [i,o] : _c.latches) {
        add_equiv({ sh(i,(k-1)*_c.shift()) }, sh(o,k*_c.shift()));
    }
}

void bmc::add_equiv(const vector<lit>& lhs, lit rhs) {
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
    for (const auto& lit : cl) {
        if (lit == 1) { // don't add tautologies
            return;
        }
        if (lit == 0) { // don't add false literals
            continue;
        }
        int var = lit >> 1;
        if (_phase_b) {
            _vars_b.insert(var);
        }
        while (var >= _s->nVars()) {
            _s->newVar();
        }
        lits.push((lit & 1) ? ~Lit(var) : Lit(var));
    }
    _s->addClause(lits);
}

/**
 * The run is optimized to calculate the new interpolant.
 * As this object is a ProofTraverser, we can first add
 * all things that belong to B (from the interpolation A => B)
 * in the beginning. That is marked by _phase_b. During this,
 * we only create Ts for roots (see root) and record what
 * variables are in B. The reason B is first is that during
 * the phase of A, we already need the variables in B.
 */
bool bmc::run(uint64_t k, const Cnf& interpolant) {
    Solver s;
    Proof p(*this);
    s.proof = &p;
    _s = &s;

#if LOGGING
    _orig_clauses.clear();
#endif
    _clauses.clear();
    _chains.clear();
    _num_clauses = 0;
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
        cl.push_back(sh(o,k*_c.shift()));
    }
    add_clause(cl);

    _phase_b = false;
    create_a(k, interpolant);

    s.solve();

#if LOGGING
    if (s.okay()) {
        for (int i = 0; i < s.nVars(); i++) {
            if (s.model[i] != l_Undef) {
                cout << ((i==0) ? "" : " ")
                          << ((s.model[i]==l_True) ? "" : "~")
                          << "x"
                          << i+1;
            }
        }
        cout << endl;
    }
    cout << endl;
#endif

    _s = nullptr;
    return s.okay();
}
