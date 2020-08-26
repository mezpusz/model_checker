#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

class formula {
public:
    virtual ~formula() = default;
    virtual bool is_literal() const = 0;
    virtual std::string to_string() = 0;
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
    static literal* create(uint64_t var);

    bool is_literal() const override { return true; }
    std::string to_string() override { return literal_to_string(v); }
    uint64_t var() const { return v; }
private:
    literal() = default;
    uint64_t v;
};

static std::map<uint64_t, literal*> global_literals;

class junction_formula : public formula {
public:
    static junction_formula* create(connective c);
    static junction_formula* create(connective c, formula_set&& sf);
    static junction_formula* create(connective c, const formula_set& sf);
    static junction_formula* create_conjunction(formula* f1, formula* f2);
    static junction_formula* create_disjunction(formula* f1, formula* f2);
    static junction_formula* wrap_single_formula(connective c, formula* f);

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

private:
    junction_formula() = default;

    const formula_set* sf = nullptr;
    connective c = connective::UNSET;

    static formula_set merge_subformulas(connective c, formula* f1, formula* f2);
};

static std::map<formula_set, junction_formula*> global_ands;
static std::map<formula_set, junction_formula*> global_ors;

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

void list_global();

// cnf
void add_clause(formula*& cnf, formula* cl);
void merge(formula*& cnf, formula* other);
void add_equiv(formula*& cnf, const conjunction& conj1, const conjunction& conj2);
formula* duplicate(formula* cnf, uint64_t shift);

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

bool equal_cnf(formula* cnf1, formula* cnf2);
bool is_true(formula* f);
