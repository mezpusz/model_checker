#include "interpolation.h"

#include "bmc.h"
#include "helper.h"

bool check_contains(formula* cnf, const vec<Lit>& cl) {
    auto cnf_j = to_conjunction(cnf);
    std::set<uint64_t> cl_o;
    for (int i = 0; i < cl.size(); i++) {
        cl_o.insert(var(cl[i])*2 + sign(cl[i]));
    }

    for (const auto& cl : *cnf_j) {
        auto cl_j = to_disjunction(cl);
        if (cl_o.empty() && cl_j->subformulas.empty()) {
            return true;
        }
        bool found = !cl_j->subformulas.empty();
        for (const auto& l : *cl_j) {
            auto l_l = to_literal(l);
            if (cl_o.count(l_l->var) == 0) {
                found = false;
                break;
            }
        }
        if (found) {
            return true;
        }
    }
    return false;
}

std::string to_string(const vec<Lit>& c) {
    std::string res;
    for (int i = 0; i < c.size(); i++) {
        res+=(sign(c[i])?"~":"");
        res+="x"+std::to_string(var(c[i]));
        if (i < c.size()-1) {
            res+=" | ";
        }
    }
    return res;
}

void InterpolantCreator::root(const vec<Lit>& c) {
    formula* f = nullptr;
    // std::cout << "ROOT[" << clauses.size() << "]: " << to_string(c);
    if (check_contains(a, c)) {
        // std::cout << " is in A, ";
        for (int i = 0; i < c.size(); i++) {
            if (var_b.count(var(c[i])) == 0) { // var not in A
                continue;
            }
            auto l = new literal;
            l->var = var(c[i])*2 + sign(c[i]);
            if (f == nullptr) {
                f = l;
            } else {
                junction_formula* disj;
                if (!f->is_literal()) {
                    disj = to_disjunction(f);
                } else {
                    disj = new junction_formula;
                    disj->conn = connective::OR;
                    disj->subformulas.push_back(f);
                }
                disj->subformulas.push_back(l);
                f = disj;
            }
        }
        if (f == nullptr) {
            auto l = new literal;
            l->var = 0;
            f = l;    
        }
    } else {
        // std::cout << " is not in A, ";
        auto l = new literal;
        l->var = 0;
        f = l;
    }
    // std::cout << "formula: " << f->to_string() << std::endl;
    clauses.push_back(f);
}

void InterpolantCreator::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    assert(xs.size() > 0);
    assert(cs.size() > 1);
    assert(cs[0] >= 0 && cs[0] < clauses.size());

    // std::cout << "CHAIN[" << clauses.size() << "]: " << clauses[cs[0]]->to_string()
    //           << "[" << cs[0] << "] [x";
    formula* f = nullptr;
    for (uint64_t i = 0; i < xs.size(); i++) {
        assert(cs[i+1] >= 0 && cs[i+1] < clauses.size());
        // std::cout  << xs[i] << "] " << clauses[cs[i+1]]->to_string() << "[" << cs[i+1] << "] ";
        if (is_true(clauses[cs[i]]) && is_true(clauses[cs[i+1]])) {
            if (f == nullptr || var_b.count(xs[i]) == 0) {
                f = clauses[cs[i]];
            }
        } else if (is_true(clauses[cs[i]])) {
            if (var_b.count(xs[i])==0) {
                f = clauses[cs[i]];
            } else {
                if (f == nullptr) {
                    f = clauses[cs[i+1]];
                } else {
                    auto nf = new junction_formula;
                    nf->subformulas.push_back(f);
                    nf->subformulas.push_back(clauses[cs[i+1]]);
                    nf->conn = connective::AND;
                    f = nf;
                }
            }
        } else if (clauses[cs[i+1]] == nullptr) {
            if (var_b.count(xs[i])==0) {
                f = clauses[cs[i]];
            } else {
                if (f == nullptr) {
                    f = clauses[cs[i]];
                } else {
                    auto nf = new junction_formula;
                    nf->subformulas.push_back(f);
                    nf->subformulas.push_back(clauses[cs[i]]);
                    nf->conn = connective::AND;
                    f = nf;
                }
            }
        } else {
            auto nf = new junction_formula;
            nf->subformulas.push_back(clauses[cs[i]]);
            nf->subformulas.push_back(clauses[cs[i+1]]);
            nf->conn = (var_b.count(xs[i]/2)==0) ? connective::OR : connective::AND;
            f = nf;
        }
        // std::cout << " formula: " << clauses.back()->to_string() << std::endl;
    }
    clauses.push_back(f);
}

std::set<uint64_t> get_vars(formula* cnf) {
    auto cnf_j = to_conjunction(cnf);
    std::set<uint64_t> res;
    for (const auto& cl : *cnf_j) {
        auto cl_j = to_disjunction(cl);
        for (const auto& l : *cl_j) {
            auto v = to_literal(l)->var;
            res.insert(v/2);
        }
    }
    return res;
}

formula* create_interpolant(formula* a, formula* b, Proof* p) {
    auto v_a = get_vars(a);
    auto v_b = get_vars(b);
    // std::cout << "A: " << a->to_string() << std::endl;
    // cnf_debug(a);
    // std::cout << "B: " << b->to_string() << std::endl;
    // cnf_debug(b);
    InterpolantCreator ic(v_a, v_b, a);
    p->traverse(ic);

    return ic.clauses.back();
}

bool interpolation(circuit&& c) {
    auto shift = c.shift();
    bmc b(std::move(c));

    uint64_t k = 1;
    while (true) {
        std::cout << "k=" << k << std::endl;
        b.reset();
        auto a = b.create_initial();
        merge(a, duplicate(b.create_ands(), shift));
        merge(a, b.create_transition());
        b.set_a(a);
        if (b.run(k)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        while (true) {
            // checkProof(b.get_proof());
            auto interpolant = create_interpolant(a, b.get_b(), b.get_proof());
            cleanse(interpolant);
            std::cout << "interpolant: " << interpolant->to_string() << std::endl;
            auto c = to_cnf(interpolant);
            // cnf_debug(c);
            if (equal(a, c)) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            auto temp = new junction_formula;
            temp->conn = connective::OR;
            temp->subformulas.push_back(a);
            temp->subformulas.push_back(c);
            // std::cout << "new initial: " << formula_to_string(temp) << std::endl;
            a = to_cnf(temp);
            b.set_a(a);
            if (b.run(k)) {
                k++;
                break;
            }
        }
    }
}
