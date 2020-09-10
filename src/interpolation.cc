#include "interpolation.h"

#include "bmc.h"

#undef LOGGING
#define LOGGING 0

using namespace std;

bool interpolation(circuit c) {
    bmc b(c);
    uint64_t k = 1; // first round is I(s0) /\ T(s0,s1) /\ B(s1)

    while (true) {
        if (b.run(k)) {
            return true;
        }
        uint64_t i = 0;
        auto interpolant = b.get_interpolant();
        while (true) {
#if LOGGING
            cout << "k=" << k << ", i=" << i++ << ", interpolant: " << interpolant << endl;
#endif
            if (b.run(k, interpolant)) {
                k++;
                break; // round is inconclusive, increase k
            }
            auto temp = b.get_interpolant();
            if (temp == interpolant) {
                return false; // interpolant has converged, return
            }
            interpolant = move(temp);
        }
    }
    return false; // this should never be reached, only here to prevent warning
}
