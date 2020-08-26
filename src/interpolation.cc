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
        auto sub = cl_j->sub();
        if (cl_o.empty() && sub.empty()) {
            return true;
        }
        bool found = !sub.empty();
        for (const auto& l : *cl_j) {
            auto l_l = to_literal(l);
            if (cl_o.count(l_l->var()) == 0) {
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
            auto l = literal::create(var(c[i])*2 + sign(c[i]));
            if (f == nullptr) {
                f = l;
            } else {
                // junction_formula* disj;
                // if (!f->is_literal()) {
                //     disj = to_disjunction(f);
                // } else {
                //     disj = junction_formula::create(connective::OR);
                //     disj->sf.insert(f);
                // }
                // disj->sf.insert(l);
                // f = disj;
                f = junction_formula::create_disjunction(f, l);
            }
        }
        if (f == nullptr) {
            f = literal::create(0);
        }
    } else {
        // std::cout << " is not in A, ";
        f = literal::create(0);
    }
    // std::cout << "root: " << f->to_string() << std::endl;
    clauses.push_back(f);
}

void InterpolantCreator::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    assert(xs.size() > 0);
    assert(cs.size() > 1);
    assert(cs[0] >= 0 && cs[0] < clauses.size());

    // std::cout << "CHAIN[" << clauses.size() << "]: " << clauses[cs[0]]->to_string()
    //           << "[" << cs[0] << "] [x";
    formula* f = clauses[cs[0]];
    for (uint64_t i = 0; i < xs.size(); i++) {
        assert(cs[i+1] >= 0 && cs[i+1] < clauses.size());
        // std::cout  << xs[i] << "] " << clauses[cs[i+1]]->to_string() << "[" << cs[i+1] << "] ";
        if (is_true(clauses[cs[i+1]]) && var_b.count(xs[i]) == 0) { // f | T = T
            f = clauses[cs[i]];
        } else if (var_b.count(xs[i]) == 0) { // f | g
            f = junction_formula::create_disjunction(f, clauses[cs[i+1]]);
            // if (!f->is_literal() && to_junction_formula(f)->conn == connective::OR) {
            //     to_junction_formula(f)->sf.insert(clauses[cs[i+1]]);
            // } else if (!is_true(f)) {
            //     auto nf = junction_formula::create(connective::OR);
            //     nf->sf.insert(f);
            //     if (!clauses[cs[i+1]]->is_literal() && to_junction_formula(clauses[cs[i+1]])->conn == connective::OR) {
            //         for (auto& sf : *to_junction_formula(clauses[cs[i+1]])) {
            //             nf->sf.insert(sf);
            //         }
            //     } else {
            //         nf->sf.insert(clauses[cs[i+1]]);
            //     }
            //     f = nf;
            // }
        } else if (!is_true(clauses[cs[i+1]])) { // f & g
            f = junction_formula::create_conjunction(f, clauses[cs[i+1]]);
            // if (!f->is_literal() && to_junction_formula(f)->conn == connective::AND) {
            //     to_junction_formula(f)->sf.insert(clauses[cs[i+1]]);
            // } else if (is_true(f)) {
            //     f = clauses[cs[i+1]];
            // } else {
            //     auto nf = junction_formula::create(connective::AND);
            //     nf->sf.insert(f);
            //     if (!clauses[cs[i+1]]->is_literal() && to_junction_formula(clauses[cs[i+1]])->conn == connective::AND) {
            //         for (auto& sf : *to_junction_formula(clauses[cs[i+1]])) {
            //             nf->sf.insert(sf);
            //         }
            //     } else {
            //         nf->sf.insert(clauses[cs[i+1]]);
            //     }
            //     f = nf;
            // }
        }
    }
    // if (!f->is_literal()) {
        // std::cout << "chain: " << f->to_string() << std::endl;
    // }
    clauses.push_back(f);
}

std::set<uint64_t> get_vars(formula* cnf) {
    auto cnf_j = to_conjunction(cnf);
    std::set<uint64_t> res;
    for (const auto& cl : *cnf_j) {
        auto cl_j = to_disjunction(cl);
        for (const auto& l : *cl_j) {
            auto v = to_literal(l)->var();
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
            // cleanse(interpolant);
            auto c = to_cnf(interpolant);
            std::cout << "interpolant cnf: " << c->to_string() << std::endl;
            // cnf_debug(c);
            if (equal_cnf(a, c)) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            std::cout << a->to_string() << std::endl;
            formula_set sf;
            sf.insert(a);
            sf.insert(interpolant);
            auto temp = junction_formula::create(connective::OR, std::move(sf));
            std::cout << "new initial: " << temp->to_string() << std::endl;
            a = to_cnf(temp);
            b.set_a(a);
            if (b.run(k)) {
                k++;
                break;
            }
        }
    }
}
