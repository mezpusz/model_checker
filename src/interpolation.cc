#include "interpolation.h"

#include "bmc.h"
#include "helper.h"

#define LOGGING 0

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
#if LOGGING
    std::cout << "ROOT[" << clauses.size() << "]: " << to_string(c);
#endif
    if (check_contains(a, c)) {
        clause cl;
#if LOGGING
        std::cout << " is in A, ";
#endif
        for (int i = 0; i < c.size(); i++) {
            if (!var_b.count(var(c[i]))) { // var not in B
                continue;
            }
            cl.insert(var(c[i])*2 + sign(c[i]));
        }
        f.insert(std::move(cl));
    } else {
#if LOGGING
        std::cout << " not in A, ";
#endif
    }
#if LOGGING
    std::cout << f << std::endl;
#endif
    clauses.push_back(f);
}

void InterpolantCreator::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    assert(xs.size() > 0);
    assert(cs.size() > 1);
    assert(cs[0] >= 0 && cs[0] < clauses.size());

#if LOGGING
    std::cout << "CHAIN[" << clauses.size() << "]: " << clauses[cs[0]]
              << "[" << cs[0] << "] [x";
#endif
    Cnf f = clauses[cs[0]];
    for (uint64_t i = 0; i < xs.size(); i++) {
        assert(cs[i+1] >= 0 && cs[i+1] < clauses.size());
#if LOGGING
        std::cout  << xs[i] << "] ";
#endif
        if (!var_b.count(xs[i])) { // f | g
#if LOGGING
            std::cout << "not in B, ";
#endif
            f = to_cnf_or(f, clauses[cs[i+1]]);
        } else { // f & g
#if LOGGING
            std::cout << "in B, ";
#endif
            merge(f, clauses[cs[i+1]]);
        }
#if LOGGING
        std::cout << clauses[cs[i+1]] << "[" << cs[i+1] << "] [x";
#endif
    }
#if LOGGING
    std::cout << '\t' << f << std::endl;
#endif
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
#if LOGGING
    std::cout << "B: " << b << std::endl;
#endif
    auto v_a = get_vars(a);
    auto v_b = get_vars(b);
    InterpolantCreator ic(v_a, v_b, a);
    p->traverse(ic);

    return ic.clauses.back();
}

bool interpolation(circuit&& c) {
    auto shift = c.shift();
    std::cout << shift << std::endl;
    bmc b(std::move(c));

    uint64_t k = 1;
    while (k<shift) {
        std::cout << "k=" << k << std::endl;
        auto a = b.create_a(k);
        b.set_a(&a);
        if (b.run(k)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        unsigned i = 0;
        Cnf interpolant;
        while (true) {
            auto temp = create_interpolant(a, b.get_b(), b.get_proof());
            clean(temp);
            std::cout << "interpolant: " << temp << std::endl;

            if (temp == interpolant) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            interpolant = std::move(temp);
            // std::cout << interpolant << std::endl << std::endl;
            auto temp2 = duplicate(interpolant, -shift);
#if LOGGING
            std::cout << "interpolant: " << temp2 << std::endl;
#endif
            a = b.create_a(k, &temp2);
#if LOGGING
            std::cout << "new initial: " << a << std::endl;
#endif
            // clean(a);
            b.set_a(&a);
            i++;
            if (b.run(k)) {
                std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
        }
    }
}
