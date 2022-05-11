#pragma once
#include <vector>
#include "Hash.h"

struct HashedKey {
    uint64_t mhc;

    HashedKey() {
        this->mhc = 0;
    }

    explicit HashedKey(const std::string &element, uint32_t seed = 0) {
        mhc = Hash::hash(element, seed, UINT64_MAX);
    }

    [[nodiscard]] inline uint64_t hash(int hashFunctionIndex, size_t range) const {
        return fastrange64(sux::function::remix(mhc + hashFunctionIndex), range);
    }
};


class RandomWalkCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t hashFunctionMask = 0;
        };
        std::vector<TableEntry*> cells;
        size_t N = 0;
        size_t M = 0;
        size_t numEntries = 0;
        size_t maxEntries = 0;
        TableEntry *heap;
        size_t seed = 0;
        const uint64_t threshold1;
        const uint64_t threshold2;

        explicit RandomWalkCuckooHashTable(size_t maxEntries, uint64_t threshold1_, uint64_t threshold2_)
                : maxEntries(maxEntries), threshold1(threshold1_), threshold2(threshold2_) {
            heap = new TableEntry[maxEntries];
        }

        ~RandomWalkCuckooHashTable() {
            delete heap;
        }

        static std::string name() {
            return "RandomWalkCuckooHashTable";
        }

        void prepare(HashedKey hash) {
            assert(numEntries < maxEntries);
            heap[numEntries].hash = hash;
            if (hash.mhc <= threshold1) {
                heap[numEntries].hashFunctionMask = 0b001;
            } else if (hash.mhc <= threshold2) {
                heap[numEntries].hashFunctionMask = 0b011;
            } else {
                heap[numEntries].hashFunctionMask = 0b111;
            }
            numEntries++;
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
    private:
        bool insert(TableEntry *entry) {
            size_t tries = 0;
            while (tries < 10000) {
                size_t cell = entry->hash.hash(entry->hashFunctionIndex + seed, M);
                std::swap(entry, cells[cell]);
                if (entry == nullptr) {
                    return true;
                }
                entry->hashFunctionIndex = (entry->hashFunctionIndex + 1) & entry->hashFunctionMask;
                tries++;
            }
            return false;
        }
};

using HeterogeneousCuckooHashTable = RandomWalkCuckooHashTable;
