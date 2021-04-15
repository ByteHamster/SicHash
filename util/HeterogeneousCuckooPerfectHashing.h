#pragma once
#include <vector>
#include "HeterogeneousCuckooHashTable.h"
#include "SimpleRibbon.h"

template<size_t ribbonWidth=64>
class HeterogeneousCuckooPerfectHashing {
    public:
        static constexpr size_t HASH_FUNCTION_BUCKET_ASSIGNMENT = 42;
        static constexpr size_t SMALL_TABLE_SIZE = 5000;
        static constexpr size_t SMALL_TABLE_MAX_SIZE = 6000;
        size_t numSmallTables;
        std::vector<size_t> smallTableOffsets;
        std::vector<uint8_t> smallTableSeeds;
        SimpleRibbon<1, ribbonWidth> *ribbon1 = nullptr;
        SimpleRibbon<2, ribbonWidth> *ribbon2 = nullptr;
        SimpleRibbon<3, ribbonWidth> *ribbon3 = nullptr;
        uint64_t threshold1;
        uint64_t threshold2;

        HeterogeneousCuckooPerfectHashing(const std::vector<std::string> &keys,
                                          double loadFactor, size_t _threshold1, size_t _threshold2) {
            numSmallTables = keys.size() / SMALL_TABLE_SIZE + 1;
            threshold1 = UINT64_MAX / 100 * _threshold1;
            threshold2 = UINT64_MAX / 100 * (_threshold1 + _threshold2);
            std::vector<HeterogeneousCuckooHashTable> tables;
            tables.reserve(numSmallTables);
            for (size_t i = 0; i < numSmallTables; i++) {
                tables.emplace_back(SMALL_TABLE_MAX_SIZE, threshold1, threshold2);
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
            maps[0b001].reserve(keys.size() / 100 * _threshold1);
            maps[0b011].reserve(keys.size() / 100 * _threshold2);
            maps[0b111].reserve(keys.size() / 100 * (100 - _threshold1 - threshold2));
            size_t sizePrefix = 0;
            size_t unnecessaryConstructions = 0;
            for (HeterogeneousCuckooHashTable &table : tables) {
                size_t tableM = table.numEntries / loadFactor;
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

                for (size_t i = 0; i < table.numEntries; i++) {
                    HeterogeneousCuckooHashTable::TableEntry &entry = table.heap[i];
                    maps[entry.hashFunctionMask].emplace_back(entry.hash.mhc, entry.hashFunctionIndex);
                }
            }
            smallTableOffsets.push_back(sizePrefix);
            std::cout<<"In total, "<<unnecessaryConstructions<<" of "<<numSmallTables<<" hash tables were retried."<<std::endl;

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
            if (hash.mhc <= threshold1) {
                hashFunction = ribbon1->retrieve(hash.mhc);
            } else if (hash.mhc <= threshold2) {
                hashFunction = ribbon2->retrieve(hash.mhc);
            } else {
                hashFunction = ribbon3->retrieve(hash.mhc);
            }
            size_t M = smallTableOffsets[smallTable + 1] - smallTableOffsets[smallTable];
            return hash.hash(hashFunction + smallTableSeeds[smallTable], M) + smallTableOffsets[smallTable];
        }
};
