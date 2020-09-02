
#include <iostream>
#include <cstdlib>

#include "aiger_parser.h"
#include "bmc.h"
#include "interpolation.h"

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

    // if (k == -1) {
    //     std::cerr << "Bound was not given as argument" << std::endl;
    //     return -1;
    // }

    circuit c;
    if (!parse_aiger_file(input_file, c)) {
        return -1;
    }

    if (k == -1) {
        std::cout << (interpolation(std::move(c)) ? "sat" : "unsat") << std::endl;
    } else {
        bmc b(std::move(c));
        auto a = b.create_a(k);
        b.set_a(&a);
        std::cout << (b.run(k) ? "sat" : "unsat") << std::endl;
    }

    return 0;
}
