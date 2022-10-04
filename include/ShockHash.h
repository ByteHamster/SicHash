#pragma once
/*
 * Based on RecSplit, Copyright (C) 2019-2020 Emmanuel Esposito and Sebastiano Vigna
 * Enhanced to use overloaded cuckoo hash tables in the leaves.
 * For tiny space usages (~1.6 bit/object), ShockHash is faster than RecSplit.
 */

// Work around a missing "#pragma once" in RecSplit
#ifndef ALLOW_UNALIGNED_READS
#include "sux/support/SpookyV2.hpp"
#endif
#include "sux/util/Vector.hpp"
#include "sux/function/DoubleEF.hpp"
#include "sux/function/RiceBitVector.hpp"
#include "sux/function/RecSplit.hpp"
#include "SimpleRibbon.h"
#include "TinyBinaryCuckooHashTable.h"
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

namespace shockhash {
using namespace sux;
using namespace sux::function;
using namespace std;
using namespace std::chrono;

// Assumed *maximum* size of a bucket. Works with high probability up to average bucket size ~2000.
static const int MAX_BUCKET_SIZE = 3000;
static const int MAX_FANOUT = 32;
static const int MAX_LEAF_SIZE = 30;

#if defined(STATS)
static uint64_t bij_unary, bij_fixed;
static uint64_t split_unary, split_fixed;
static uint64_t time_bij;
static uint64_t time_split[MAX_LEVEL_TIME];
#endif

// Starting seed at given distance from the root (extracted at random).
static const uint64_t start_seed[] = {0x106393c187cae21a, 0x6453cec3f7376937, 0x643e521ddbd2be98, 0x3740c6412f6572cb, 0x717d47562f1ce470, 0x4cd6eb4c63befb7c, 0x9bfd8c5e18c8da73,
                                      0x082f20e10092a9a3, 0x2ada2ce68d21defc, 0xe33cb4f3e7c6466b, 0x3980be458c509c59, 0xc466fd9584828e8c, 0x45f0aabe1a61ede6, 0xf6e7b8b33ad9b98d,
                                      0x4ef95e25f4b4983d, 0x81175195173b92d3, 0x4e50927d8dd15978, 0x1ea2099d1fafae7f, 0x425c8a06fbaaa815, 0xcd4216006c74052a};

// Optimal Golomb-Rice parameters for leaves.
static constexpr uint8_t bij_memo[MAX_LEAF_SIZE + 1] = {
        0, 0, 0, 0, 0, 0, 1, 1, 2, 2,
        2, 3, 3, 4, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 9, 10, 10, 10, 11};

template <size_t LEAF_SIZE> class SplittingStrategy {
    public:
        static constexpr size_t _leaf = LEAF_SIZE;
        static_assert(_leaf >= 1);
        static_assert(_leaf <= MAX_LEAF_SIZE);
        //static constexpr size_t lower_aggr = _leaf * max(2, ceil(0.35 * _leaf + 1. / 2));
        //static constexpr size_t upper_aggr = lower_aggr * (_leaf < 7 ? 2 : ceil(0.21 * _leaf + 9. / 10));
        static constexpr size_t upper_aggr = _leaf * 10;
        static constexpr size_t lower_aggr = _leaf * 4;
};

// Generates the precomputed table of 32-bit values holding the Golomb-Rice code
// of a splitting (upper 5 bits), the number of nodes in the associated subtree
// (following 11 bits) and the sum of the Golomb-Rice codelengths in the same
// subtree (lower 16 bits).

template <size_t LEAF_SIZE> static constexpr void _fill_golomb_rice(const int m, array<uint32_t, MAX_BUCKET_SIZE> *memo) {
    array<int, MAX_FANOUT> k{0};

    constexpr size_t lower_aggr = SplittingStrategy<LEAF_SIZE>::lower_aggr;
    constexpr size_t upper_aggr = SplittingStrategy<LEAF_SIZE>::upper_aggr;

    size_t fanout = 0, unit = 0;
    if (m > upper_aggr) { // High-level aggregation (fanout 2)
        unit = upper_aggr * (uint16_t(m / 2 + upper_aggr - 1) / upper_aggr);
        fanout = 2;
    } else if (m > lower_aggr) { // Second-level aggregation
        unit = lower_aggr;
        fanout = uint16_t(m + lower_aggr - 1) / lower_aggr;
    } else { // First-level aggregation
        unit = LEAF_SIZE;
        fanout = uint16_t(m + LEAF_SIZE - 1) / LEAF_SIZE;
    }

    k[fanout - 1] = m;
    for (size_t i = 0; i < fanout - 1; ++i) {
        k[i] = unit;
        k[fanout - 1] -= k[i];
    }

    double sqrt_prod = 1;
    for (size_t i = 0; i < fanout; ++i) sqrt_prod *= sqrt(k[i]);

    const double p = sqrt(m) / (pow(2 * M_PI, (fanout - 1.) / 2) * sqrt_prod);
    auto golomb_rice_length = (uint32_t)ceil(log2(-log((sqrt(5) + 1) / 2) / log1p(-p))); // log2 Golomb modulus

    assert(golomb_rice_length <= 0x1F); // Golomb-Rice code, stored in the 5 upper bits
    (*memo)[m] = golomb_rice_length << 27;
    for (size_t i = 0; i < fanout; ++i) golomb_rice_length += (*memo)[k[i]] & 0xFFFF;
    assert(golomb_rice_length <= 0xFFFF); // Sum of Golomb-Rice codeslengths in the subtree, stored in the lower 16 bits
    (*memo)[m] |= golomb_rice_length;

    uint32_t nodes = 1;
    for (size_t i = 0; i < fanout; ++i) nodes += ((*memo)[k[i]] >> 16) & 0x7FF;
    assert(LEAF_SIZE < 3 || nodes <= 0x7FF); // Number of nodes in the subtree, stored in the middle 11 bits
    (*memo)[m] |= nodes << 16;
}

template <size_t LEAF_SIZE> static constexpr array<uint32_t, MAX_BUCKET_SIZE> fill_golomb_rice() {
    array<uint32_t, MAX_BUCKET_SIZE> memo{0};
    size_t s = 0;
    for (; s <= LEAF_SIZE; ++s) memo[s] = bij_memo[s] << 27 | (s > 1) << 16 | bij_memo[s];
    for (; s < MAX_BUCKET_SIZE; ++s) _fill_golomb_rice<LEAF_SIZE>(s, &memo);
    return memo;
}

#define first_hash(k, len) spooky(k, len, 0)
#define golomb_param(m) (memo[m] >> 27)
#define skip_bits(m) (memo[m] & 0xFFFF)
#define skip_nodes(m) ((memo[m] >> 16) & 0x7FF)

template <size_t LEAF_SIZE, sux::util::AllocType AT = sux::util::AllocType::MALLOC> class ShockHash {
        static_assert(LEAF_SIZE <= MAX_LEAF_SIZE);
        static constexpr size_t _leaf = LEAF_SIZE;
        static constexpr size_t lower_aggr = SplittingStrategy<LEAF_SIZE>::lower_aggr;
        static constexpr size_t upper_aggr = SplittingStrategy<LEAF_SIZE>::upper_aggr;

        // For each bucket size, the Golomb-Rice parameter (upper 8 bits) and the number of bits to
        // skip in the fixed part of the tree (lower 24 bits).
        static constexpr array<uint32_t, MAX_BUCKET_SIZE> memo = fill_golomb_rice<LEAF_SIZE>();

        size_t bucket_size;
        size_t nbuckets;
        size_t keys_count;
        RiceBitVector<AT> descriptors;
        DoubleEF<AT> ef;
        sichash::SimpleRibbon<1> *ribbon = nullptr;
        std::vector<std::pair<uint64_t, uint8_t>> ribbonInput;

    public:
        ShockHash() {}


        ShockHash(const vector<string> &keys, const size_t bucket_size) {
            this->bucket_size = bucket_size;
            this->keys_count = keys.size();
            hash128_t *h = (hash128_t *)malloc(this->keys_count * sizeof(hash128_t));
            for (size_t i = 0; i < this->keys_count; ++i) {
                h[i] = first_hash(keys[i].c_str(), keys[i].size());
            }
            hash_gen(h);
            free(h);
        }

        /** Returns the value associated with the given 128-bit hash.
         *
         * Note that this method is mainly useful for benchmarking.
         * @param hash a 128-bit hash.
         * @return the associated value.
         */
        size_t operator()(const hash128_t &hash) {
            const size_t bucket = hash128_to_bucket(hash);
            uint64_t cum_keys, cum_keys_next, bit_pos;
            ef.get(bucket, cum_keys, cum_keys_next, bit_pos);

            // Number of keys in this bucket
            size_t m = cum_keys_next - cum_keys;
            auto reader = descriptors.reader();
            reader.readReset(bit_pos, skip_bits(m));
            int level = 0;

            while (m > upper_aggr) { // fanout = 2
                const auto d = reader.readNext(golomb_param(m));
                const size_t hmod = remap16(remix(hash.second + d + start_seed[level]), m);

                const uint32_t split = ((uint16_t(m / 2 + upper_aggr - 1) / upper_aggr)) * upper_aggr;
                if (hmod < split) {
                    m = split;
                } else {
                    reader.skipSubtree(skip_nodes(split), skip_bits(split));
                    m -= split;
                    cum_keys += split;
                }
                level++;
            }
            if (m > lower_aggr) {
                const auto d = reader.readNext(golomb_param(m));
                const size_t hmod = remap16(remix(hash.second + d + start_seed[level]), m);

                const int part = uint16_t(hmod) / lower_aggr;
                m = min(lower_aggr, m - part * lower_aggr);
                cum_keys += lower_aggr * part;
                if (part) reader.skipSubtree(skip_nodes(lower_aggr) * part, skip_bits(lower_aggr) * part);
                level++;
            }

            if (m > _leaf) {
                const auto d = reader.readNext(golomb_param(m));
                const size_t hmod = remap16(remix(hash.second + d + start_seed[level]), m);

                const int part = uint16_t(hmod) / _leaf;
                m = min(_leaf, m - part * _leaf);
                cum_keys += _leaf * part;
                if (part) reader.skipSubtree(part, skip_bits(_leaf) * part);
                level++;
            }

            const auto b = reader.readNext(golomb_param(m));

            // Begin: difference to RecSplit.
            sichash::HashedKey key(hash.second);
            size_t hashFunctionIndex = ribbon->retrieve(hash.second);
            return cum_keys + sichash::TinyBinaryCuckooHashTable::hashToCell(key, b + start_seed[level], m, hashFunctionIndex);
            // End: difference to RecSplit.
        }

        /** Returns the value associated with the given key.
         *
         * @param key a key.
         * @return the associated value.
         */
        size_t operator()(const string &key) { return operator()(first_hash(key.c_str(), key.size())); }

        /** Returns the number of keys used to build this RecSplit instance. */
        inline size_t size() { return this->keys_count; }

        /** Returns an estimate of the size in bits of this structure. */
        size_t getBits() {
            return ef.bitCountCumKeys() + ef.bitCountPosition()
                    + descriptors.getBits() + 8 * ribbon->size() + 8 * sizeof(ShockHash);
        }

    private:
        // Maps a 128-bit to a bucket using the first 64-bit half.
        inline uint64_t hash128_to_bucket(const hash128_t &hash) const { return remap128(hash.first, nbuckets); }

        // Computes and stores the splittings and bijections of a bucket.
        void recSplit(vector<uint64_t> &bucket, typename RiceBitVector<AT>::Builder &builder, vector<uint32_t> &unary) {
            const auto m = bucket.size();
            vector<uint64_t> temp(m);
            recSplit(bucket, temp, 0, bucket.size(), builder, unary, 0);
        }

        void recSplit(vector<uint64_t> &bucket, vector<uint64_t> &temp, size_t start, size_t end, typename RiceBitVector<AT>::Builder &builder, vector<uint32_t> &unary, const int level) {
            const auto m = end - start;
            assert(m > 1);
            uint64_t x = start_seed[level];

            if (m <= _leaf) {
#ifdef STATS
                sum_depths += m * level;
                auto start_time = high_resolution_clock::now();
#endif
                // Begin: difference to RecSplit.
                sichash::TinyBinaryCuckooHashTable table(m, m);
                for (size_t i = start; i < end; i++) {
                    table.prepare(sichash::HashedKey(bucket[i]));
                }
                for (;;) {
                    if (table.construct(x)) break;
                    x++;
                }
                for (size_t i = 0; i < m; i++) {
                    size_t cell1 = sichash::TinyBinaryCuckooHashTable::hashToCell(table.cells[i]->hash, x, m, 0);
                    ribbonInput.emplace_back(table.cells[i]->hash.mhc, i == cell1 ? 0 : 1);
                }
                // End: difference to RecSplit.

                x -= start_seed[level];
                const auto log2golomb = golomb_param(m);
                builder.appendFixed(x, log2golomb);
                unary.push_back(x >> log2golomb);

#ifdef STATS
                bij_unary += 1 + (x >> log2golomb);
                bij_fixed += log2golomb;
                time_bij += duration_cast<nanoseconds>(high_resolution_clock::now() - start_time).count();
#endif
            } else {
#ifdef STATS
                auto start_time = high_resolution_clock::now();
#endif
                if (m > upper_aggr) { // fanout = 2
                    const size_t split = ((uint16_t(m / 2 + upper_aggr - 1) / upper_aggr)) * upper_aggr;

                    size_t count[2];
                    for (;;) {
                        count[0] = 0;
                        for (size_t i = start; i < end; i++) {
                            count[remap16(remix(bucket[i] + x), m) >= split]++;
                        }
                        if (count[0] == split) break;
                        x++;
                    }

                    count[0] = 0;
                    count[1] = split;
                    for (size_t i = start; i < end; i++) {
                        temp[count[remap16(remix(bucket[i] + x), m) >= split]++] = bucket[i];
                    }
                    copy(&temp[0], &temp[m], &bucket[start]);
                    x -= start_seed[level];
                    const auto log2golomb = golomb_param(m);
                    builder.appendFixed(x, log2golomb);
                    unary.push_back(x >> log2golomb);

#ifdef STATS
                    time_split[min(MAX_LEVEL_TIME, level)] += duration_cast<nanoseconds>(high_resolution_clock::now() - start_time).count();
#endif
                    recSplit(bucket, temp, start, start + split, builder, unary, level + 1);
                    if (m - split > 1) recSplit(bucket, temp, start + split, end, builder, unary, level + 1);
                } else if (m > lower_aggr) { // 2nd aggregation level
                    const size_t fanout = uint16_t(m + lower_aggr - 1) / lower_aggr;
                    size_t count[fanout]; // Note that we never read count[fanout-1]
                    for (;;) {
                        memset(count, 0, sizeof count - sizeof *count);
                        for (size_t i = start; i < end; i++) {
                            count[uint16_t(remap16(remix(bucket[i] + x), m)) / lower_aggr]++;
                        }
                        size_t broken = 0;
                        for (size_t i = 0; i < fanout - 1; i++) broken |= count[i] - lower_aggr;
                        if (!broken) break;
                        x++;
                    }

                    for (size_t i = 0, c = 0; i < fanout; i++, c += lower_aggr) count[i] = c;
                    for (size_t i = start; i < end; i++) {
                        temp[count[uint16_t(remap16(remix(bucket[i] + x), m)) / lower_aggr]++] = bucket[i];
                    }
                    copy(&temp[0], &temp[m], &bucket[start]);

                    x -= start_seed[level];
                    const auto log2golomb = golomb_param(m);
                    builder.appendFixed(x, log2golomb);
                    unary.push_back(x >> log2golomb);

#ifdef STATS
                    time_split[min(MAX_LEVEL_TIME, level)] += duration_cast<nanoseconds>(high_resolution_clock::now() - start_time).count();
#endif
                    size_t i;
                    for (i = 0; i < m - lower_aggr; i += lower_aggr) {
                        recSplit(bucket, temp, start + i, start + i + lower_aggr, builder, unary, level + 1);
                    }
                    if (m - i > 1) recSplit(bucket, temp, start + i, end, builder, unary, level + 1);
                } else { // First aggregation level, m <= lower_aggr
                    const size_t fanout = uint16_t(m + _leaf - 1) / _leaf;
                    size_t count[fanout]; // Note that we never read count[fanout-1]
                    for (;;) {
                        memset(count, 0, sizeof count - sizeof *count);
                        for (size_t i = start; i < end; i++) {
                            count[uint16_t(remap16(remix(bucket[i] + x), m)) / _leaf]++;
                        }
                        size_t broken = 0;
                        for (size_t i = 0; i < fanout - 1; i++) broken |= count[i] - _leaf;
                        if (!broken) break;
                        x++;
                    }
                    for (size_t i = 0, c = 0; i < fanout; i++, c += _leaf) count[i] = c;
                    for (size_t i = start; i < end; i++) {
                        temp[count[uint16_t(remap16(remix(bucket[i] + x), m)) / _leaf]++] = bucket[i];
                    }
                    copy(&temp[0], &temp[m], &bucket[start]);

                    x -= start_seed[level];
                    const auto log2golomb = golomb_param(m);
                    builder.appendFixed(x, log2golomb);
                    unary.push_back(x >> log2golomb);

#ifdef STATS
                    time_split[min(MAX_LEVEL_TIME, level)] += duration_cast<nanoseconds>(high_resolution_clock::now() - start_time).count();
#endif
                    size_t i;
                    for (i = 0; i < m - _leaf; i += _leaf) {
                        recSplit(bucket, temp, start + i, start + i + _leaf, builder, unary, level + 1);
                    }
                    if (m - i > 1) recSplit(bucket, temp, start + i, end, builder, unary, level + 1);
                }
#ifdef STATS
                const auto log2golomb = golomb_param(m);
                split_unary += 1 + (x >> log2golomb);
                split_fixed += log2golomb;
#endif
            }
        }

        void hash_gen(hash128_t *hashes) {
#ifdef STATS
            split_unary = split_fixed = 0;
            bij_unary = bij_fixed = 0;
            time_bij = 0;
            memset(time_split, 0, sizeof time_split);
#endif

#ifndef __SIZEOF_INT128__
            if (keys_count > (1ULL << 32)) {
			fprintf(stderr, "For more than 2^32 keys, you need 128-bit integer support.\n");
			abort();
		}
#endif
            nbuckets = max(1, (keys_count + bucket_size - 1) / bucket_size);
            auto bucket_size_acc = vector<int64_t>(nbuckets + 1);
            auto bucket_pos_acc = vector<int64_t>(nbuckets + 1);

            sort(hashes, hashes + keys_count, [this](const hash128_t &a, const hash128_t &b) { return hash128_to_bucket(a) < hash128_to_bucket(b); });
            typename RiceBitVector<AT>::Builder builder;

            bucket_size_acc[0] = bucket_pos_acc[0] = 0;
            for (size_t i = 0, last = 0; i < nbuckets; i++) {
                vector<uint64_t> bucket;
                for (; last < keys_count && hash128_to_bucket(hashes[last]) == i; last++) bucket.push_back(hashes[last].second);

                const size_t s = bucket.size();
                bucket_size_acc[i + 1] = bucket_size_acc[i] + s;
                if (bucket.size() > 1) {
                    vector<uint32_t> unary;
                    recSplit(bucket, builder, unary);
                    builder.appendUnaryAll(unary);
                }
                bucket_pos_acc[i + 1] = builder.getBits();
            }
            builder.appendFixed(1, 1); // Sentinel (avoids checking for parts of size 1)
            descriptors = builder.build();
            ef = DoubleEF<AT>(vector<uint64_t>(bucket_size_acc.begin(), bucket_size_acc.end()), vector<uint64_t>(bucket_pos_acc.begin(), bucket_pos_acc.end()));

            // Begin: difference to RecSplit.
            ribbon = new sichash::SimpleRibbon<1>(ribbonInput);
            ribbonInput.clear();
            // End: difference to RecSplit.

#ifdef STATS
            // Evaluation purposes only
            double ef_sizes = (double)ef.bitCountCumKeys() / keys_count;
            double ef_bits = (double)ef.bitCountPosition() / keys_count;
            double rice_desc = (double)builder.getBits() / keys_count;
            double retrieval = 8.0 * (double)ribbon->size() / keys_count;
            printf("Elias-Fano cumul sizes:  %f bits/bucket\n", (double)ef.bitCountCumKeys() / nbuckets);
            printf("Elias-Fano cumul bits:   %f bits/bucket\n", (double)ef.bitCountPosition() / nbuckets);
            printf("Elias-Fano cumul sizes:  %f bits/key\n", ef_sizes);
            printf("Elias-Fano cumul bits:   %f bits/key\n", ef_bits);
            printf("Rice-Golomb descriptors: %f bits/key\n", rice_desc);
            printf("Retrieval:               %f bits/key\n", retrieval);
            printf("Total bits:              %f bits/key\n", ef_sizes + ef_bits + rice_desc + retrieval);

            printf("Total split bits        %16.3f\n", (double)split_fixed + split_unary);
            printf("Total bij bits:         %16.3f\n", (double)bij_fixed + bij_unary);

            printf("\n");
            printf("Bijections: %13.3f ms\n", time_bij * 1E-6);
            for (int i = 0; i < MAX_LEVEL_TIME; i++) {
                if (time_split[i] > 0) {
                    printf("Split level %d: %10.3f ms\n", i, time_split[i] * 1E-6);
                }
            }
#endif
        }
};

} // namespace shockhash
