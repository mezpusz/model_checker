
#include "aiger_parser.h"
#include "bmc.h"
#include "interpolation.h"

#undef LOGGING
#define LOGGING 0

using namespace std;

void circuit_debug(const circuit& c) {
    cout << "circuit" << endl;
    cout << "- inputs" << endl;
    for (const auto& i : c.inputs) {
        cout << lit_to_string(i) << endl;
    }
    cout << "- latches" << endl;
    for (const auto& [i, o] : c.latches) {
        cout << lit_to_string(i) << " -> " << lit_to_string(o) << endl;
    }
    cout << "- ands" << endl;
    for (const auto& [i1, i2, o] : c.ands) {
        cout << lit_to_string(i1) << ", " << lit_to_string(i2) << " <-> " << lit_to_string(o) << endl;
    }
    cout << "- outputs" << endl;
    for (const auto& o : c.outputs) {
        cout << lit_to_string(o) << endl;
    }
    cout << endl;
}

int main(int argc, char** argv) {
    string input_file;
    int k = -1;
    if (argc < 2) {
        cerr << "Usage: ./model_checker input_file [k]" << endl;
        return -1;
    } else if (argc == 2) {
        input_file = argv[1];
    } else {
        input_file = argv[1];
        k = atoi(argv[2]);
    }

    circuit c;
    if (!parse_aiger_file(input_file, c)) {
        cerr << "Could not parse file " << input_file << endl;
        return -1;
    }

#if LOGGING
    circuit_debug(c);
#endif

    if (k == -1) {
        cout << (interpolation(move(c)) ? "FAIL" : "OK") << endl;
    } else {
        bmc b(c);
        cout << (b.run(k) ? "FAIL" : "OK") << endl;
    }

    return 0;
}
