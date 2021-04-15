#ifndef TESTCOMPARISON_CUCKOOWALKMULTI_H
#define TESTCOMPARISON_CUCKOOWALKMULTI_H

#include "HashAssignmentThresholdCuckoo.h"
#include <unistd.h>

class CuckooWalkMulti {
    public:
        struct MultiCuckooEntry {
            std::vector<uint64_t> elements;
            std::unique_ptr<HashAssignmentThresholdCuckoo> structure;
            int seed = 0;
            int sizePrefixSum = 0;
        };
        const int N;
        const int numBuckets;
        const int maxSeed = 15;
        const double mByN;
        double bumpedPercentage;
        std::unique_ptr<SimpleRibbon<1>> retrieval1 = nullptr;
        std::unique_ptr<SimpleRibbon<2>> retrieval2 = nullptr;
        std::unique_ptr<SimpleRibbon<3>> retrieval3 = nullptr;
        int *thresholds;

        std::unique_ptr<MultiCuckooEntry[]> multiEntries;

        CuckooWalkMulti(int N, double mByN, int *thresholds) : numBuckets(N / 2000), N(N),
                mByN(mByN), thresholds(thresholds) {
            multiEntries = std::make_unique<MultiCuckooEntry[]>(numBuckets);
        }

        void construct(std::vector<std::string> &keys) {
            for (int i = 0; i < N; i++) {
                uint64_t mhc = Hash::hash(keys[i], HASH_FUNCTION_MHC, INT64_MAX);
                int bucket = mhc % numBuckets;
                multiEntries[bucket].elements.push_back(mhc);
            }
            std::vector<std::vector<std::pair<uint64_t, unsigned char>>> hashFunctionRetrieval;
            hashFunctionRetrieval.resize(NUM_THRESHOLDS);
            int bumpedBuckets = 0;
            int sizePrefixSum = 0;
            for (int i = 0; i < numBuckets; i++) {
                multiEntries[i].seed = 0;
                const int N = multiEntries[i].elements.size();
                while (multiEntries[i].seed < maxSeed) {
                    int M = N * mByN;
                    auto *cuckoo = new HashAssignmentThresholdCuckoo(M, N, thresholds);
                    for (size_t mhc : multiEntries[i].elements) {
                        cuckoo->insert(ElementHasher(mhc, multiEntries[i].seed));
                    }
                    bool success = cuckoo->construct(hashFunctionRetrieval);
                    if (success) {
                        multiEntries[i].structure = std::unique_ptr<HashAssignmentThresholdCuckoo>(cuckoo);
                        multiEntries[i].sizePrefixSum = sizePrefixSum;
                        sizePrefixSum += M;
                        break;
                    } else {
                        delete cuckoo;
                    }
                    multiEntries[i].seed++;
                }
                if (multiEntries[i].seed == maxSeed) {
                    bumpedBuckets++;
                }
            }
            if (bumpedBuckets != 0) {
                bumpedPercentage = 100.0*bumpedBuckets/numBuckets;
                std::cout<<"Unable to place "<<bumpedBuckets<<" ("<<bumpedPercentage<<"%) of the buckets"<<std::endl;
                if (bumpedPercentage > 20) {
                    throw std::logic_error("Too many buckets were impossible to construct");
                }
            }

            retrieval1 = std::make_unique<SimpleRibbon<1>>(hashFunctionRetrieval.at(0));
            retrieval2 = std::make_unique<SimpleRibbon<2>>(hashFunctionRetrieval.at(1));
            retrieval3 = std::make_unique<SimpleRibbon<3>>(hashFunctionRetrieval.at(2));

            /*int bucketHistogram[maxSeed];
            for (int i = 0; i < maxSeed; i++) {
                bucketHistogram[i] = 0;
            }
            for (int i = 0; i < numBuckets; i++) {
                bucketHistogram[multiEntries[i].seed]++;
            }
            std::cout<<std::endl;
            for (int i = 0; i < maxSeed; i++) {
                std::cout<<i<<": "<<bucketHistogram[i]<<" buckets"<<std::endl;
            }*/
        }

        size_t space_bits() {
            size_t spaceUsed = 8 * (retrieval1->size() + retrieval2->size() + retrieval3->size());
            //std::cout<<"("<<(8.0*retrieval1->size()/(1*hashFunctionRetrieval.at(0).size())-1)*100<<"%, ";
            //std::cout<<(8.0*retrieval2->size()/(2*hashFunctionRetrieval.at(1).size())-1)*100<<"%, ";
            //std::cout<<(8.0*retrieval3->size()/(3*hashFunctionRetrieval.at(2).size())-1)*100<<"%) ";
            spaceUsed += numBuckets * 16; // size prefix sum (deviation from expected size can be stored in 16 bits)
            spaceUsed += numBuckets * log2(maxSeed + 1);
            //std::cout<<bumpedBuckets<<"/"<<numBuckets<<" buckets bumped ";
            return spaceUsed;
        }

        inline int hash(std::string &key) {
            uint64_t mhc = Hash::hash(key, HASH_FUNCTION_MHC, INT64_MAX);
            int bucket = mhc % numBuckets;
            if (multiEntries[bucket].seed == maxSeed) {
                return -1; // Bumped
            }
            ElementHasher element(mhc, multiEntries[bucket].seed);
            int elementClass = multiEntries[bucket].structure->getElementClass(element);
            int hashFunction;
            switch (elementClass) {
                case 0:
                    hashFunction = retrieval1->retrieve(element.mhc);
                    break;
                case 1:
                    hashFunction = retrieval2->retrieve(element.mhc);
                    break;
                case 2:
                    hashFunction = retrieval3->retrieve(element.mhc);
                    break;
                default:
                    assert(false);
            }
            return element.hash(hashFunction, multiEntries[bucket].structure->M) + multiEntries[bucket].sizePrefixSum;
        }

        inline int operator ()(std::string &key) {
            return hash(key);
        }
};


#endif //TESTCOMPARISON_CUCKOOWALKMULTI_H
