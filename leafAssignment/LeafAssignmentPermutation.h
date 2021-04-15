#ifndef TESTCOMPARISON_LEAFASSIGNMENTPERMUTATION_H
#define TESTCOMPARISON_LEAFASSIGNMENTPERMUTATION_H

#include "LeafAssignment.h"
#include <bitset>
#include <vector>
#include <algorithm>

// https://arvid.io/2018/07/02/better-cxx-prng/
class xorshift
{
    public:
        using result_type = uint32_t;
        static constexpr result_type (min)() { return 0; }
        static constexpr result_type (max)() { return UINT32_MAX; }
        friend bool operator==(xorshift const &, xorshift const &);
        friend bool operator!=(xorshift const &, xorshift const &);

        xorshift() : m_seed(0xc1f651c67c62c6e0ull) {}

        explicit xorshift(uint64_t seed)
        {
            m_seed = seed;
            assert(seed != 0);
        }

        result_type operator()()
        {
            uint64_t result = m_seed * 0xd989bcacc137dcd5ull;
            m_seed ^= m_seed >> 11;
            m_seed ^= m_seed << 31;
            m_seed ^= m_seed >> 18;
            return uint32_t(result >> 32ull);
        }

        void discard(unsigned long long n)
        {
            for (unsigned long long i = 0; i < n; ++i)
                operator()();
        }

    private:
        uint64_t m_seed;
};

bool operator==(xorshift const &lhs, xorshift const &rhs)
{
    return lhs.m_seed == rhs.m_seed;
}
bool operator!=(xorshift const &lhs, xorshift const &rhs)
{
    return lhs.m_seed != rhs.m_seed;
}

// https://github.com/lemire/fastrange/blob/master/fastrange.h
static inline uint32_t fastrange32(uint32_t word, uint32_t p) {
    return (uint32_t)(((uint64_t)word * (uint64_t)p) >> 32);
}

template <uint8_t leafSize, uint8_t expansionSize>
class LeafAssignmentPermutation : public LeafAssignment<leafSize> {
    public:
        inline std::string name() override {
            return std::string("Permutation");
        }

        uint32_t firstPermutation[1<<expansionSize] = {0};
        const int numPermutationsToTest = 1e6;

        LeafAssignmentPermutation() {
            res.resize(expansionSize);

            int seeds = 0;
            double seedSum = 0;
            uint32_t compressed = (1<<leafSize) - 1;
            for (uint32_t mask = 0; mask < (1<<expansionSize); mask++) {
                if (sux::nu(mask) != leafSize) {
                    continue; // Never queried
                }
                if (mask % 234 == 0) {
                    std::cout<<"\rPre-calculating permutation "<<mask<<"/"<<(1<<expansionSize)<<std::flush;
                }

                uint32_t permutationIdx = 1;
                for (; permutationIdx < numPermutationsToTest; permutationIdx++) {
                    uint32_t permutated = permutate(mask, permutationIdx);
                    if (permutated == compressed) {
                        firstPermutation[mask] = permutationIdx;
                        seedSum += permutationIdx;
                        seeds++;
                        break;
                    }
                }

                if (permutationIdx == numPermutationsToTest) {
                    std::cerr<<"Did not find permutation"<<std::endl;
                    return;
                }
            }
            std::cout<<std::endl;
            std::cout<<"Average permutation: "<<seedSum/seeds<<std::endl;
        }

        inline uint32_t permutate(uint32_t inputInt, int seed) {
            std::vector<uint32_t> permutation = generatePermutation(seed);
            std::bitset<32> input(inputInt);
            std::bitset<32> output(0);
            for (int i = 0; i < expansionSize; i++) {
                output[permutation[i]] = input[i];
            }
            return uint32_t(output.to_ulong());
        }

        std::vector<uint32_t> res;
        inline std::vector<uint32_t> &generatePermutation(int seed) {
            for (std::size_t i = 0; i < expansionSize; i++) {
                res[i] = i;
            }
            std::shuffle(res.begin(), res.end(), xorshift(seed));
            return res;
        }

        inline int hash(uint64_t key, int hashfunction) override {
            uint32_t x = hashfunction & 0b1111111111;
            uint32_t seed = hashfunction >> 10;
            uint32_t collisionFreeIndex = fastrange32(sux::function::remix(key + x), expansionSize);
            std::vector<uint32_t> permutation = generatePermutation(seed);
            return permutation[collisionFreeIndex];
        }

        inline int assign(std::vector<uint64_t> &keys) override {
            uint32_t mask;
            uint32_t x = 0;

            for (;;) {
                mask = 0;
                for (size_t i = 0; i < leafSize; i++) {
                    mask |= uint32_t(1) << fastrange32(
                            sux::function::remix(keys[i] + x), expansionSize);
                }
                if (sux::nu(mask) == leafSize) {
                    break; // Found assignment without collision
                }
                x++;
            }
            return (firstPermutation[mask] << 10) + x;
        }
};
#endif //TESTCOMPARISON_LEAFASSIGNMENTPERMUTATION_H
