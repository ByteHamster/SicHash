#pragma once
#include <vector>
#include "IrregularCuckooHashTable.h"
#include "SimpleRibbon.h"
#include <EliasFano.h>

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
        SicHashConfig config;
        size_t numSmallTables;
        size_t N;
        std::vector<size_t> smallTableOffsets;
        std::vector<seed_t> smallTableSeeds;
        SimpleRibbon<1, ribbonWidth> *ribbon1 = nullptr;
        SimpleRibbon<2, ribbonWidth> *ribbon2 = nullptr;
        SimpleRibbon<3, ribbonWidth> *ribbon3 = nullptr;
        std::vector<size_t> emptySlots;
        util::EliasFano<minimalFanoLowerBits> minimalRemap;
        size_t unnecessaryConstructions = 0;

        SicHash(const std::vector<std::string> &keys,
                SicHashConfig _config)
                  : config(_config), N(keys.size()),
                        minimalRemap(minimal ? (N / config.loadFactor - N) : 0, minimal ? N : 0) {
            numSmallTables = keys.size() / config.smallTableSize + 1;
            std::vector<IrregularCuckooHashTable> tables;
            tables.reserve(numSmallTables);
            IrregularCuckooHashTableConfig cuckooConfig;
            cuckooConfig.threshold1 = config.threshold1;
            cuckooConfig.threshold2 = config.threshold2;
            cuckooConfig.maxEntries = config.smallTableSize * 1.2 + 100;
            for (size_t i = 0; i < numSmallTables; i++) {
                tables.emplace_back(cuckooConfig);
            }

            if (!config.silent) {
                std::cout<<"Creating MHCs"<<std::endl;
            }
            smallTableOffsets.reserve(numSmallTables);
            smallTableSeeds.reserve(numSmallTables);
            for (const std::string &key : keys) {
                HashedKey hash = HashedKey(key);
                size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
                tables[smallTable].prepare(hash);
            }

            if (!config.silent) {
                std::cout<<"Inserting into Cuckoo"<<std::endl;
            }
            std::vector<std::vector<std::pair<uint64_t, uint8_t>>> maps; // Avoids conditional jumps later
            maps.resize(0b111 + 1);
            maps[0b001].reserve(keys.size() * config.class1Percentage());
            maps[0b011].reserve(keys.size() * config.class2Percentage());
            maps[0b111].reserve(keys.size() * config.class3Percentage());
            size_t sizePrefix = 0;
            unnecessaryConstructions = 0;
            for (IrregularCuckooHashTable &table : tables) {
                size_t tableM = table.size() / config.loadFactor;
                size_t seed = 0;
                while (!table.construct(tableM, seed)) {
                    unnecessaryConstructions++;
                    seed++;
                    if (seed >= std::numeric_limits<seed_t>::max()) {
                        throw std::logic_error("Selected thresholds that cannot be constructed");
                    }
                }
                smallTableSeeds.push_back(seed);
                smallTableOffsets.push_back(sizePrefix);

                for (size_t i = 0; i < table.size(); i++) {
                    IrregularCuckooHashTable::TableEntry &entry = table.heap[i];
                    maps[entry.hashFunctionMask].emplace_back(entry.hash.mhc, entry.hashFunctionIndex);
                }
                if constexpr (minimal) {
                    for (size_t i = 0; i < tableM; i++) {
                        if (table.cells[i] == nullptr) {
                            emptySlots.push_back(sizePrefix + i);
                        }
                    }
                }
                sizePrefix += tableM;
            }

            smallTableOffsets.push_back(sizePrefix);
            if (!config.silent) {
                std::cout<<"On average, the small hash tables needed to be retried "
                         <<(double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables<<" times"<<std::endl;
                std::cout<<"Constructing Ribbon"<<std::endl;
            }
            ribbon1 = new SimpleRibbon<1, ribbonWidth>(maps[0b001]);
            ribbon2 = new SimpleRibbon<2, ribbonWidth>(maps[0b011]);
            ribbon3 = new SimpleRibbon<3, ribbonWidth>(maps[0b111]);

            if constexpr (minimal) {
                if (!config.silent) {
                    std::cout<<"Making minimal"<<std::endl;
                }
                size_t smallTableToRemap = 0;
                while (smallTableOffsets[smallTableToRemap] < N) {
                    smallTableToRemap++;
                }
                smallTableToRemap--;
                // Iterate over last few tables and remap filled positions
                size_t emptyIndex = 0;
                size_t i = keys.size() - smallTableOffsets[smallTableToRemap];
                for (;smallTableToRemap < numSmallTables; smallTableToRemap++) {
                    for (; i < tables[smallTableToRemap].M; i++) {
                        minimalRemap.push_back(emptySlots[emptyIndex]);
                        if (tables[smallTableToRemap].cells[i] != nullptr) {
                            emptyIndex++;
                            if (emptySlots[emptyIndex] >= N) {
                                // No more empty slots left. We do not need to write the following items to
                                // the EF sequence because they are never queried
                                emptyIndex--;
                                break;
                            }
                        }
                    }
                    i = 0;
                }
                minimalRemap.buildRankSelect();
                emptySlots.clear();
                emptySlots.shrink_to_fit();
            }
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            size_t bytes = ribbon1->size() + ribbon2->size() + ribbon3->size()
                    + smallTableOffsets.size() * sizeof(size_t)
                    + smallTableSeeds.size() * sizeof(seed_t);
            if constexpr (minimal) {
                bytes += minimalRemap.space();
            }
            return bytes * 8;
        }

        /** Theoretic space usage when pretending to encode bucket metadata with Rice and Elias-Fano */
        [[nodiscard]] size_t spaceUsageTheory() const {
            size_t bytes = ribbon1->size() + ribbon2->size() + ribbon3->size();
            if constexpr (minimal) {
                bytes += minimalRemap.space();
            }

            size_t efN = smallTableOffsets.size();
            size_t efBits = 2 * efN;
            efBits += efN * std::ceil(std::log2((double) smallTableOffsets.back() / (double)efN));

            size_t golombBits = 0;
            double averageSeed = (double)(unnecessaryConstructions+numSmallTables)/(double)numSmallTables;
            size_t b = std::log2(averageSeed);
            for (size_t seed : smallTableSeeds) {
                size_t q = seed >> b;
                // size_t r = seed - q;
                golombBits += b; // Remainder binary coded
                golombBits += q + 1; // Quotient unary coded
            }
            return bytes * 8 + efBits + golombBits;
        }

        size_t operator() (std::string &key) const {
            HashedKey hash = HashedKey(key);
            size_t smallTable = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numSmallTables);
            uint8_t hashFunction;
            if (hash.mhc <= config.threshold1) {
                hashFunction = ribbon1->retrieve(hash.mhc);
            } else if (hash.mhc <= config.threshold2) {
                hashFunction = ribbon2->retrieve(hash.mhc);
            } else {
                hashFunction = ribbon3->retrieve(hash.mhc);
            }
            size_t M = smallTableOffsets[smallTable + 1] - smallTableOffsets[smallTable];
            size_t result = hash.hash(hashFunction + smallTableSeeds[smallTable], M) + smallTableOffsets[smallTable];
            if constexpr (minimal) {
                if (result >= N) {
                    return minimalRemap.at(result - N);
                }
            }
            return result;
        }
};
} // Namespace sichash
