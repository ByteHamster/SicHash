#ifndef TESTCOMPARISON_LEAFASSIGNMENTRECSPLIT_H
#define TESTCOMPARISON_LEAFASSIGNMENTRECSPLIT_H

#include "LeafAssignment.h"
#include "../sux/sux/function/RecSplit.hpp"

template <size_t leafSize>
class LeafAssignmentRecSplit : public LeafAssignment<leafSize> {
    private:
        static constexpr sux::function::array<uint8_t, sux::function::MAX_LEAF_SIZE> bij_midstop = sux::function::fill_bij_midstop();
    public:
        inline std::string name() override {
            return "RecSplit\t";
        }

        inline int hash(uint64_t key, int hashfunction) override {
            return sux::function::remap16(sux::function::remix(key + hashfunction), leafSize);
        }

        inline int assign(std::vector<uint64_t> &keys) override {
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
#endif //TESTCOMPARISON_LEAFASSIGNMENTRECSPLIT_H
