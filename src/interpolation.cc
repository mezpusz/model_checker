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
            auto l = store->create(var(c[i])*2 + sign(c[i]));
            if (f == nullptr) {
                f = l;
            } else {
                f = store->create_disjunction(f, l);
            }
        }
        if (f == nullptr) {
            f = store->create(0);
        }
    } else {
        // std::cout << " is not in A, ";
        f = store->create(0);
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
        junction_formula* temp;
        if (var_b.count(xs[i]) == 0) { // f | g
            temp = store->create_disjunction(f, clauses[cs[i+1]]);
        } else { // f & g
            temp = store->create_conjunction(f, clauses[cs[i+1]]);
        }
        if (temp->size() == 1) {
            f = *temp->begin();
        } else {
            f = temp;
        }
    }
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

formula* create_interpolant(formula* a, formula* b, Proof* p, formula_store* store) {
    auto v_a = get_vars(a);
    auto v_b = get_vars(b);
    InterpolantCreator ic(v_a, v_b, a, store);
    p->traverse(ic);

    return ic.clauses.back();
}

bool interpolation(circuit&& c, formula_store* store) {
    auto shift = c.shift();
    bmc b(std::move(c), store);

    uint64_t k = 1;
    while (true) {
        std::cout << "k=" << k << std::endl;
        b.reset();
        auto a = b.create_initial();
        merge(a, duplicate(b.create_ands(), shift, store), store);
        merge(a, b.create_transition(), store);
        b.set_a(a);
        if (b.run(k)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        unsigned i = 0;
        while (true) {
            auto interpolant = create_interpolant(a, b.get_b(), b.get_proof(), store);
            auto b_j = to_junction_formula(b.get_b());
            // TODO: should be true in 3rd arg
            store->decrease_junction_refcount(b_j->conn(), b_j->sub(), true);

            // auto c = to_cnf(interpolant, store);


            // since they contain the same variables, the formulas should be
            // only equal when they are exactly the same

            auto temp = a;
            a = to_cnf(a, interpolant, store);
            if (a == temp) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            // formula_set sf;
            // sf.insert(a);
            // sf.insert(interpolant);
            // auto temp = junction_formula::create(connective::OR, std::move(sf));
            store->log_static();
            
            // auto temp = a;
            // a = to_cnf(a, interpolant, store);


            auto t_j = to_junction_formula(temp);
            store->decrease_junction_refcount(t_j->conn(), t_j->sub(), true);
            store->log_static();
            b.reset();
            b.set_a(a);
            i++;
            if (b.run(k)) {
                std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
        }
    }
}
