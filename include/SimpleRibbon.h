#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

// Ribbon can only be included in one single file in the whole project
#define SIMPLE_RIBBON
#ifdef SIMPLE_RIBBON
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
#else
template<int bits>
class SimpleRibbon {
    public:
        explicit SimpleRibbon(std::vector<std::pair<uint64_t, uint8_t>> data) {}
        ~SimpleRibbon() {}
        uint8_t retrieve(uint64_t key) { return 0; }
        std::size_t size() { return 0; }
};
#endif
