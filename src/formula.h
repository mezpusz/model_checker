#pragma once

#include <set>
#include <string>
#include <vector>
#include <cassert>

using clause = std::set<uint64_t>;
using Cnf = std::set<clause>;

std::string literal_to_string(uint64_t n);

// misc
uint64_t negate_literal(uint64_t lit);
std::ostream& operator<<(std::ostream& out, const clause& cl);
std::ostream& operator<<(std::ostream& out, const Cnf& cnf);
Cnf to_cnf_or(const Cnf& lhs, const Cnf& rhs);
void clean(Cnf& cnf);
