#include "interpolation.h"

#include "bmc.h"
#include "helper.h"

void InterpolantCreator::root(const vec<Lit>& c) {
    Cnf f;
    clause cl;
    for (int i = 0; i < c.size(); i++) {
        cl.insert(index(c[i]));
    }
    if (clauses_a.count(cl)) {
        clause cl;
        for (int i = 0; i < c.size(); i++) {
            if (!var_b.count(var(c[i]))) { // var not in B
                continue;
            }
            cl.insert(index(c[i])-shift); // we already shift all variables back here
        }
        f.insert(std::move(cl));
    }
    clauses.push_back(f);
}

void InterpolantCreator::chain(const vec<ClauseId>& cs, const vec<Var>& xs) {
    Cnf f = clauses[cs[0]];
    for (int i = 0; i < xs.size(); i++) {
        if (!var_b.count(xs[i])) { // f | g
            f = to_cnf_or(f, clauses[cs[i+1]]);
        } else { // f & g
            f.insert(clauses[cs[i+1]].begin(), clauses[cs[i+1]].end());
        }
    }
    clauses.push_back(f);
}

Cnf create_interpolant(int shift, const std::set<uint64_t>& v_b, const std::set<clause>& clauses_a, Proof* p) {
    InterpolantCreator ic(shift, v_b, clauses_a);
    p->traverse(ic);
    delete p;

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
            auto temp = create_interpolant(shift, b._vars_b, b._clauses_a, b._p);
            clean(temp);
            // std::cout << "interpolant: " << temp << std::endl;

            if (i > 0 && temp == interpolant) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            interpolant = std::move(temp);
            i++;
            if (b.run(k, interpolant)) {
                std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
        }
    }
    return false;
}
