#pragma once

#include <set>
#include <string>
#include <vector>

std::string literal_to_string(uint64_t n);

enum class connective : uint8_t {
    AND,
    OR,
    UNSET,
};

struct formula {
    virtual ~formula() = default;
    virtual bool is_literal() const = 0;
    virtual std::string to_string() = 0;
    virtual formula* copy() = 0;
};

struct literal : formula {
    uint64_t var;

    bool is_literal() const override { return true; }
    std::string to_string() override { return literal_to_string(var); }
    formula* copy() override {
        auto c = new literal;
        c->var = var;
        return c;
    }
};

struct junction_formula : formula {
    std::vector<formula*> subformulas;
    connective conn = connective::UNSET;

    auto begin() {
        return subformulas.begin();
    }

    auto end() {
        return subformulas.end();
    }

    bool is_literal() const override { return false; }

    std::string to_string() override {
        std::string res = "(";
        for (uint64_t i = 0; i < subformulas.size(); i++) {
            if (subformulas[i] == nullptr) {
                res += "T";
            } else {
                res += subformulas[i]->to_string();
            }
            if (i < subformulas.size()-1) {
                if (conn == connective::AND) {
                    res += " & ";
                } else {
                    res += " | ";
                }
            }
        }
        res += ")";
        return res;
    }

    formula* copy() override {
        auto c = new junction_formula;
        c->conn = conn;
        c->subformulas.resize(subformulas.size());
        for (uint64_t i = 0; i < subformulas.size(); i++) {
            if (subformulas[i] == nullptr) {
                c->subformulas[i] = nullptr;
            } else {
                c->subformulas[i] = subformulas[i]->copy();
            }
        }
        return c;
    }
};

struct conjunction {
    std::vector<literal*> c;

    conjunction(literal* one) : c() {
        c.push_back(one);
    }
    conjunction(literal* one, literal* two) : c() {
        c.push_back(one);
        c.push_back(two);
    }
};

// cnf
void add_clause(formula* cnf, formula* cl);
void merge(formula* cnf, formula* other);
void add_equiv(formula* cnf, const conjunction& conj1, const conjunction& conj2);
formula* duplicate(formula* cnf, uint64_t shift);
void cleanse(formula* f); 

// misc
formula* negate_literal(formula* lit);
void cnf_debug(formula* cnf);
formula* to_cnf(formula* f);

inline literal* to_literal(formula* f) {
    // assert(f->is_literal());
    return static_cast<literal*>(f);
}

inline junction_formula* to_junction_formula(formula* f) {
    // assert(!f->is_literal());
    return static_cast<junction_formula*>(f);
}

inline junction_formula* to_conjunction(formula* f) {
    auto res = to_junction_formula(f);
    // assert(res->conn == connective::AND);
    return res;
}

inline junction_formula* to_disjunction(formula* f) {
    auto res = to_junction_formula(f);
    // assert(res->conn == connective::OR);
    return res;
}

bool equal(formula* cnf1, formula* cnf2);
bool is_true(formula* f);

junction_formula* duplicate_clause(junction_formula* cl);