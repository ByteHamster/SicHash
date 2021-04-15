#ifndef TESTCOMPARISON_LEAFASSIGNMENTRECSPLITROTATE_H
#define TESTCOMPARISON_LEAFASSIGNMENTRECSPLITROTATE_H

#include "LeafAssignment.h"

template <uint8_t leafSize, bool useLookupTable>
class LeafAssignmentRecSplitRotate : public LeafAssignment<leafSize> {
    public:
        inline std::string name() override {
            return std::string("Rotate") + (useLookupTable ? "Lookup" : "\t\t");
        }

        uint8_t normalize_rotations[1<<leafSize] = {0};

        static inline constexpr uint16_t rol(uint16_t val) {
            return ((val << 1) | (val >> (leafSize - 1))) & ((1<<leafSize) - 1);
        }

        static inline constexpr uint16_t rol(uint16_t val, uint8_t x) {
            return ((val << x) | (val >> (leafSize - x))) & ((1<<leafSize) - 1);
        }

        LeafAssignmentRecSplitRotate() {
            if constexpr (useLookupTable) {
                for (uint16_t pos = 1; pos != uint16_t(1<<leafSize); pos++) {
                    uint16_t x = pos;
                    uint16_t min = x;
                    normalize_rotations[pos] = 0;
                    for (uint8_t i = 0; i < leafSize; i++) {
                        x = rol(x);
                        if (x < min) {
                            min = x;
                            normalize_rotations[pos] = i + 1;
                        }
                    }
                }
            }
        }

        inline int hash(uint64_t key, int hashfunction) override {
            bool isLeft = (sux::function::remix(key - 1) % 2) == 0;
            uint64_t shift = hashfunction % leafSize;
            uint64_t function = hashfunction - shift;
            uint64_t offset = isLeft ? 0 : shift;
            return (sux::function::remap16(sux::function::remix(key + function), leafSize) + offset) % leafSize;
        }

        inline int assign(std::vector<uint64_t> &keys) override {
            static_assert(leafSize <= 16, "Using uint16_t, so 16 is the maximum leaf size");

            int itemsLeftCount = 0;
            uint64_t itemsLeft[leafSize];
            int itemsRightCount = 0;
            uint64_t itemsRight[leafSize];
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

            int x = 0;
            constexpr uint16_t found = uint16_t(1<<leafSize) - 1;
            for (;;) {
                uint16_t maskLeft = 0;
                uint16_t maskRight = 0;
                size_t i;
                for (i = 0; i < itemsLeftCount; i++) {
                    maskLeft |= uint16_t(1) << sux::function::remap16(
                            sux::function::remix(itemsLeft[i] + x), leafSize);
                }
                if (sux::nu(maskLeft) == itemsLeftCount) {
                    for (i = 0; i < itemsRightCount; i++) {
                        maskRight |= uint16_t(1) << sux::function::remap16(
                                sux::function::remix(itemsRight[i] + x), leafSize);
                    }
                    if (sux::nu(maskRight) == itemsRightCount) {
                        if constexpr (useLookupTable) {
                            uint8_t rotations = (normalize_rotations[uint16_t(~maskRight) & found]
                                                - normalize_rotations[maskLeft] + leafSize) % leafSize;
                            if (found == (maskLeft | rol(maskRight, rotations))) {
                                return x + rotations;
                            }
                        } else {
                            for (int rot = 0; rot < leafSize; rot++) {
                                if (found == (maskLeft | maskRight)) {
                                    return x + rot;
                                }
                                maskRight = rol(maskRight);
                            }
                        }
                    }
                }
                x += leafSize;
            }
        }
};
#endif //TESTCOMPARISON_LEAFASSIGNMENTRECSPLITROTATE_H
