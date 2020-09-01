#include "interpolation.h"

#include "bmc.h"
#include "helper.h"

bool check_contains(Cnf cnf, const vec<Lit>& cl) {
    clause cl_o;
    for (int i = 0; i < cl.size(); i++) {
        cl_o.insert(var(cl[i])*2 + sign(cl[i]));
    }
    // std::cout << cl_o << "\t" << cnf << std::endl;
    for (const auto& cl : cnf) {
        if (cl_o == cl) {
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
    Cnf f;
    // std::cout << "ROOT[" << clauses.size() << "]: " << to_string(c);
    if (check_contains(a, c)) {
        clause cl;
        // std::cout << " is in A, ";
        for (int i = 0; i < c.size(); i++) {
            if (!var_b.count(var(c[i]))) { // var not in B
                continue;
            }
            cl.insert(var(c[i])*2 + sign(c[i]));
        }
        f.insert(std::move(cl));
    } else {
        // std::cout << " not in A, ";
    }
    // std::cout << "formula: " << f << std::endl;
    clauses.push_back(f);
}

void InterpolantCreator::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    assert(xs.size() > 0);
    assert(cs.size() > 1);
    assert(cs[0] >= 0 && cs[0] < clauses.size());

    // std::cout << "chain" << std::endl;
    // std::cout << "CHAIN[" << clauses.size() << "]: " << xs.size() << std::endl; //<< clauses[cs[0]]->to_string()
            //   << "[" << cs[0] << "] [x";
    Cnf f = clauses[cs[0]];
    for (uint64_t i = 0; i < xs.size(); i++) {
        assert(cs[i+1] >= 0 && cs[i+1] < clauses.size());
        // std::cout  << xs[i] << "] " << clauses[cs[i+1]]->to_string() << "[" << cs[i+1] << "] ";
        if (!var_b.count(xs[i])) { // f | g
            f = to_cnf_or(f, clauses[cs[i+1]]);
        } else { // f & g
            merge(f, clauses[cs[i+1]]);
        }
    }
    // std::cout << "chain: " << f << std::endl;
    clauses.push_back(f);
}

std::set<uint64_t> get_vars(const Cnf& cnf) {
    std::set<uint64_t> res;
    for (const auto& cl : cnf) {
        for (const auto& l : cl) {
            res.insert(l/2);
        }
    }
    return res;
}

Cnf create_interpolant(const Cnf& a, const Cnf& b, Proof* p) {
    // std::cout << "B: " << b << std::endl;
    auto v_a = get_vars(a);
    auto v_b = get_vars(b);
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
        b.set_a(&a);
        if (b.run(k)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        unsigned i = 0;
        Cnf interpolant;
        while (true) {
            auto temp = create_interpolant(a, b.get_b(), b.get_proof());
            // clean(temp);
            // std::cout << "interpolant: " << temp << std::endl;

            if (temp == interpolant) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            interpolant = std::move(temp);
            auto temp2 = duplicate(interpolant, -shift);
            a = to_cnf_or(a, temp2);
            // clean(a);
            b.reset();
            b.set_a(&a);
            i++;
            if (b.run(k)) {
                // std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
        }
    }
}
