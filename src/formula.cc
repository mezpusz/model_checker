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
    auto v = to_literal(lit)->var;
    auto res = new literal;
    res->var = (v%2 == 0) ? (v + 1) : (v - 1);
    return res;
}

void add_clause(formula* cnf, formula* cl) {
    auto cnf_j = to_conjunction(cnf);
    auto cl_j = to_disjunction(cl);
    cnf_j->subformulas.push_back(cl_j);
}

void merge(formula* cnf, formula* other) {
    auto cnf_j = to_junction_formula(cnf);
    auto other_j = to_junction_formula(other);
    cnf_j->subformulas.insert(cnf_j->end(), other_j->begin(), other_j->end());
}

void cleanse(formula* f) {
    bool changed = false;
    do {
        changed = false;
        if (f->is_literal()) {
            return;
        }
        auto f_j = to_junction_formula(f);
        for (auto& sf : *f_j) {
            cleanse(sf);
            if (is_true(sf)) {
                if (f_j->conn == connective::OR) {
                    auto l = new literal;
                    l->var = 0;
                    f = l;
                    return;
                } else if (f_j->conn == connective::AND) {
                    // f_j->subformulas.erase(sf);
                } else {
                    assert(false);
                }
            }
        }
    } while (changed);
}

void add_equiv(formula* cnf, const conjunction& conj1, const conjunction& conj2) {
    auto cl = new junction_formula;
    cl->conn = connective::OR;
    auto cnf_j = to_junction_formula(cnf);
    for (const auto& l : conj1.c) {
        assert(l->is_literal());
        cl->subformulas.push_back(negate_literal(l));
    }
    for (const auto& l : conj2.c) {
        // assert(cl.lits.count(l) == 0);
        cl->subformulas.push_back(l);
        cnf_j->subformulas.push_back(duplicate_clause(cl));
        cl->subformulas.pop_back();
    }
    cl->subformulas.clear();
    for (const auto& l : conj2.c) {
        assert(l->is_literal());
        cl->subformulas.push_back(negate_literal(l));
    }
    for (const auto& l : conj1.c) {
        // assert(cl.lits.count(l) == 0);
        cl->subformulas.push_back(l);
        cnf_j->subformulas.push_back(duplicate_clause(cl));
        cl->subformulas.pop_back();
    }
}

formula* duplicate(formula* cnf, uint64_t shift) {
    auto cnf_j = to_conjunction(cnf);
    auto res = new junction_formula;
    res->conn = connective::AND;
    for (const auto& cl : *cnf_j) {
        auto cl_j = to_disjunction(cl);
        auto cl_n = new junction_formula;
        cl_n->conn = connective::OR;
        for (const auto& lit : *cl_j) {
            auto lit_j = to_literal(lit);
            auto lit_n = new literal;
            lit_n->var = lit_j->var + shift;
            cl_n->subformulas.push_back(lit_n);
        }
        add_clause(res, cl_n);
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

junction_formula* duplicate_clause(junction_formula* cl) {
    auto dupl = new junction_formula;
    dupl->conn = connective::OR;
    for (const auto& lit : *cl) {
        dupl->subformulas.push_back(lit->copy());
    }
    return dupl;
}

formula* to_cnf(formula* f) {
    std::cout << "to_cnf start" << std::endl;
    if (f == nullptr) {
        return f;
    }
    auto s = new junction_formula;
    s->conn = connective::AND;
    auto c = new junction_formula;
    c->conn = connective::OR;
    c->subformulas.push_back(f);
    s->subformulas.push_back(c);

    bool changed = false;
    do {
        changed = false;
        for (auto it = s->begin(); it != s->end(); it++) {
            auto sf = *it;
            auto cl = to_disjunction(sf);

            for (uint64_t i = 0; i < cl->subformulas.size();) {
                auto curr = cl->subformulas[i];
                if (!curr->is_literal()) {
                    auto bf = to_junction_formula(curr);
                    auto sf = bf->subformulas;
                    auto conn = bf->conn;
                    if (conn == connective::AND) {
                        for (uint64_t j = 1; j < sf.size(); j++) {
                            auto dup_cl = duplicate_clause(cl);
                            dup_cl->subformulas[i] = sf[j];
                            s->subformulas.push_back(dup_cl);
                        }
                        cl->subformulas[i] = sf[0];
                    } else {
                        for (uint64_t j = 1; j < sf.size(); j++) {
                            cl->subformulas.push_back(sf[j]);
                        }
                        cl->subformulas[i] = sf[0];
                    }
                    delete curr;
                    changed = true;
                    // break;
                } else {
                    // if (is_true(curr)) {
                    //     s->subformulas.erase(it);
                    //     changed = true;
                    //     break;
                    // }
                    i++;
                }
            }
            // std::cout << "to_cnf new formula: " << s->to_string() << std::endl;
            // if (changed) {
            //     // std::cout << "to_cnf new formula: " << formula_to_string(s) << std::endl;
            //     break;
            // }
        }
    } while (changed);

    std::cout << "to_cnf end" << std::endl;

    return s;
}

literal* to_literal(formula* f) {
    // assert(f->is_literal());
    return static_cast<literal*>(f);
}

junction_formula* to_junction_formula(formula* f) {
    // assert(!f->is_literal());
    return static_cast<junction_formula*>(f);
}

junction_formula* to_conjunction(formula* f) {
    auto res = to_junction_formula(f);
    // assert(res->conn == connective::AND);
    return res;
}

junction_formula* to_disjunction(formula* f) {
    auto res = to_junction_formula(f);
    // assert(res->conn == connective::OR);
    return res;
}

bool equal(formula* cnf1, formula* cnf2) {
    std::cout << "equal start" << std::endl;
    auto cnf1_j = to_conjunction(cnf1);
    auto cnf2_j = to_conjunction(cnf2);
    bool match = true;
    for (const auto& cl1 : *cnf1_j) {
        std::set<uint64_t> lits;
        auto cl1_j = to_disjunction(cl1);
        for (const auto& lit1 : *cl1_j) {
            lits.insert(to_literal(lit1)->var);
        }
        for (const auto& cl2 : *cnf2_j) {
            auto cl2_j = to_disjunction(cl2);
            bool found = true;
            for (const auto& lit2 : *cl2_j) {
                if (lits.count(to_literal(lit2)->var) == 0) {
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
    auto l = to_literal(f);
    return l->var == 0;
}
