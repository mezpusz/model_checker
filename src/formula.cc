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

// void cleanse(formula* f) {
//     bool changed = false;
//     do {
//         changed = false;
//         if (f->is_literal()) {
//             return;
//         }
//         auto f_j = to_junction_formula(f);
//         for (auto& sf : *f_j) {
//             cleanse(sf);
//             if (is_true(sf)) {
//                 if (f_j->conn == connective::OR) {
//                     auto l = new literal;
//                     l->var = 0;
//                     f = l;
//                     return;
//                 } else if (f_j->conn == connective::AND) {
//                     // f_j->subformulas.erase(sf);
//                 } else {
//                     assert(false);
//                 }
//             }
//         }
//     } while (changed);
// }

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
    auto cnf_j = to_conjunction(cnf);
    std::stringstream str;
    for (auto it = cnf_j->begin(); it != cnf_j->end();) {
        str << "(";
        auto cl_j = to_disjunction(*it);
        for (auto it2 = cl_j->begin(); it2 != cl_j->end();) {
            str << (*it2)->to_string();
            // str << literal_to_string(to_literal(*it2)->var);
            it2++;
            if (it2 != cl_j->end()) {
                str << " | ";
            }
        }
        str << ")";
        it++;
        if (it != cnf_j->end()) {
            str << " & ";
        }
    }
    // std::cout << str.str() << std::endl;
    std::cout << cnf->to_string() << std::endl;
}

formula* to_cnf(formula* f) {
    std::cout << "to_cnf start" << std::endl;
    if (f == nullptr) {
        return f;
    }
    formula_set sf_c;
    sf_c.insert(f);
    formula_set sf_s;
    sf_s.insert(junction_formula::create(connective::OR, sf_c));
    formula* s = junction_formula::create(connective::AND, sf_s);
    // std::cout << s->to_string() << std::endl;

    bool changed = false;
    do {
        // std::cout << "new iter1" << std::endl;
        changed = false;
        auto s_j = static_cast<junction_formula*>(s);
        for (auto it = s_j->begin(); it != s_j->end(); it++) {
            auto sf = *it;
            auto cl = to_disjunction(sf);

            // std::cout << "new iter2" << std::endl;
            for (auto oit = cl->begin(); oit != cl->end(); oit++) {
                // std::cout << curr->to_string() << std::endl;
                auto curr = *oit;
                if (!curr->is_literal()) {
                    auto bf = to_junction_formula(curr);
                    auto conn = bf->conn();
                    auto cl_sf = cl->sub();
                    cl_sf.erase(curr);
                    auto bf_sf = bf->sub();
                    auto it = bf_sf.begin();
                    it++;
                    if (conn == connective::AND) {
                        for (; it != bf_sf.end(); it++) {
                            cl_sf.insert(*it);
                            add_clause(s, junction_formula::create(connective::OR, cl_sf));
                            cl_sf.erase(*it);
                        }
                    } else {
                        for (; it != bf_sf.end(); it++) {
                            auto temp = junction_formula::create_disjunction(cl, *it);
                            assert(temp == cl);
                        }
                    }
                    // std::cout << curr->to_string() << ", " << cl->to_string() << std::endl;
                    auto temp = junction_formula::create_disjunction(cl, *bf_sf.begin());
                    assert(temp == cl);
                    // std::cout << curr->to_string() << ", " << cl->to_string() << std::endl;
                    // delete curr;
                    changed = true;
                    break;
                } else {
                    // if (is_true(curr)) {
                    //     s->subformulas.erase(it);
                    //     changed = true;
                    //     break;
                    // }
                    // std::cout << curr->to_string() << " done" << std::endl;
                    continue;
                }
            }
            // std::cout << "to_cnf new formula: " << s->to_string() << std::endl;
            if (changed) {
                // std::cout << "to_cnf new formula: " << std::endl;
                break;
            }
        }
    } while (changed);

    std::cout << "to_cnf end" << std::endl;

    return s;
}

bool less::operator()(formula* f1, formula* f2) {
    bool res = true;
    if (f1->is_literal() && f2->is_literal()) {
        res = to_literal(f1)->var() < to_literal(f2)->var();
    } else if (f1->is_literal()) {
        res = false;
    } else if (f2->is_literal()) {
        res = true;
    } else {
        auto f1_j = to_junction_formula(f1);
        auto f2_j = to_junction_formula(f2);
        auto sub1 = f1_j->sub();
        auto sub2 = f2_j->sub();

        if (sub1.size() == sub2.size() && sub1.empty()) {
            res = false;
        } else if (sub1.size() == sub2.size() && sub1.size() == 1) {
            res = operator()(*sub1.begin(), *sub2.begin());
        } else if (f1_j->conn() == connective::AND && f2_j->conn() == connective::OR) {
            res = true;
        } else if (f1_j->conn() == connective::OR && f2_j->conn() == connective::AND) {
            res = false;
        } else if (sub1.size() < sub2.size()) {
            res = true;
        } else if (sub1.size() > sub2.size()) {
            res = false;
        } else {
            for (auto it1 = sub1.begin(), it2 = sub2.begin(); it1 != sub1.end(); it1++, it2++) {
                if (operator()(*it1,*it2)) {
                    break;
                } else if (operator()(*it2,*it1)) {
                    res = false;
                    break;
                }
            }
        }
    }
    // std::cout << f1->to_string() << " " << f2->to_string() << " " << res << std::endl;
    return res;
}

bool equal_cnf(formula* cnf1, formula* cnf2) {
    std::cout << "equal start" << std::endl;
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
                break;
            }
        }
    }
    std::cout << "equal end" << std::endl;
    return match;
}

bool is_true(formula* f) {
    if (!f->is_literal()) {
        return false;
    }
    return to_literal(f)->var() == 0;
}

literal* literal::create(uint64_t var) {
    auto l = new literal;
    l->v = var;
    return static_cast<literal*>(try_insert_into_global(l));
}

junction_formula* junction_formula::create(connective c) {
    auto j = new junction_formula;
    j->c = c;
    return static_cast<junction_formula*>(try_insert_into_global(j));
}

junction_formula* junction_formula::create(connective c, formula_set&& sf) {
    auto j = new junction_formula;
    j->c = c;
    j->sf = sf;
    return static_cast<junction_formula*>(try_insert_into_global(j));
}

junction_formula* junction_formula::create(connective c, const formula_set& sf) {
    auto j = new junction_formula;
    j->c = c;
    j->sf = sf;
    return static_cast<junction_formula*>(try_insert_into_global(j));
}

formula* junction_formula::create_conjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return f2;
    }
    if (is_true(f2)) {
        return f1;
    }
    auto conj = new junction_formula;
    merge_subformulas(connective::AND, conj, f1, f2);
    return try_insert_into_global(conj);
}

formula* junction_formula::create_disjunction(formula* f1, formula* f2) {
    if (is_true(f1)) {
        return f1;
    }
    if (is_true(f2)) {
        return f2;
    }
    auto conj = new junction_formula;
    merge_subformulas(connective::OR, conj, f1, f2);
    return try_insert_into_global(conj);
}

void junction_formula::merge_subformulas(connective c, junction_formula* merged, formula* f1, formula* f2) {
    merged->c = c;
    if (!f1->is_literal() && to_junction_formula(f1)->c == c) {
        merged->sf = to_junction_formula(f1)->sf;
        if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
            merged->sf.insert(to_junction_formula(f2)->sf.begin(), to_junction_formula(f2)->sf.end());
        } else {
            merged->sf.insert(f2);
        }
    } else if (!f2->is_literal() && to_junction_formula(f2)->c == c) {
        merged->sf = to_junction_formula(f2)->sf;
        merged->sf.insert(f2);
    } else {
        merged->sf.insert(f1);
        merged->sf.insert(f2);
    }
}

formula* formula::try_insert_into_global(formula* f) {
    auto res = global_formula_set.insert(f);
    // std::cout << global_formula_set.size() << std::endl;
    if (!res.second) {
        delete f;
    }
    return *res.first;
}

void list_global() {
    for (const auto& c : global_formula_set) {
        std::cout << c->to_string() << std::endl;
    }
}
