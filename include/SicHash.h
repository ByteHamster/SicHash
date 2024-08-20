#pragma once
#include <vector>
#include "IrregularCuckooHashTable.h"
#include <SimpleRibbon.h>
#include <EliasFano.h>
#include <ips2ra.hpp>

namespace sichash {
using seed_t = uint8_t;

struct SicHashConfig {
    // Load factor of the PHF. When constructing an MPHF, this is the load factor before compaction.
    double loadFactor = 0.9;

    // Expected size of each of the small cuckoo hash tables.
    size_t smallTableSize = 5000;

    // Don't print progress to std::cout. The messages are rare, so they do not affect the measurements,
    // but they might be annoying when using SicHash as a library.
    bool silent = false;

    // Main configuration parameters. Set values using .percentages() or .spaceBudget().
    uint64_t threshold1;
    uint64_t threshold2;

    // For convenience, only set when calling .spaceBudget(). Not used during construction.
    float x = -1;

    SicHashConfig() {
        percentages(0.5, 0.25);
    }

    /**
     * Percentages in [0, 1] of items with 2 and 4 choices (1 and 2 bits, respectively).
     * The percentage of items with 8 choices (3 bits) is calculated automatically.
     * Both values must be >=0 and the sum must be <=1
     */
    SicHashConfig &percentages(float percentage1, float percentage2) {
        if (percentage1 + percentage2 > 1.0) {
            throw std::logic_error("Selected thresholds have >100%");
        }
        if (percentage1 < 0.0 || percentage2 < 0.0) {
            throw std::logic_error("Selected negative thresholds");
        }
        threshold1 = UINT64_MAX * 0.99999f * percentage1;
        threshold2 = UINT64_MAX * 0.99999f * (percentage1 + percentage2);
        if (threshold2 < threshold1) {
            throw std::logic_error("Overflow when determining thresholds");
        }
        return *this;
    }

    /**
     * Try to construct a PHF with a given space budget (in bits per key).
     * Because we are using 1,2,3 bit retrieval data structures, the space budget must be in [1, 3].
     * Parameter x in [0, 1] is a tuning parameter for selecting which mix of hash functions to use.
     * High x have a higher load threshold, while low x are usually faster to construct.
     */
    SicHashConfig &spaceBudget(float spaceBudget, float _x) {
        x = _x;
        spaceBudget -= 8.0 * (sizeof(size_t) + sizeof(seed_t)) / smallTableSize;
        if (x < 0.0 || x > 1.0) {
            throw std::logic_error("x must be in [0, 1]");
        }
        if (spaceBudget < 1.0 || spaceBudget > 3.0) {
            throw std::logic_error("space budget must be in [1, 3]");
        }
        float p1_min = std::max(0.0, 2.0 - spaceBudget);
        float p1_max = (3 - spaceBudget) / 2;
        float p1 = p1_min + (p1_max - p1_min) * x;
        float p2 = 3 - 2*p1 - spaceBudget;
        percentages(p1, p2);
        return *this;
    }

    [[nodiscard]] double class1Percentage() const {
        return (double) threshold1 / (double) UINT64_MAX;
    }

    [[nodiscard]] double class2Percentage() const {
        return (double) (threshold2 - threshold1) / (double) UINT64_MAX;
    }

    [[nodiscard]] double class3Percentage() const {
        return (double) (UINT64_MAX - threshold2 - threshold1) / (double) UINT64_MAX;
    }
};

/**
 * SicHash perfect hash function.
 * @tparam minimal Remap values >N to empty slots to get a MPHF
 * @tparam ribbonWidth Tuning parameter for the ribbon retrieval data structure. Usually 64 or 32.
 * @tparam minimalFanoLowerBits Number of lower bits in the EliasFano coding for remapping.
 *                              Only interesting for minimal=true. See paper for details.
 *                              loadFactor < ~0.89 ==> use minimalFanoLowerBits=3
 *                              loadFactor < ~0.94 ==> use minimalFanoLowerBits=4
 *                              loadFactor < ~0.97 ==> use minimalFanoLowerBits=5
 */
template<bool minimal=false, size_t ribbonWidth=64, int minimalFanoLowerBits = 3>
class SicHash {
    public:
        static constexpr size_t HASH_FUNCTION_BUCKET_ASSIGNMENT = 42;
        const SicHashConfig config;
        size_t N;
        const size_t numSmallTables;
        struct BucketInfo {
            size_t offset : 48;
            size_t seed   : 16;
        };
        std::vector<BucketInfo> bucketInfo;
        SimpleRibbon<1, ribbonWidth> *ribbon1 = nullptr;
        SimpleRibbon<2, ribbonWidth> *ribbon2 = nullptr;
        SimpleRibbon<3, ribbonWidth> *ribbon3 = nullptr;
        util::EliasFano<minimalFanoLowerBits> *minimalRemap = nullptr;
        size_t unnecessaryConstructions = 0;
        size_t M = 0;

        // Keys parameter must be an std::vector<std::string> or an std::vector<HashedKey>.
        SicHash(const auto &keys, SicHashConfig _config)
                : config(_config),
                  N(keys.size()),
                  numSmallTables(N / config.smallTableSize + 1),
                  bucketInfo(numSmallTables + 1) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            if (!config.silent) {
                std::cout << "Creating MHCs" << std::endl;
            }
            std::vector<std::pair<size_t, HashedKey>> hashedKeys(N);
            initialHash(0, N, keys, hashedKeys);
            ips2ra::sort(hashedKeys.begin(), hashedKeys.end(),
                         [](const std::pair<size_t, HashedKey> &pair) { return pair.first; });

            if (!config.silent) {
                std::cout << "MHCs took " << std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - begin).count() << std::endl;
            }

            construct(hashedKeys);
        }

        void construct(std::vector<std::pair<size_t, HashedKey>> &hashedKeys) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            if (!config.silent) {
                std::cout<<"Inserting into Cuckoo"<<std::endl;
            }
            std::vector<std::vector<std::pair<uint64_t, uint8_t>>> maps; // Avoids conditional jumps later
            maps.resize(0b111 + 1);
            maps[0b001].reserve(N * config.class1Percentage());
            maps[0b011].reserve(N * config.class2Percentage());
            maps[0b111].reserve(N * config.class3Percentage());
            unnecessaryConstructions = 0;
            hashedKeys.emplace_back(numSmallTables + 1, 0); // Sentinel

            std::vector<size_t> emptySlots;
            std::vector<size_t> takenSlotsOutsideRange;
            if constexpr (minimal) {
                emptySlots.reserve(N / config.loadFactor - N);
                takenSlotsOutsideRange.reserve(N / config.loadFactor - N);
            }
            constructSmallTables(0, numSmallTables, hashedKeys, emptySlots, maps, takenSlotsOutsideRange);
            M = bucketInfo[0].offset;
            bucketInfo[numSmallTables].offset = M;
            bucketInfo[0].offset = 0;

            if (!config.silent) {
                std::cout << "Buckets took " << std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - begin).count() << std::endl;
                begin = std::chrono::steady_clock::now();
                std::cout<<"On average, the small hash tables needed to be retried "
                         <<(double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables<<" times"<<std::endl;
                std::cout<<"Constructing Ribbon"<<std::endl;
            }

            ribbon1 = new SimpleRibbon<1, ribbonWidth>(maps[0b001]);
            ribbon2 = new SimpleRibbon<2, ribbonWidth>(maps[0b011]);
            ribbon3 = new SimpleRibbon<3, ribbonWidth>(maps[0b111]);
            if (!config.silent) {
                std::cout << "Ribbon took " << std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - begin).count() << std::endl;
            }

            if constexpr (minimal) {
                std::vector<size_t> emptySlotsWithGaps;
                emptySlots.reserve(N / config.loadFactor - N);
                size_t emptySlotsIdx = 0;
                size_t takenSlotsOutsideIdx = 0;
                for (size_t slot = N; slot < M; slot++) {
                    if (takenSlotsOutsideRange[takenSlotsOutsideIdx] < slot) {
                        takenSlotsOutsideIdx++;
                    }
                    emptySlotsWithGaps.push_back(emptySlots[emptySlotsIdx]);
                    if (takenSlotsOutsideRange[takenSlotsOutsideIdx] == slot) {
                        emptySlotsIdx++; // Consume empty slot
                    }
                }
                minimalRemap = new util::EliasFano<minimalFanoLowerBits>(
                        emptySlotsWithGaps.size(), emptySlotsWithGaps.back() + 1);
                for (size_t slot : emptySlotsWithGaps) {
                    minimalRemap->push_back(slot);
                }
                minimalRemap->buildRankSelect();
                M = N;
            }
        }

        void initialHash(size_t from, size_t to, const auto &keys,
                         std::vector<std::pair<size_t, HashedKey>> &hashedKeys) {
            for (size_t i = from; i < to; i++) {
                HashedKey hash = HashedKey(keys[i]);
                size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
                hashedKeys[i] = std::make_pair(smallTable, hash);
            }
        }

        void constructSmallTables(size_t from, size_t to, const std::vector<std::pair<size_t, HashedKey>> &hashedKeys,
                                  std::vector<size_t> &emptySlots,
                                  std::vector<std::vector<std::pair<uint64_t, uint8_t>>> &maps,
                                  std::vector<size_t> &takenSlotsOutsideRange) {
            IrregularCuckooHashTableConfig cuckooConfig;
            cuckooConfig.threshold1 = config.threshold1;
            cuckooConfig.threshold2 = config.threshold2;
            cuckooConfig.maxEntries = config.smallTableSize * 1.2 + 100;
            IrregularCuckooHashTable irregularCuckooHashTable(cuckooConfig);
            size_t sizePrefix = 0;

            // Find key to start with
            size_t keyIdx = (double(from) / double(numSmallTables)) * N; // Rough estimate
            while (hashedKeys[keyIdx].first < from) {
                keyIdx++;
            }
            while (hashedKeys[keyIdx].first > from) {
                keyIdx--;
            }

            for (size_t bucketIdx = from; bucketIdx < to; bucketIdx++) {
                irregularCuckooHashTable.clear();
                while (hashedKeys[keyIdx].first == bucketIdx) {
                    irregularCuckooHashTable.prepare(hashedKeys[keyIdx].second);
                    keyIdx++;
                }
                size_t tableM = irregularCuckooHashTable.size() / config.loadFactor;
                size_t seed = 0;
                while (!irregularCuckooHashTable.construct(tableM, seed)) {
                    unnecessaryConstructions++;
                    seed++;
                    if (seed >= std::numeric_limits<seed_t>::max()) {
                        throw std::logic_error("Selected thresholds that cannot be constructed");
                    }
                }
                bucketInfo[bucketIdx] = BucketInfo(sizePrefix, seed);

                for (size_t k = 0; k < irregularCuckooHashTable.size(); k++) {
                    IrregularCuckooHashTable::TableEntry &entry = irregularCuckooHashTable.heap[k];
                    maps[entry.hashFunctionMask].emplace_back(entry.hash.mhc, entry.hashFunctionIndex & entry.hashFunctionMask);
                }
                if constexpr (minimal) {
                    for (size_t k = 0; k < tableM; k++) {
                        size_t position = sizePrefix + k;
                        if (irregularCuckooHashTable.cells[k] == nullptr) {
                            emptySlots.push_back(position);
                        } else if (position >= N) {
                            takenSlotsOutsideRange.push_back(position);
                        }
                    }
                }
                sizePrefix += tableM;
            }
            bucketInfo[from].offset = sizePrefix;
        }

        ~SicHash() {
            delete ribbon1;
            delete ribbon2;
            delete ribbon3;
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            size_t ribbonBytes = ribbon1->size() + ribbon2->size() + ribbon3->size();
            std::cout<<"Ribbon space: "<<8.0*ribbonBytes/N<<std::endl;
            size_t bytes = ribbonBytes + bucketInfo.size() * sizeof(bucketInfo.at(0));
            if constexpr (minimal) {
                bytes += minimalRemap->space();
                std::cout<<"Remap space: "<<8.0*minimalRemap->space()/N<<std::endl;
            }
            return bytes * 8;
        }

        /** Theoretic space usage when pretending to encode bucket metadata with Rice and Elias-Fano */
        [[nodiscard]] size_t spaceUsageTheory() const {
            size_t bytes = ribbon1->size() + ribbon2->size() + ribbon3->size();
            if constexpr (minimal) {
                bytes += minimalRemap->space();
            }

            size_t efN = bucketInfo.size();
            size_t efBits = 2 * efN;
            efBits += efN * std::ceil(std::log2((double) bucketInfo.back().offset / (double)efN));

            size_t golombBits = 0;
            double averageSeed = (double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables;
            size_t b = std::log2(averageSeed);
            for (auto [offset, seed] : bucketInfo) {
                size_t q = seed >> b;
                // size_t r = seed - q;
                golombBits += b; // Remainder binary coded
                golombBits += q + 1; // Quotient unary coded
            }
            return bytes * 8 + efBits + golombBits;
        }

        // Parameter must be an std::string or a HashedKey.
        size_t operator() (const auto &key) const {
            HashedKey hash = HashedKey(key);
            size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
            __builtin_prefetch(&bucketInfo[smallTable],0,0);
            uint8_t hashFunction;
            if (hash.mhc <= config.threshold1) {
                hashFunction = ribbon1->retrieve(hash.mhc);
            } else if (hash.mhc <= config.threshold2) {
                hashFunction = ribbon2->retrieve(hash.mhc);
            } else {
                hashFunction = ribbon3->retrieve(hash.mhc);
            }
            size_t smallTableM = bucketInfo[smallTable + 1].offset - bucketInfo[smallTable].offset;
            size_t result = hash.hash(hashFunction + bucketInfo[smallTable].seed, smallTableM)
                    + bucketInfo[smallTable].offset;
            if constexpr (minimal) {
                if (result >= N) {
                    return *minimalRemap->at(result - N);
                }
            }
            return result;
        }
};
} // Namespace sichash
