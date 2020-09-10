#include "interpolation.h"

#include "bmc.h"

#define LOGGING 0

bool interpolation(circuit c) {
    bmc b(c);
    uint64_t k = 1;

    while (true) {
#if LOGGING
        std::cout << "k=" << k << std::endl;
#endif
        Cnf temp;
        temp.emplace_back();
        if (b.run(k, temp)) {
#if LOGGING
            std::cout << "Sat in first round of k=" << k << std::endl;
#endif
            return true;
        }
        unsigned i = 0;
        auto interpolant = b.get_interpolant();
        clean(interpolant);
#if LOGGING
        std::cout << "interpolant: " << interpolant << std::endl;
#endif

        while (true) {
            if (b.run(k, interpolant)) {
#if LOGGING
                std::cout << i << " iterations inside" << std::endl;
#endif
                k++;
                break;
            }
            auto temp = b.get_interpolant();
            clean(temp);
#if LOGGING
            std::cout << "interpolant: " << temp << std::endl;
#endif

            if (temp == interpolant) {
#if LOGGING
                std::cout << "Interpolant is same as in previous round" << std::endl;
#endif
                return false;
            }
            i++;
            interpolant = std::move(temp);
        }
    }
    return false;
}
