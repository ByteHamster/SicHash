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


class RandomWalkCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t hashFunctionMask = 0;
        };
        TableEntry *heap;
    private:
        std::vector<TableEntry*> cells;
        size_t M = 0;
        size_t numEntries = 0;
        size_t maxEntries = 0;
        size_t seed = 0;
        const uint64_t threshold1;
        const uint64_t threshold2;
    public:

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

class HopcroftKarpMatchingCuckooHashTable {
    public:
        struct TableEntry {
            HashedKey hash;
            uint8_t hashFunctionIndex = 0;
            uint8_t hashFunctionMask = 0;
        };
        size_t M = 0;
        size_t numEntries = 0;
        size_t maxEntries = 0;
        TableEntry *heap;
        size_t seed = 0;
        const uint64_t threshold1;
        const uint64_t threshold2;

        explicit HopcroftKarpMatchingCuckooHashTable(size_t maxEntries, uint64_t threshold1_, uint64_t threshold2_)
                : maxEntries(maxEntries), threshold1(threshold1_), threshold2(threshold2_) {
            heap = new TableEntry[maxEntries];
        }

        ~HopcroftKarpMatchingCuckooHashTable() {
            delete heap;
        }

        static std::string name() {
            return "HopcroftKarpMatchingCuckooHashTable";
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

            n_left = numEntries;
            n_right = M;
            g.clear();
            g.resize(n_left);
            match_from_left.clear();
            match_from_left.resize(n_left, -1);
            match_from_right.clear();
            match_from_right.resize(n_right, -1);
            dist.clear();
            dist.resize(n_left);

            for (size_t i = 0; i < numEntries; i++) {
                for (size_t h = 0; h <= heap[i].hashFunctionMask; h++) {
                     add(i, heap[i].hash.hash(h + seed, M));
                }
            }
            size_t matchingSize = get_max_matching();
            if (matchingSize != numEntries) {
                std::cout<<"Matching size: "<<matchingSize<<", N="<<numEntries<<std::endl;
                return false;
            }

            for (int i = 0; i < numEntries; i++) {
                for (size_t h = 0; h <= heap[i].hashFunctionMask; h++) {
                    if (heap[i].hash.hash(h + seed, M) == match_from_left[i]) {
                        heap[i].hashFunctionIndex = h;
                        break;
                    }
                }
            }
            return true;
        }
    private:
        // https://judge.yosupo.jp/submission/52112
        int n_left, n_right, flow = 0;
        std::vector<std::vector<int>> g;
        std::vector<int> match_from_left, match_from_right;
        std::vector<int> dist;

        void add(int u, int v) {
            g[u].push_back(v);
        }

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
                for (size_t i = 0; i < g[u].size(); i++) {
                    int v = g[u][i];
                    if (~match_from_right[v] && !~dist[match_from_right[v]]) {
                        dist[match_from_right[v]] = dist[u] + 1;
                        q.push(match_from_right[v]);
                    }
                }
            }
        }

        bool dfs(int u) {
            for (size_t i = 0; i < g[u].size(); i++) {
                int v = g[u][i];
                if (!~match_from_right[v]) {
                    match_from_left[u] = v;
                    match_from_right[v] = u;
                    return true;
                }
            }
            for (size_t i = 0; i < g[u].size(); i++) {
                int v = g[u][i];
                if (dist[match_from_right[v]] == dist[u] + 1 && dfs(match_from_right[v])) {
                    match_from_left[u] = v;
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
