#include "aiger_parser.h"
#include "bmc.h"
#include "interpolation.h"

void circuit_debug(const circuit& c) {
    std::cout << "circuit" << std::endl;
    std::cout << "- inputs" << std::endl;
    for (const auto& i : c.inputs) {
        std::cout << literal_to_string(i) << std::endl;
    }
    std::cout << "- latches" << std::endl;
    for (const auto& [i, o] : c.latches) {
        std::cout << literal_to_string(i) << " -> " << literal_to_string(o) << std::endl;
    }
    std::cout << "- ands" << std::endl;
    for (const auto& [i1, i2, o] : c.ands) {
        std::cout << literal_to_string(i1) << ", " << literal_to_string(i2) << " <-> " << literal_to_string(o) << std::endl;
    }
    std::cout << "- outputs" << std::endl;
    for (const auto& o : c.outputs) {
        std::cout << literal_to_string(o) << std::endl;
    }
    std::cout << std::endl;
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

#if LOGGING
    circuit_debug(c);
#endif

    if (k == -1) {
        std::cout << (interpolation(std::move(c)) ? "sat" : "unsat") << std::endl;
    } else {
        bmc b(c, false);
        Cnf temp;
        temp.emplace_back();
        std::cout << (b.run(k, temp) ? "sat" : "unsat") << std::endl;
    }

    return 0;
}
