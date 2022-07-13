#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace sichash {
template<size_t bits, size_t coeff_bits = 64>
class SimpleRibbon {
    private:
        void *ribbon;
    public:
        explicit SimpleRibbon(std::vector<std::pair<uint64_t, uint8_t>> &data);
        ~SimpleRibbon();
        uint8_t retrieve(uint64_t key);
        std::size_t size();
};
} // Namespace sichash
