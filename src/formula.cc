#include "formula.h"

#include <cassert>
#include <iostream>
#include <sstream>

std::string literal_to_string(uint64_t n) {
    if (n == 0) {
        return "T";
    }
    if (n == 1) {
        return "F";
    }
    std::stringstream str;
    str << ((n%2==1) ? "~" : "") << "x" << n/2;
    return str.str();
}

formula* negate_literal(formula* lit, formula_store* store) {
    auto v = to_literal(lit)->var();
    return store->create((v%2 == 0) ? (v + 1) : (v - 1));
}

void add_clause(formula*& cnf, formula* cl, formula_store* store) {
    cnf = store->create_conjunction(cnf, cl);
}

void merge(formula*& cnf, formula* other, formula_store* store) {
    cnf = store->create_conjunction(cnf, other);
}

void add_equiv(formula*& cnf, const conjunction& conj1, const conjunction& conj2, formula_store* store) {
    formula_set sf;
    for (const auto& l : conj1.c) {
        assert(l->is_literal());
        sf.insert(negate_literal(l, store));
    }
    for (const auto& l : conj2.c) {
        sf.insert(l);
        cnf = store->create_conjunction(cnf, store->create(connective::OR, sf));
        sf.erase(l);
    }
    sf.clear();
    for (const auto& l : conj2.c) {
        assert(l->is_literal());
        sf.insert(negate_literal(l, store));
    }
    for (const auto& l : conj1.c) {
        sf.insert(l);
        add_clause(cnf, store->create(connective::OR, sf), store);
        sf.erase(l);
    }
}

formula* duplicate(formula* cnf, uint64_t shift, formula_store* store) {
    auto cnf_j = to_conjunction(cnf);
    formula* res = store->create(connective::AND);
    for (const auto& cl : *cnf_j) {
        formula_set sf;
        auto cl_j = to_disjunction(cl);
        for (const auto& lit : *cl_j) {
            auto lit_j = to_literal(lit);
            auto lit_n = store->create(lit_j->var() + shift);
            sf.insert(lit_n);
        }
        add_clause(res, store->create(connective::OR, std::move(sf)), store);
    }
    return res;
}

void cnf_debug(formula* cnf) {
    std::cout << cnf->to_string() << std::endl;
}

formula* to_cnf(formula* f, formula_store* store) {
    formula_set sf_c;
    formula_set sf_s;
    sf_s.insert(store->create(connective::OR, sf_c));
    formula* s = store->create(connective::AND, sf_s);
    return to_cnf(s, f, store);
}

formula* to_cnf(formula* s, formula* f, formula_store* store) {
    auto subs = to_junction_formula(s)->sub();
    for (const auto& cl : *to_junction_formula(s)) {
        subs.erase(cl);
        auto cl_sf = to_junction_formula(cl)->sub();
        cl_sf.insert(f);
        subs.insert(store->create(connective::OR, std::move(cl_sf)));
    }
    s = store->create(connective::AND, std::move(subs));

    bool changed = false;
    do {
        changed = false;
        auto s_j = static_cast<junction_formula*>(s);
        auto sf_s = s_j->sub();
        for (auto it = s_j->begin(); it != s_j->end(); it++) {
            auto sf = *it;
            auto cl = to_disjunction(sf);

            for (auto oit = cl->begin(); oit != cl->end(); oit++) {
                auto curr = *oit;
                if (!curr->is_literal()) {
                    auto curr_j = to_junction_formula(curr);
                    auto conn = curr_j->conn();
                    auto cl_sf = cl->sub();
                    store->decrease_junction_refcount(conn, curr_j->sub(), false);
                    store->decrease_junction_refcount(connective::OR, cl_sf, false);
                    store->decrease_junction_refcount(connective::AND, sf_s, false);
                    cl_sf.erase(curr);
                    sf_s.erase(cl);
                    auto curr_sf = curr_j->sub();
                    auto it = curr_sf.begin();
                    it++;
                    for (; it != curr_sf.end(); it++) {
                        cl_sf.insert(*it);
                        if (conn == connective::AND) {
                            sf_s.insert(store->create(connective::OR, cl_sf));
                            cl_sf.erase(*it);
                        }
                    }
                    cl_sf.insert(*curr_sf.begin());
                    cl = store->create(connective::OR, cl_sf);
                    sf_s.insert(cl);
                    s = store->create(connective::AND, sf_s);
                    changed = true;
                    break;
                }
            }
            if (changed) {
                break;
            }
        }
    } while (changed);

    return s;
}

// bool equal_cnf(formula* cnf1, formula* cnf2) {
//     auto cnf1_j = to_conjunction(cnf1);
//     auto cnf2_j = to_conjunction(cnf2);
//     bool match = true;
//     for (const auto& cl1 : *cnf1_j) {
//         std::set<uint64_t> lits;
//         auto cl1_j = to_disjunction(cl1);
//         for (const auto& lit1 : *cl1_j) {
//             lits.insert(to_literal(lit1)->var());
//         }
//         for (const auto& cl2 : *cnf2_j) {
//             auto cl2_j = to_disjunction(cl2);
//             bool found = true;
//             for (const auto& lit2 : *cl2_j) {
//                 if (lits.count(to_literal(lit2)->var()) == 0) {
//                     found = false;
//                     break;
//                 }
//             }
//             if (!found) {
//                 match = false;
//                 return match;
//             }
//         }
//     }
//     return match;
// }

bool is_true(formula* f) {
    if (!f->is_literal()) {
        return false;
    }
    return to_literal(f)->var() == 0;
}

literal* formula_store::create(uint64_t var) {
    auto res = literals.insert(std::make_pair(var, std::make_pair(nullptr, 0)));
    if (res.second) {
        res.first->second.first = new literal;
        res.first->second.first->v = var;
    }
    res.first->second.second++;
    return res.first->second.first;
}

void formula_store::decrease_literal_refcount(uint64_t var) {
    // return;

    auto it = literals.find(var);
    assert(it != literals.end());
    if (it->second.second == 1) {
        delete it->second.first;
        literals.erase(it);
        // std::cout << "deleted2 " << it2->second->to_string() << std::endl;
    } else {
        it->second.second--;
    }
}

junction_formula* formula_store::create(connective c) {
    return create(c, formula_set());
}

junction_formula* formula_store::create(connective c, formula_set&& sf) {
    auto& m = get(c);
    auto res = m.insert(std::make_pair(sf, std::make_pair(nullptr, 0)));
    if (res.second) {
        res.first->second.first = new junction_formula;
        res.first->second.first->c = c;
        res.first->second.first->sf = new formula_set(res.first->first);
        // std::cout << "created " << res.first->second->to_string() << std::endl;
    }
    else {
        // std::cout << "queried " << res.first->second->to_string() << std::endl;
    }
    res.first->second.second++;
    return res.first->second.first;
}

junction_formula* formula_store::create(connective c, const formula_set& sf) {
    auto& m = get(c);
    auto res = m.insert(std::make_pair(sf, std::make_pair(nullptr, 0)));
    if (res.second) {
        res.first->second.first = new junction_formula;
        res.first->second.first->c = c;
        res.first->second.first->sf = new formula_set(res.first->first);
    }
    res.first->second.second++;
    return res.first->second.first;
}

junction_formula* formula_store::create_conjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return wrap_single_formula(connective::AND, f2);
    }
    if (is_true(f2)) {
        return wrap_single_formula(connective::AND, f1);
    }
    auto sf = merge_subformulas(connective::AND, f1, f2);
    return create(connective::AND, std::move(sf));
}

junction_formula* formula_store::create_disjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return wrap_single_formula(connective::OR, f1);
    }
    if (is_true(f2)) {
        return wrap_single_formula(connective::OR, f2);
    }
    auto sf = merge_subformulas(connective::OR, f1, f2);
    return create(connective::OR, std::move(sf));
}

junction_formula* formula_store::wrap_single_formula(connective c, formula* f) {
    if (f->is_literal() || to_junction_formula(f)->c != c) {
        formula_set sf;
        sf.insert(f);
        return create(c, std::move(sf));
    }
    return to_junction_formula(f);
}

void formula_store::decrease_junction_refcount(connective c, const formula_set& sf, bool subformulas) {
    // return;

    auto& m = get(c);
    auto it = m.find(sf);
    if (it == m.end()) {
        return;
    }
    if (it->second.second == 1) {
        // std::cout << "deleted " << it2->second->to_string() << std::endl;
        delete it->second.first->sf;
        delete it->second.first;
        m.erase(it);
        if (subformulas) {
            for (const auto& f : sf) {
                // std::cout << "deleted sf: " << f->to_string() << std::endl;
                if (f->is_literal()) {
                    decrease_literal_refcount(to_literal(f)->var());
                } else {
                    auto f_j = to_junction_formula(f);
                    decrease_junction_refcount(f_j->conn(), f_j->sub(), true);
                }
            }
        }
    } else {
        it->second.second--;
    }
}

formula_set formula_store::merge_subformulas(connective c, formula* f1, formula* f2) {
    formula_set sf;
    if (!f1->is_literal() && to_junction_formula(f1)->c == c) {
        sf = *to_junction_formula(f1)->sf;
        if (!f1->manual_refcount) {
            decrease_junction_refcount(c, sf, false);
        }
        if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
            auto sf2 = to_junction_formula(f2)->sf;
            if (!f2->manual_refcount) {
                decrease_junction_refcount(c, *sf2, false);
            }
            sf.insert(sf2->begin(), sf2->end());
        } else {
            sf.insert(f2);
        }
    } else if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
        sf = *to_junction_formula(f2)->sf;
        if (!f2->manual_refcount) {
            decrease_junction_refcount(c, sf, false);
        }
        sf.insert(f2);
    } else {
        sf.insert(f1);
        sf.insert(f2);
    }
    return sf;
}

void formula_store::log_static() {
    std::cout << literals.size() << '\t'
              << ands.size() << '\t'
              << ors.size() << std::endl;
}
