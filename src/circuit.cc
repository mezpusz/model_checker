#include "circuit.h"
#include "formula.h"

#include <sstream>
#include <iostream>

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
    for (const auto& [kv, o] : c.ands) {
        std::cout << literal_to_string(kv.first) << ", " << literal_to_string(kv.second) << " -> " << literal_to_string(o) << std::endl;
    }
    std::cout << "- outputs" << std::endl;
    for (const auto& o : c.outputs) {
        std::cout << literal_to_string(o) << std::endl;
    }
    std::cout << std::endl;
}