#pragma once
#include <vector>
#include <cassert>
#include <queue>
#include <Function.h>
#include <MurmurHash64.h>
#include <cstring>
#include "IrregularCuckooHashTable.h"

namespace sichash {
/**
 * Cuckoo hash table with M=N. Construction needs multiple tries before succeeding.
 */
class MinimalPerfectCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            size_t candidateCellsXor = 0;
        };
        TableEntry *heap;
        TableEntry** cells;
        size_t N;
    private:
        size_t seed = 0;
        size_t numEntries = 0;
    public:
        explicit MinimalPerfectCuckooHashTable(size_t N) : N(N) {
            heap = new TableEntry[N];
            cells = new TableEntry*[N];
        }

        ~MinimalPerfectCuckooHashTable() {
            delete[] heap;
            delete[] cells;
        }

        void prepare(HashedKey hash) {
            assert(numEntries < N);
            heap[numEntries].hash = hash;
            numEntries++;
        }

        bool construct(size_t seed_) {
            seed = seed_;
            memset(cells, 0, N * sizeof(void*)); // Fill with nullpointers
            for (size_t i = 0; i < numEntries; i++) {
                if (!insert(&heap[i])) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] size_t size() const {
            return numEntries;
        }
    private:
        bool insert(TableEntry *entry) {
            size_t cell1 = entry->hash.hash(seed, N);
            size_t cell2 = entry->hash.hash(seed + 1, N);
            entry->candidateCellsXor = cell1 ^ cell2;
            size_t currentCell = cell2;

            size_t tries = 0;
            while (tries < 2 * N) {
                size_t alternativeCell = entry->candidateCellsXor ^ currentCell;
                std::swap(entry, cells[alternativeCell]);
                if (entry == nullptr) {
                    return true;
                }
                currentCell = alternativeCell;
                tries++;
            }
            return false;
        }
};
} // Namespace sichash
