#pragma once
#include <stdexcept>
#include "IrregularCuckooHashTable.h"

namespace sichash {
/**
 * This cuckoo hash table is used for certain types of benchmarks that do NOT measure timing.
 * The implementation is more flexible than IrregularCuckooHashTable
 * (as it supports a more flexible number of hash functions)
 * but is not optimized for performance.
 * It also supports incremental insertion.
 */
class SlowIrregularCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t numHashFunctions = 0;
        };
        TableEntry *heap;
        size_t displacements = 0;
    private:
        std::vector<TableEntry*> cells;
        size_t M = 0;
        size_t numEntries = 0;
        std::vector<std::pair<uint64_t, uint8_t>> thresholds;
    public:
        explicit SlowIrregularCuckooHashTable(size_t M, std::vector<std::pair<uint64_t, uint8_t>> &thresholds_, size_t maxEntries)
                : M(M), thresholds(thresholds_) {
            heap = new TableEntry[maxEntries];
            cells.resize(M, nullptr);
            displacements = 0;
        }

        ~SlowIrregularCuckooHashTable() {
            delete heap;
        }

        static std::string name() {
            return "SlowIrregularCuckooHashTable";
        }

        bool insert(HashedKey hash) {
            heap[numEntries].hash = hash;
            heap[numEntries].numHashFunctions = 0;
            for (auto [thresh, num] : thresholds) {
                if (hash.mhc <= thresh) {
                    heap[numEntries].numHashFunctions = num;
                    break;
                }
            }
            if (heap[numEntries].numHashFunctions == 0) {
                throw std::logic_error("Thresholds invalid. No threshold found for mhc " + std::to_string(hash.mhc));
            }
            numEntries++;
            return insert(&heap[numEntries - 1]);
        }

        [[nodiscard]] size_t size() const {
            return numEntries;
        }
    private:
        bool insert(TableEntry *entry) {
            size_t tries = 0;
            while (tries < 10 * M) {
                size_t cell = entry->hash.hash(entry->hashFunctionIndex, M);
                std::swap(entry, cells[cell]);
                if (entry == nullptr) {
                    return true;
                }
                entry->hashFunctionIndex = (entry->hashFunctionIndex + 1) % entry->numHashFunctions;
                tries++;
                displacements++;
            }
            return false;
        }
};
} // Namespace sichash
