#pragma once
#include <vector>
#include <cassert>
#include <queue>
#include <Function.h>
#include <MurmurHash64.h>

namespace sichash {
struct HashedKey {
    uint64_t mhc;

    HashedKey() {
        this->mhc = 0;
    }

    explicit HashedKey(const std::string &element, uint32_t seed = 0) {
        uint64_t stringHash = util::MurmurHash64(element.data(), element.length());
        uint64_t modified = stringHash + seed;
        mhc = util::MurmurHash64(&modified, sizeof(uint64_t));
    }

    [[nodiscard]] inline uint64_t hash(int hashFunctionIndex, size_t range) const {
        return util::fastrange64(util::remix(mhc + hashFunctionIndex), range);
    }
};

struct IrregularCuckooHashTableConfig {
    uint64_t threshold1 = UINT64_MAX / 100 * 50; // 50%
    uint64_t threshold2 = UINT64_MAX / 100 * 20; // 25%
    size_t maxEntries = 0;
};

//#define PRECALCULATE_HASHES

class IrregularCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t hashFunctionMask = 0;
            #ifdef PRECALCULATE_HASHES
                size_t hashes[8];
            #endif
        };
        TableEntry *heap;
        std::vector<TableEntry*> cells;
        size_t M = 0;
    private:
        size_t numEntries = 0;
        size_t seed = 0;
        const IrregularCuckooHashTableConfig config;
    public:
        explicit IrregularCuckooHashTable(IrregularCuckooHashTableConfig config_)
                : config(config_) {
            heap = new TableEntry[config.maxEntries];
        }

        ~IrregularCuckooHashTable() {
            delete[] heap;
        }

        static std::string name() {
            #ifdef PRECALCULATE_HASHES
                return "IrregularCuckooHashTablePre";
            #else
                return "IrregularCuckooHashTable";
            #endif
        }

        void prepare(HashedKey hash) {
            assert(numEntries < config.maxEntries);
            heap[numEntries].hash = hash;
            if (hash.mhc <= config.threshold1) {
                heap[numEntries].hashFunctionMask = 0b001;
            } else if (hash.mhc <= config.threshold2) {
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
            #ifdef PRECALCULATE_HASHES
                for (size_t i = 0; i < numEntries; i++) {
                    for (size_t h = 0; h <= heap[i].hashFunctionMask; h++) {
                        heap[i].hashes[h] = heap[i].hash.hash(h + seed, M);
                    }
                }
            #endif
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
            size_t tries = 0;
            while (tries < 10000) {
                #ifdef PRECALCULATE_HASHES
                    size_t cell = entry->hashes[entry->hashFunctionIndex];
                #else
                    size_t cell = entry->hash.hash(entry->hashFunctionIndex + seed, M);
                #endif
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
} // Namespace sichash
