
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "formula.h"

cnf create_initial(const circuit& c) {
    cnf res;
    for (const auto& [i, o] : c.latches) {
        clause cl;
        cl.lits.insert(negate_literal(o));
        res.add_clause(cl);
    }
    return res;
}

int main(int argc, char** argv) {
    std::string input_file;
    int k = -1;
    if (argc < 2) {
        std::cerr << "Usage: ./model_checker input_file [k]" << std::endl;
        return -1;
    } else if (argc == 2) {
        input_file = argv[1];
    } else {
        input_file = argv[1];
        k = atoi(argv[2]);
    }

    circuit c;
    if (!parse_aiger_file(input_file, c)) {
        return -1;
    }

    circuit_debug(c);

    cnf_debug(create_initial(c));

    return 0;
}