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
 * Tiny binary cuckoo hash table. Construction needs multiple tries before succeeding.
 */
class TinyBinaryCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            size_t candidateCellsXor = 0;
        };
        TableEntry *heap;
        TableEntry** cells;
        size_t N;
        size_t M;
    private:
        size_t seed = 0;
        size_t numEntries = 0;
    public:
        explicit TinyBinaryCuckooHashTable(size_t N, size_t M) : N(N), M(M) {
            heap = new TableEntry[N];
            cells = new TableEntry*[M];
        }

        ~TinyBinaryCuckooHashTable() {
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
            memset(cells, 0, M * sizeof(void*)); // Fill with nullpointers
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
            size_t cell1 = entry->hash.hash(seed, M);
            size_t cell2 = entry->hash.hash(seed + 1, M);
            entry->candidateCellsXor = cell1 ^ cell2;
            size_t currentCell = cell2;

            size_t tries = 0;
            while (tries < 2 * M) {
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
