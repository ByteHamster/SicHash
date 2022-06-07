#include <vector>
#include <random>
#include "sux/function/RecSplit.hpp"

static const int MAX_LEAF_SIZE = 24;
#define DO_NOT_OPTIMIZE(value) asm volatile ("" : : "r,m"(value) : "memory")

/**
 * Copy of RecSplit's bijection code.
 */
template <size_t leafSize>
class BijectionsRecSplit {
    public:
        static constexpr std::array<uint8_t, MAX_LEAF_SIZE> fill_bij_midstop() {
            std::array<uint8_t, MAX_LEAF_SIZE> memo{0};
            for (int s = 0; s < MAX_LEAF_SIZE; ++s) memo[s] = s < (int)ceil(2 * sqrt(s)) ? s : (int)ceil(2 * sqrt(s));
            return memo;
        }
        static constexpr sux::function::array<uint8_t, sux::function::MAX_LEAF_SIZE> bij_midstop = fill_bij_midstop();

        inline int calculateBijection(std::vector<uint64_t> &keys) {
            uint32_t mask;
            int x = 0;
            const uint32_t found = (1 << leafSize) - 1;

            if constexpr (leafSize <= 8) {
                for (;;) {
                    mask = 0;
                    for (size_t i = 0; i < leafSize; i++) {
                        mask |= uint32_t(1) << sux::function::remap16(
                                sux::function::remix(keys[i] + x), leafSize);
                    }
                    if (mask == found) {
                        return x;
                    }
                    x++;
                }
            } else {
                const size_t midstop = bij_midstop[leafSize];
                for (;;) {
                    mask = 0;
                    size_t i;
                    for (i = 0; i < midstop; i++) {
                        mask |= uint32_t(1) << sux::function::remap16(
                                sux::function::remix(keys[i] + x), leafSize);
                    }
                    if (sux::nu(mask) == midstop) {
                        for (; i < leafSize; i++) {
                            mask |= uint32_t(1) << sux::function::remap16(
                                    sux::function::remix(keys[i] + x), leafSize);
                        }
                        if (mask == found) {
                            return x;
                        }
                    }
                    x++;
                }
            }
        }
};

/**
 * Alternative method of finding bijections that rotates two halves.
 */
template <uint8_t leafSize, bool useLookupTable>
class BijectionsRotate {
    public:
        // Only used when useLookupTable is true.
        // normalize_rotations[x] contains i such that rotate(x, i) is minimal.
        // For two numbers (x,y) that can be rotated to match, we get:
        // rotate(x, normalize_rotations[x]) == rotate(y, normalize_rotations[y]).
        // To calculate how many times to rotate y to match x, we can subtract the number of rotations.
        uint8_t normalize_rotations[1<<leafSize] = {0};

        static inline constexpr uint16_t rotate(uint16_t val, uint8_t x) {
            return ((val << x) | (val >> (leafSize - x))) & ((1<<leafSize) - 1);
        }

        // Temporary variables
        uint64_t itemsLeft[leafSize] = {0};
        uint64_t itemsRight[leafSize] = {0};

        BijectionsRotate() {
            if constexpr (useLookupTable) {
                // This can probably be more efficient and calculated at compile time
                for (uint16_t pos = 1; pos != uint16_t(1<<leafSize); pos++) {
                    uint16_t x = pos;
                    uint16_t min = x;
                    normalize_rotations[pos] = 0;
                    for (uint8_t i = 0; i < leafSize; i++) {
                        x = rotate(x, 1);
                        if (x < min) {
                            min = x;
                            normalize_rotations[pos] = i + 1;
                        }
                    }
                }
            }
        }

        inline int calculateBijection(std::vector<uint64_t> &keys) {
            static_assert(leafSize <= 16, "Using uint16_t, so 16 is the maximum leaf size");

            // Split objects into two groups ("left" and "right")
            int itemsLeftCount = 0;
            int itemsRightCount = 0;
            for (int i = 0; i < leafSize; i++) {
                bool isLeft = (sux::function::remix(keys[i] - 1) % 2) == 0;
                if (isLeft) {
                    itemsLeft[itemsLeftCount] = keys[i];
                    itemsLeftCount++;
                } else {
                    itemsRight[itemsRightCount] = keys[i];
                    itemsRightCount++;
                }
            }

            constexpr uint16_t found = uint16_t(1<<leafSize) - 1;
            for (int x = 0; true; x += leafSize) {
                uint16_t maskLeft = 0;
                uint16_t maskRight = 0;
                for (size_t i = 0; i < itemsLeftCount; i++) {
                    maskLeft |= uint16_t(1) << sux::function::remap16(sux::function::remix(itemsLeft[i] + x), leafSize);
                }
                if (sux::nu(maskLeft) != itemsLeftCount) {
                    continue; // Collisions in left part
                }
                for (size_t i1 = 0; i1 < itemsRightCount; i1++) {
                    maskRight |= uint16_t(1) << sux::function::remap16(sux::function::remix(itemsRight[i1] + x), leafSize);
                }
                if (sux::nu(maskRight) != itemsRightCount) {
                    continue; // Collisions in right part
                }
                // Try to rotate right part to see if both together form a bijection
                if constexpr (useLookupTable) {
                    uint8_t rotations = (normalize_rotations[uint16_t(~maskRight) & found]
                                         - normalize_rotations[maskLeft] + leafSize) % leafSize;
                    if (found == (maskLeft | rotate(maskRight, rotations))) {
                        return x + rotations;
                    }
                } else {
                    for (int rotations = 0; rotations < leafSize; rotations++) {
                        if (found == (maskLeft | maskRight)) {
                            return x + rotations;
                        }
                        maskRight = rotate(maskRight, 1);
                    }
                }
            }
        }
};

template<class T>
double testSingle(size_t iterations) {
    T t;
    std::vector<std::vector<uint64_t>> leaves;
    leaves.resize(iterations);

    std::mt19937_64 mt(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0ull, ~0ull);
    for (size_t i = 0; i < iterations; i++) {
        for (size_t j = 0; j < 16; j++) {
            leaves.at(i).push_back(dist(mt));
        }
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        int x = t.calculateBijection(leaves[i]);
        DO_NOT_OPTIMIZE(x);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    return 0.001 * (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / (double)iterations;
}

template<uint8_t leafSize>
void test(size_t iterations) {
    std::cout<<"Leaf size "<<(int)leafSize<<std::endl;
    std::cout<<"Normal:        "<<testSingle<BijectionsRecSplit<leafSize>>(iterations)<<" us/leaf"<<std::endl;
    std::cout<<"Rotate:        "<<testSingle<BijectionsRotate<leafSize, false>>(iterations)<<" us/leaf"<<std::endl;
    std::cout<<"Rotate lookup: "<<testSingle<BijectionsRotate<leafSize, true>>(iterations)<<" us/leaf"<<std::endl;
    std::cout<<std::endl;
}

int main() {
    test<8>(100000);
    test<12>(6000);
    test<16>(500);
}