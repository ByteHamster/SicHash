#pragma once
#include <vector>
#include <queue>
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

struct HeterogeneousCuckooConfig {
    uint64_t threshold1 = UINT64_MAX / 100 * 50; // 50%
    uint64_t threshold2 = UINT64_MAX / 100 * 20; // 25%
    size_t maxEntries = 0;
};

//#define PRECALCULATE_HASHES

class RandomWalkCuckooHashTable {
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
    private:
        std::vector<TableEntry*> cells;
        size_t M = 0;
        size_t numEntries = 0;
        size_t seed = 0;
        const HeterogeneousCuckooConfig config;
    public:
        explicit RandomWalkCuckooHashTable(HeterogeneousCuckooConfig config_)
                : config(config_) {
            heap = new TableEntry[config.maxEntries];
        }

        ~RandomWalkCuckooHashTable() {
            delete heap;
        }

        static std::string name() {
            #ifdef PRECALCULATE_HASHES
                return "RandomWalkCuckooHashTablePre";
            #else
                return "RandomWalkCuckooHashTable";
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

        size_t size() {
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

class HopcroftKarpMatchingCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t hashFunctionMask = 0;
            #ifdef PRECALCULATE_HASHES
                size_t hashes[8];
            #endif
        };
        size_t M = 0;
        size_t numEntries = 0;
        TableEntry *heap;
        size_t seed = 0;
        const HeterogeneousCuckooConfig config;

        explicit HopcroftKarpMatchingCuckooHashTable(HeterogeneousCuckooConfig config_)
                : config(config_) {
            heap = new TableEntry[config.maxEntries];
        }

        ~HopcroftKarpMatchingCuckooHashTable() {
            delete heap;
        }

        static std::string name() {
            #ifdef PRECALCULATE_HASHES
                return "HopcroftKarpMatchingCuckooHashTablePre";
            #else
                return "HopcroftKarpMatchingCuckooHashTable";
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

            n_left = numEntries;
            n_right = M;
            match_from_left.clear();
            match_from_left.resize(n_left, -1);
            match_from_right.clear();
            match_from_right.resize(n_right, -1);
            dist.clear();
            dist.resize(n_left);
            #ifdef PRECALCULATE_HASHES
                for (size_t i = 0; i < numEntries; i++) {
                    for (size_t h = 0; h <= heap[i].hashFunctionMask; h++) {
                        heap[i].hashes[h] = heap[i].hash.hash(h + seed, M);
                    }
                }
            #endif

            size_t matchingSize = get_max_matching();
            if (matchingSize != numEntries) {
                return false;
            }
            return true;
        }
    private:
        // https://judge.yosupo.jp/submission/52112
        int n_left = 0, n_right = 0, flow = 0;
        std::vector<int> match_from_left, match_from_right;
        std::vector<int> dist;

        void bfs() {
            std::queue<int> q;
            for (int u = 0; u < n_left; ++u) {
                if (!~match_from_left[u]) {
                    q.push(u);
                    dist[u] = 0;
                } else {
                    dist[u] = -1;
                }
            }
            while (!q.empty()) {
                int u = q.front();
                q.pop();
                for (size_t i = 0; i <= heap[u].hashFunctionMask; i++) {
                    #ifdef PRECALCULATE_HASHES
                        int v = heap[u].hashes[i];
                    #else
                        int v = heap[u].hash.hash(i + seed, M);
                    #endif
                    if (~match_from_right[v] && !~dist[match_from_right[v]]) {
                        dist[match_from_right[v]] = dist[u] + 1;
                        q.push(match_from_right[v]);
                    }
                }
            }
        }

        bool dfs(int u) {
            for (size_t i = 0; i <= heap[u].hashFunctionMask; i++) {
                #ifdef PRECALCULATE_HASHES
                    int v = heap[u].hashes[i];
                #else
                    int v = heap[u].hash.hash(i + seed, M);
                #endif
                if (!~match_from_right[v]) {
                    match_from_left[u] = v;
                    heap[u].hashFunctionIndex = i;
                    match_from_right[v] = u;
                    return true;
                }
            }
            for (size_t i = 0; i <= heap[u].hashFunctionMask; i++) {
                #ifdef PRECALCULATE_HASHES
                    int v = heap[u].hashes[i];
                #else
                    int v = heap[u].hash.hash(i + seed, M);
                #endif
                if (dist[match_from_right[v]] == dist[u] + 1 && dfs(match_from_right[v])) {
                    match_from_left[u] = v;
                    heap[u].hashFunctionIndex = i;
                    match_from_right[v] = u;
                    return true;
                }
            }
            return false;
        }

        int get_max_matching() {
            flow = 0;
            while (true) {
                bfs();
                int augment = 0;
                for (int u = 0; u < n_left; ++u) {
                    if (!~match_from_left[u]) {
                        augment += dfs(u);
                    }
                }
                if (!augment) {
                    break;
                }
                flow += augment;
            }
            return flow;
        }
};

using HeterogeneousCuckooHashTable = RandomWalkCuckooHashTable;
