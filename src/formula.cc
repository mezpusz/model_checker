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

formula* negate_literal(formula* lit) {
    auto v = to_literal(lit)->var();
    return literal::create((v%2 == 0) ? (v + 1) : (v - 1));
}

void add_clause(formula*& cnf, formula* cl) {
    cnf = junction_formula::create_conjunction(cnf, cl);
}

void merge(formula*& cnf, formula* other) {
    cnf = junction_formula::create_conjunction(cnf, other);
}

void add_equiv(formula*& cnf, const conjunction& conj1, const conjunction& conj2) {
    formula_set sf;
    for (const auto& l : conj1.c) {
        assert(l->is_literal());
        sf.insert(negate_literal(l));
    }
    for (const auto& l : conj2.c) {
        sf.insert(l);
        cnf = junction_formula::create_conjunction(cnf, junction_formula::create(connective::OR, sf));
        sf.erase(l);
    }
    sf.clear();
    for (const auto& l : conj2.c) {
        assert(l->is_literal());
        sf.insert(negate_literal(l));
    }
    for (const auto& l : conj1.c) {
        sf.insert(l);
        add_clause(cnf, junction_formula::create(connective::OR, sf));
        sf.erase(l);
    }
}

formula* duplicate(formula* cnf, uint64_t shift) {
    auto cnf_j = to_conjunction(cnf);
    formula* res = junction_formula::create(connective::AND);
    for (const auto& cl : *cnf_j) {
        formula_set sf;
        auto cl_j = to_disjunction(cl);
        for (const auto& lit : *cl_j) {
            auto lit_j = to_literal(lit);
            auto lit_n = literal::create(lit_j->var() + shift);
            sf.insert(lit_n);
        }
        add_clause(res, junction_formula::create(connective::OR, std::move(sf)));
    }
    return res;
}

void cnf_debug(formula* cnf) {
    std::cout << cnf->to_string() << std::endl;
}

formula* to_cnf(formula* f) {
    if (f == nullptr) {
        return f;
    }
    formula_set sf_c;
    sf_c.insert(f);
    formula_set sf_s;
    sf_s.insert(junction_formula::create(connective::OR, sf_c));
    formula* s = junction_formula::create(connective::AND, sf_s);

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
                    cl_sf.erase(curr);
                    sf_s.erase(cl);
                    auto curr_sf = curr_j->sub();
                    auto it = curr_sf.begin();
                    it++;
                    for (; it != curr_sf.end(); it++) {
                        cl_sf.insert(*it);
                        if (conn == connective::AND) {
                            sf_s.insert(junction_formula::create(connective::OR, cl_sf));
                            cl_sf.erase(*it);
                        }
                    }
                    cl_sf.insert(*curr_sf.begin());
                    cl = junction_formula::create(connective::OR, cl_sf);
                    sf_s.insert(cl);
                    s = junction_formula::create(connective::AND, sf_s);
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

bool equal_cnf(formula* cnf1, formula* cnf2) {
    auto cnf1_j = to_conjunction(cnf1);
    auto cnf2_j = to_conjunction(cnf2);
    bool match = true;
    for (const auto& cl1 : *cnf1_j) {
        std::set<uint64_t> lits;
        auto cl1_j = to_disjunction(cl1);
        for (const auto& lit1 : *cl1_j) {
            lits.insert(to_literal(lit1)->var());
        }
        for (const auto& cl2 : *cnf2_j) {
            auto cl2_j = to_disjunction(cl2);
            bool found = true;
            for (const auto& lit2 : *cl2_j) {
                if (lits.count(to_literal(lit2)->var()) == 0) {
                    found = false;
                    break;
                }
            }
            if (!found) {
                match = false;
                return match;
            }
        }
    }
    return match;
}

bool is_true(formula* f) {
    if (!f->is_literal()) {
        return false;
    }
    return to_literal(f)->var() == 0;
}

literal* literal::create(uint64_t var) {
    auto res = global_literals.insert(std::make_pair(var, nullptr));
    if (res.second) {
        res.first->second = new literal;
        res.first->second->v = var;
    }
    return res.first->second;
}

junction_formula* junction_formula::create(connective c) {
    return junction_formula::create(c, formula_set());
}

junction_formula* junction_formula::create(connective c, formula_set&& sf) {
    auto m = global_ands;
    if (c == connective::OR) {
        m = global_ors;
    }
    auto res = m.insert(std::make_pair(sf, nullptr));
    if (res.second) {
        res.first->second = new junction_formula;
        res.first->second->c = c;
        res.first->second->sf = new formula_set(res.first->first);
    }
    return res.first->second;
}

junction_formula* junction_formula::create(connective c, const formula_set& sf) {
    auto& m = global_ands;
    if (c == connective::OR) {
        m = global_ors;
    }
    auto res = m.insert(std::make_pair(sf, nullptr));
    if (res.second) {
        res.first->second = new junction_formula;
        res.first->second->c = c;
        res.first->second->sf = new formula_set(res.first->first);
    }
    return res.first->second;
}

junction_formula* junction_formula::create_conjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return wrap_single_formula(connective::AND, f2);
    }
    if (is_true(f2)) {
        return wrap_single_formula(connective::AND, f1);
    }
    auto sf = merge_subformulas(connective::AND, f1, f2);
    return junction_formula::create(connective::AND, std::move(sf));
}

junction_formula* junction_formula::create_disjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return wrap_single_formula(connective::OR, f1);
    }
    if (is_true(f2)) {
        return wrap_single_formula(connective::OR, f2);
    }
    auto sf = merge_subformulas(connective::OR, f1, f2);
    return junction_formula::create(connective::OR, std::move(sf));
}

junction_formula* junction_formula::wrap_single_formula(connective c, formula* f) {
    if (f->is_literal() || to_junction_formula(f)->c != c) {
        formula_set sf;
        sf.insert(f);
        return junction_formula::create(c, std::move(sf));
    }
    return to_junction_formula(f);
}

formula_set junction_formula::merge_subformulas(connective c, formula* f1, formula* f2) {
    formula_set sf;
    if (!f1->is_literal() && to_junction_formula(f1)->c == c) {
        sf = *to_junction_formula(f1)->sf;
        if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
            sf.insert(to_junction_formula(f2)->sf->begin(), to_junction_formula(f2)->sf->end());
        } else {
            sf.insert(f2);
        }
    } else if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
        sf = *to_junction_formula(f2)->sf;
        sf.insert(f2);
    } else {
        sf.insert(f1);
        sf.insert(f2);
    }
    return sf;
}
