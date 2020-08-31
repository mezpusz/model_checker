#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <cassert>

class formula {
public:
    virtual ~formula() = default;
    virtual bool is_literal() const = 0;
    virtual std::string to_string() = 0;

    bool manual_refcount = false;
};

using formula_set = std::set<formula*>;

std::string literal_to_string(uint64_t n);

enum class connective : uint8_t {
    AND,
    OR,
    UNSET,
};

class literal : public formula {
public:
    bool is_literal() const override { return true; }
    std::string to_string() override { return literal_to_string(v); }
    uint64_t var() const { return v; }
private:
    literal() = default;
    uint64_t v;

    friend class formula_store;
};

class junction_formula : public formula {
public:
    size_t size() {
        return sf->size();
    }

    auto begin() {
        return sf->begin();
    }

    auto end() {
        return sf->end();
    }

    bool is_literal() const override { return false; }

    std::string to_string() override {
        std::string res = "(";
        for (auto it = sf->begin(); it != sf->end();) {
            res += (*it)->to_string();
            it++;
            if (it != sf->end()) {
                if (c == connective::AND) {
                    res += " & ";
                } else {
                    res += " | ";
                }
            }
        }
        res += ")";
        return res;
    }

    connective conn() { return c; }
    formula_set sub() { return *sf; }
    const formula_set& sub() const { return *sf; }

private:
    junction_formula() = default;

    const formula_set* sf = nullptr;
    connective c = connective::UNSET;

    friend class formula_store;
};

class formula_store {
public:
    literal* create(uint64_t var);
    void decrease_literal_refcount(uint64_t var);

    void log_static();

    junction_formula* create(connective c);
    junction_formula* create(connective c, formula_set&& sf);
    junction_formula* create(connective c, const formula_set& sf);
    junction_formula* create_conjunction(formula* f1, formula* f2);
    junction_formula* create_disjunction(formula* f1, formula* f2);
    junction_formula* wrap_single_formula(connective c, formula* f);
    void decrease_junction_refcount(connective c, const formula_set& sf, bool subformulas);

private:
    formula_set merge_subformulas(connective c, formula* f1, formula* f2);

    std::map<formula_set, std::pair<junction_formula*, uint64_t>>& get(connective c) {
        return c == connective::AND ? ands : ors;
    }

    std::map<uint64_t, std::pair<literal*, uint64_t>> literals;
    std::map<formula_set, std::pair<junction_formula*, uint64_t>> ands;
    std::map<formula_set, std::pair<junction_formula*, uint64_t>> ors;
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
void add_clause(formula*& cnf, formula* cl, formula_store* store);
void merge(formula*& cnf, formula* other, formula_store* store);
void add_equiv(formula*& cnf, const conjunction& conj1, const conjunction& conj2, formula_store* store);
formula* duplicate(formula* cnf, uint64_t shift, formula_store* store);

// misc
formula* negate_literal(formula* lit, formula_store* store);
void cnf_debug(formula* cnf);
formula* to_cnf(formula* f, formula_store* store);
formula* to_cnf(formula* s, formula* f, formula_store* store);

inline literal* to_literal(formula* f) {
    assert(f->is_literal());
    return static_cast<literal*>(f);
}

inline junction_formula* to_junction_formula(formula* f) {
    assert(!f->is_literal());
    return static_cast<junction_formula*>(f);
}

inline junction_formula* to_conjunction(formula* f) {
    auto res = to_junction_formula(f);
    assert(res->conn() == connective::AND);
    return res;
}

inline junction_formula* to_disjunction(formula* f) {
    auto res = to_junction_formula(f);
    assert(res->conn() == connective::OR);
    return res;
}

bool equal_cnf(formula* cnf1, formula* cnf2);
bool is_true(formula* f);

void log_static();