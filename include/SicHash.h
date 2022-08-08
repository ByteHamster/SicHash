#pragma once
#include <vector>
#include "IrregularCuckooHashTable.h"
#include "SimpleRibbon.h"
#include <EliasFano.h>

namespace sichash {
struct SicHashConfig {
    uint64_t threshold1 = UINT64_MAX / 100 * 50; // 50%
    uint64_t threshold2 = UINT64_MAX / 100 * 25; // 25%
    double loadFactor = 0.9;
    size_t smallTableSize = 5000;

    [[nodiscard]] double class1Percentage() const {
        return (double) threshold1 / (double) UINT64_MAX;
    }

    [[nodiscard]] double class2Percentage() const {
        return (double) (threshold2 - threshold1) / (double) UINT64_MAX;
    }

    [[nodiscard]] double class3Percentage() const {
        return (double) (UINT64_MAX - threshold2 - threshold1) / (double) UINT64_MAX;
    }

    void thresholdsPercentage(size_t percentage1, size_t percentage2) {
        threshold1 = UINT64_MAX / 100 * percentage1;
        threshold2 = UINT64_MAX / 100 * (percentage1 + percentage2);
    }
};

/**
 * SicHash perfect hash function.
 * @tparam minimal Remap values >N to empty slots to get a MPHF
 * @tparam ribbonWidth Tuning parameter for the ribbon retrieval data structure. Usually 64 or 32.
 * @tparam minimalFanoLowerBits Number of lower bits in the EliasFano coding for remapping.
 *                              Only interesting for minimal=true. See paper for details.
 *                              loadFactor < ~0.89 ==> use minimalFanoLowerBits=3
 *                              loadFactor < ~0.94 ==> use minimalFanoLowerBits=4
 *                              loadFactor < ~0.97 ==> use minimalFanoLowerBits=5
 */
template<bool minimal=false, size_t ribbonWidth=64, int minimalFanoLowerBits = 3>
class SicHash {
    public:
        static constexpr size_t HASH_FUNCTION_BUCKET_ASSIGNMENT = 42;
        SicHashConfig config;
        size_t numSmallTables;
        size_t N;
        std::vector<size_t> smallTableOffsets;
        std::vector<uint8_t> smallTableSeeds;
        SimpleRibbon<1, ribbonWidth> *ribbon1 = nullptr;
        SimpleRibbon<2, ribbonWidth> *ribbon2 = nullptr;
        SimpleRibbon<3, ribbonWidth> *ribbon3 = nullptr;
        std::vector<size_t> emptySlots;
        util::EliasFano<minimalFanoLowerBits> minimalRemap;

        SicHash(const std::vector<std::string> &keys,
                SicHashConfig _config)
                  : config(_config), N(keys.size()),
                        minimalRemap(minimal ? (N / config.loadFactor - N) : 0, minimal ? N : 0) {
            numSmallTables = keys.size() / config.smallTableSize + 1;
            std::vector<IrregularCuckooHashTable> tables;
            tables.reserve(numSmallTables);
            IrregularCuckooHashTableConfig cuckooConfig;
            cuckooConfig.threshold1 = config.threshold1;
            cuckooConfig.threshold2 = config.threshold2;
            cuckooConfig.maxEntries = config.smallTableSize * 1.2 + 100;
            for (size_t i = 0; i < numSmallTables; i++) {
                tables.emplace_back(cuckooConfig);
            }

            std::cout<<"Creating MHCs"<<std::endl;
            smallTableOffsets.reserve(numSmallTables);
            smallTableSeeds.reserve(numSmallTables);
            for (const std::string &key : keys) {
                HashedKey hash = HashedKey(key);
                size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
                tables[smallTable].prepare(hash);
            }

            std::cout<<"Inserting into Cuckoo"<<std::endl;
            std::vector<std::vector<std::pair<uint64_t, uint8_t>>> maps; // Avoids conditional jumps later
            maps.resize(0b111 + 1);
            maps[0b001].reserve(keys.size() * config.class1Percentage());
            maps[0b011].reserve(keys.size() * config.class2Percentage());
            maps[0b111].reserve(keys.size() * config.class3Percentage());
            size_t sizePrefix = 0;
            size_t unnecessaryConstructions = 0;
            for (IrregularCuckooHashTable &table : tables) {
                size_t tableM = table.size() / config.loadFactor;
                size_t seed = 0;
                while (!table.construct(tableM, seed)) {
                    unnecessaryConstructions++;
                    seed++;
                    if (seed >= 255) {
                        throw std::logic_error("Selected thresholds that cannot be constructed");
                    }
                }
                smallTableSeeds.push_back(seed);
                smallTableOffsets.push_back(sizePrefix);

                for (size_t i = 0; i < table.size(); i++) {
                    IrregularCuckooHashTable::TableEntry &entry = table.heap[i];
                    maps[entry.hashFunctionMask].emplace_back(entry.hash.mhc, entry.hashFunctionIndex);
                }
                if constexpr (minimal) {
                    for (size_t i = 0; i < tableM; i++) {
                        if (table.cells[i] == nullptr) {
                            emptySlots.push_back(sizePrefix + i);
                        }
                    }
                }
                sizePrefix += tableM;
            }

            smallTableOffsets.push_back(sizePrefix);
            std::cout<<"On average, the small hash tables needed to be retried "
                    <<(double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables<<" times"<<std::endl;

            std::cout<<"Constructing Ribbon"<<std::endl;
            ribbon1 = new SimpleRibbon<1, ribbonWidth>(maps[0b001]);
            ribbon2 = new SimpleRibbon<2, ribbonWidth>(maps[0b011]);
            ribbon3 = new SimpleRibbon<3, ribbonWidth>(maps[0b111]);

            if constexpr (minimal) {
                std::cout<<"Making minimal"<<std::endl;
                size_t smallTableToRemap = 0;
                while (smallTableOffsets[smallTableToRemap] < N) {
                    smallTableToRemap++;
                }
                smallTableToRemap--;
                // Iterate over last few tables and remap filled positions
                size_t emptyIndex = 0;
                size_t i = keys.size() - smallTableOffsets[smallTableToRemap];
                for (;smallTableToRemap < numSmallTables; smallTableToRemap++) {
                    for (; i < tables[smallTableToRemap].M; i++) {
                        minimalRemap.push_back(emptySlots[emptyIndex]);
                        if (tables[smallTableToRemap].cells[i] != nullptr) {
                            emptyIndex++;
                            if (emptySlots[emptyIndex] >= N) {
                                // No more empty slots left. We do not need to write the following items to
                                // the EF sequence because they are never queried
                                emptyIndex--;
                                break;
                            }
                        }
                    }
                    i = 0;
                }
                minimalRemap.buildRankSelect();
                emptySlots.clear();
                emptySlots.shrink_to_fit();
            }
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            size_t bytes = ribbon1->size() + ribbon2->size() + ribbon3->size()
                    + smallTableOffsets.size() * sizeof(size_t)
                    + smallTableSeeds.size() * sizeof(uint8_t);
            if constexpr (minimal) {
                bytes += minimalRemap.space();
            }
            return bytes * 8;
        }

        size_t operator() (std::string &key) const {
            HashedKey hash = HashedKey(key);
            size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
            uint8_t hashFunction;
            if (hash.mhc <= config.threshold1) {
                hashFunction = ribbon1->retrieve(hash.mhc);
            } else if (hash.mhc <= config.threshold2) {
                hashFunction = ribbon2->retrieve(hash.mhc);
            } else {
                hashFunction = ribbon3->retrieve(hash.mhc);
            }
            size_t M = smallTableOffsets[smallTable + 1] - smallTableOffsets[smallTable];
            size_t result = hash.hash(hashFunction + smallTableSeeds[smallTable], M) + smallTableOffsets[smallTable];
            if constexpr (minimal) {
                if (result >= N) {
                    return minimalRemap.at(result - N);
                }
            }
            return result;
        }
};
} // Namespace sichash
