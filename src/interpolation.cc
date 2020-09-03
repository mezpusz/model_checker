#include "interpolation.h"

#include "bmc.h"

bool interpolation(circuit c) {
    auto shift = c.shift();
    std::cout << shift << std::endl;
    bmc b(c);

    uint64_t k = 1;
    while (k<shift) {
        std::cout << "k=" << k << std::endl;
        Cnf temp;
        temp.emplace();
        if (b.run(k, temp)) {
            std::cout << "Sat in first round of k=" << k << std::endl;
            return true;
        }
        unsigned i = 0;
        auto interpolant = b.get_interpolant();
        clean(interpolant);
        std::cout << "interpolant: " << interpolant << std::endl;

        while (true) {
            if (b.run(k, interpolant)) {
                // std::cout << i << " iterations inside" << std::endl;
                k++;
                break;
            }
            auto temp = b.get_interpolant();
            clean(temp);
            std::cout << "interpolant: " << temp << std::endl;

            if (temp == interpolant) {
                std::cout << "Interpolant is same as in previous round" << std::endl;
                return false;
            }
            i++;
            interpolant = std::move(temp);
        }
    }
    return false;
}
