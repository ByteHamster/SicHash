#pragma once
#include <vector>
#include "HeterogeneousCuckooHashTable.h"
#include "SimpleRibbon.h"

struct HeterogeneousPerfectHashingConfig {
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

template<size_t ribbonWidth=64>
class HeterogeneousCuckooPerfectHashing {
    public:
        static constexpr size_t HASH_FUNCTION_BUCKET_ASSIGNMENT = 42;
        HeterogeneousPerfectHashingConfig config;
        size_t numSmallTables;
        std::vector<size_t> smallTableOffsets;
        std::vector<uint8_t> smallTableSeeds;
        SimpleRibbon<1, ribbonWidth> *ribbon1 = nullptr;
        SimpleRibbon<2, ribbonWidth> *ribbon2 = nullptr;
        SimpleRibbon<3, ribbonWidth> *ribbon3 = nullptr;

        HeterogeneousCuckooPerfectHashing(const std::vector<std::string> &keys,
                                          HeterogeneousPerfectHashingConfig _config)
                  : config(_config) {
            numSmallTables = keys.size() / config.smallTableSize + 1;
            std::vector<HeterogeneousCuckooHashTable> tables;
            tables.reserve(numSmallTables);
            HeterogeneousCuckooConfig cuckooConfig;
            cuckooConfig.threshold1 = config.threshold1;
            cuckooConfig.threshold2 = config.threshold2;
            cuckooConfig.maxEntries = config.smallTableSize * 1.2;
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
            for (HeterogeneousCuckooHashTable &table : tables) {
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
                sizePrefix += tableM;

                for (size_t i = 0; i < table.size(); i++) {
                    HeterogeneousCuckooHashTable::TableEntry &entry = table.heap[i];
                    maps[entry.hashFunctionMask].emplace_back(entry.hash.mhc, entry.hashFunctionIndex);
                }
            }
            smallTableOffsets.push_back(sizePrefix);
            std::cout<<"On average, the small hash tables needed to be retried "
                    <<(double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables<<" times"<<std::endl;

            std::cout<<"Constructing Ribbon"<<std::endl;
            ribbon1 = new SimpleRibbon<1, ribbonWidth>(maps[0b001]);
            ribbon2 = new SimpleRibbon<2, ribbonWidth>(maps[0b011]);
            ribbon3 = new SimpleRibbon<3, ribbonWidth>(maps[0b111]);
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            return 8 * (ribbon1->size() + ribbon2->size() + ribbon3->size()
                    + smallTableOffsets.size() * sizeof(size_t)
                    + smallTableSeeds.size() * sizeof(uint8_t));
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
            return hash.hash(hashFunction + smallTableSeeds[smallTable], M) + smallTableOffsets[smallTable];
        }
};
