#pragma once
#include <vector>
#include "IrregularCuckooHashTable.h"
#include "SimpleRibbon.h"
#include "TinyBinaryCuckooHashTable.h"
#include <EliasFano.h>
#include <GolombRice.h>

namespace sichash {
/**
 * ShockHash minimal perfect hash function: Small Heavily Overloaded CucKoo Hash Tables.
 * Note that the supplied load factor is not very granular: the load factor is multiplied with the bucket size.
 */
template<size_t expectedBucketSize, size_t expectedSeed>
class ShockHash {
    private:
        static constexpr size_t MAX_SEED = 100000000; // To avoid endless loop when setting parameters too high
        static constexpr size_t HASH_FUNCTION_BUCKET_ASSIGNMENT = 424242;
        size_t N;
        size_t numBuckets;
        float loadFactor;
        SimpleRibbon<1> *ribbon = nullptr;
        util::EliasFano<util::ceillog2(expectedBucketSize)> bucketOffsets;
        util::GolombRice<util::ceillog2(expectedSeed)> bucketSeeds;
    public:

        ShockHash(const std::vector<std::string> &keys, float loadFactor)
                : N(keys.size()), numBuckets(keys.size() / expectedBucketSize), loadFactor(loadFactor),
                  bucketOffsets(numBuckets + 1, N*(1.0/loadFactor) + 4 * expectedBucketSize), bucketSeeds(numBuckets, 2 * expectedSeed) {
            std::vector<std::vector<HashedKey>> buckets(numBuckets);
            std::vector<std::pair<uint64_t, uint8_t>> ribbonData;
            ribbonData.reserve(N*(1.0/loadFactor));

            std::cout<<"Creating MHCs"<<std::endl;
            for (const std::string& str : keys) {
                HashedKey hash(str);
                size_t bucket = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numBuckets);
                buckets.at(bucket).push_back(hash);
            }

            size_t bucketSizePrefix = 0;
            size_t seedSum = 0;
            for (size_t bucket = 0; bucket < numBuckets; bucket++) {
                size_t thisBucketSize = buckets.at(bucket).size();
                size_t thisBucketM = thisBucketSize*(1.0/loadFactor);
                bucketOffsets.push_back(bucketSizePrefix);
                bucketSizePrefix += thisBucketM;

                TinyBinaryCuckooHashTable table(thisBucketSize, thisBucketM);
                for (HashedKey& hash : buckets.at(bucket)) {
                    table.prepare(HashedKey(hash));
                }

                uint64_t seed = 0;
                for (; seed < MAX_SEED; seed++) {
                    bool success = table.construct(seed);
                    if (success) {
                        bucketSeeds.push_back(seed);
                        for (size_t i = 0; i < thisBucketM; i++) {
                            if (table.cells[i] == nullptr) {
                                continue;
                            }
                            size_t cell1 = table.cells[i]->hash.hash(seed, thisBucketM);
                            ribbonData.emplace_back(table.cells[i]->hash.mhc, i == cell1 ? 0 : 1);

                            #ifndef NDEBUG
                                size_t cell2 = table.cells[i]->hash.hash(seed + 1, thisBucketM);
                                assert(i == cell1 || i == cell2);
                            #endif
                        }
                        break;
                    }
                }
                seedSum += seed;
                if (seed == MAX_SEED) {
                    throw std::logic_error("Unable to construct. Selected bucket size too large.");
                }

                if (bucket % (std::min(numBuckets / 7, 1337ul)) == 0) {
                    std::cout<<"\rConstructing small buckets ("<<(int)(100.0*bucket/numBuckets)<<"%)"<<std::flush;
                }
            }
            bucketOffsets.push_back(bucketSizePrefix);

            std::cout<<"\rAverage seed: "<<(seedSum/numBuckets)
                    <<", compile-time expected average seed: "<<expectedSeed
                    <<" (should be similar for good space usage)"<<std::endl;
            std::cout<<"\rConstructing rank/select and ribbon"<<std::endl<<std::flush;
            bucketOffsets.buildRankSelect();
            bucketSeeds.buildRankSelect();
            assert(ribbonData.size() == N);
            ribbon = new SimpleRibbon<1>(ribbonData);
        }

        ~ShockHash() {
            delete ribbon;
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            float spaceEliasFano = 8.0f * bucketOffsets.space() / N;
            float spaceGolombRice = 8.0f * bucketSeeds.space() / N;
            float spaceRibbon = 8.0f * ribbon->size() / N;
            std::cout << "Elias-Fano offsets: " << spaceEliasFano << std::endl;
            std::cout << "Retrieval:          " << spaceRibbon << std::endl;
            std::cout << "Seeds:              " << spaceGolombRice << std::endl;
            std::cout << "Total:              " << (spaceRibbon + spaceEliasFano + spaceGolombRice) << std::endl;

            return (bucketOffsets.space() + bucketSeeds.space() + ribbon->size()) * 8;
        }

        size_t operator() (std::string &key) const {
            HashedKey hash(key);
            size_t bucket = hash.hash(HASH_FUNCTION_BUCKET_ASSIGNMENT, numBuckets);
            size_t seed = bucketSeeds.at(bucket);
            size_t offset = bucketOffsets.at(bucket);
            size_t bucketSize = bucketOffsets.at(bucket + 1) - offset;
            uint8_t hashFunctionIndex = ribbon->retrieve(hash.mhc);
            return offset + hash.hash(seed + hashFunctionIndex, bucketSize);
        }
};
} // Namespace sichash
