#include "interpolation.h"

#include "bmc.h"
#include "helper.h"

#define LOGGING 0

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
    clause cl;
    for (int i = 0; i < c.size(); i++) {
        cl.insert(var(c[i])*2 + sign(c[i]));
    }
    if (clauses_a.count(cl)) {
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
            f.insert(clauses[cs[i+1]].begin(), clauses[cs[i+1]].end());
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

Cnf create_interpolant(const std::set<uint64_t>& v_b, const std::set<clause>& clauses_a, Proof* p) {
#if LOGGING
    std::cout << "B: " << b << std::endl;
#endif
    InterpolantCreator ic(v_b, clauses_a);
    p->traverse(ic);

    return ic.clauses.back();
}

bool interpolation(circuit c) {
    auto shift = c.shift();
    std::cout << shift << std::endl;
    bmc b(c);

    uint64_t k = 1;
    while (k<shift) {
        std::cout << "k=" << k << std::endl;
        Cnf temp;
        temp.emplace();
        if (b.run(k, temp)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        unsigned i = 0;
        Cnf interpolant;
        while (true) {
            auto temp = create_interpolant(b.get_vars_b(), b.get_clauses_a(), b.get_proof());
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
            i++;
            if (b.run(k, temp2)) {
                std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
        }
    }
}
