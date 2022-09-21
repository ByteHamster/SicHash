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
            uint32_t candidateCellsXor = 0;
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
        typedef union {
            struct {
                uint32_t low;
                uint32_t high;
            } halves;
            uint64_t full;
        } Union64;

        bool insert(TableEntry *entry) {
            Union64 hash;
            hash.full = util::remix(entry->hash.mhc + seed);
            uint32_t cell1 = util::fastrange32(hash.halves.high, M);
            uint32_t cell2 = util::fastrange32(hash.halves.low, M);
            entry->candidateCellsXor = cell1 ^ cell2;
            if (cells[cell1] == nullptr) {
                cells[cell1] = entry;
                return true;
            }
            if (cells[cell2] == nullptr) {
                cells[cell2] = entry;
                return true;
            }
            uint32_t currentCell = cell2;

            size_t tries = 0;
            while (tries < M) {
                uint32_t alternativeCell = entry->candidateCellsXor ^ currentCell;
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
