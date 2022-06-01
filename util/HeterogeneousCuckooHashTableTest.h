#pragma once
#include "HeterogeneousCuckooHashTable.h"

/**
 * This cuckoo hash table is used for certain types of benchmarks that do NOT measure timing.
 * The implementation is more flexible than HeterogeneousCuckooHashTable
 * (as it supports a more flexible number of hash functions)
 * but is not optimized for performance.
 */
class HeterogeneousCuckooHashTableTest {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t numHashFunctions = 0;
        };
        TableEntry *heap;
    private:
        std::vector<TableEntry*> cells;
        size_t M = 0;
        size_t numEntries = 0;
        size_t seed = 0;
        std::vector<std::pair<uint64_t, uint8_t>> thresholds;
    public:
        explicit HeterogeneousCuckooHashTableTest(std::vector<std::pair<uint64_t, uint8_t>> thresholds_, size_t maxEntries)
                : thresholds(thresholds_) {
            heap = new TableEntry[maxEntries];
        }

        ~HeterogeneousCuckooHashTableTest() {
            delete heap;
        }

        static std::string name() {
            return "HeterogeneousCuckooHashTableTest";
        }

        void prepare(HashedKey hash) {
            heap[numEntries].hash = hash;
            for (auto [thresh, num] : thresholds) {
                if (hash.mhc <= thresh) {
                    heap[numEntries].numHashFunctions = num;
                    numEntries++;
                    return;
                }
            }
            throw std::logic_error("Thresholds invalid. No threshold found for mhc " + std::to_string(hash.mhc));
        }

        bool construct(size_t M_, size_t seed_) {
            M = M_;
            seed = seed_;
            cells.clear();
            cells.resize(M, nullptr);
            for (size_t i = 0; i < numEntries; i++) {
                if (!insert(&heap[i])) {
                    return false;
                }
            }
            return true;
        }

        size_t size() {
            return numEntries;
        }
    private:
        bool insert(TableEntry *entry) {
            size_t tries = 0;
            while (tries < 10000) {
                size_t cell = entry->hash.hash(entry->hashFunctionIndex + seed, M);
                std::swap(entry, cells[cell]);
                if (entry == nullptr) {
                    return true;
                }
                entry->hashFunctionIndex = (entry->hashFunctionIndex + 1) % entry->numHashFunctions;
                tries++;
            }
            return false;
        }
};
