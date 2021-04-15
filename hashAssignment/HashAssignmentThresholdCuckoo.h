#ifndef TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDCUCKOO_H
#define TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDCUCKOO_H

#include <functional>

#include "HashAssignmentThreshold.h"
#include "hungarian.h"
#include "../util/SimpleRibbon.h"

class HashAssignmentThresholdCuckoo : public HashAssignmentThreshold {
    private:
        struct Cuckoo {
            ElementHasher element = {0};
            uint8_t elementClass = 0;
            uint8_t hashFunctionIndex = 0;
            bool isPlaceholder = true;
        };
        std::vector<Cuckoo> storage;
        std::vector<ElementHasher> elements;
        static const int insertTtl = 10000;
        static std::vector<std::pair<int, Cuckoo>> stack;
        bool greedyBump = false;
        bool earlyStop = false; // Bump greedily but when there are more than threshold elements, still go to the next layer
        std::unique_ptr<SimpleRibbon<1>> retrieval1 = nullptr;
        std::unique_ptr<SimpleRibbon<2>> retrieval2 = nullptr;
        std::unique_ptr<SimpleRibbon<3>> retrieval3 = nullptr;
    public:
        std::vector<Cuckoo> bumped;
        std::vector<Cuckoo> bumpedRemainder;

        HashAssignmentThresholdCuckoo(int M, int N, int thresholds[NUM_THRESHOLDS], bool greedyBump = false);
        HashAssignmentThresholdCuckoo(int M, int N, bool greedyBump = false);
        int hash(std::string &key) override;
        int hash(uint64_t mhc);
        void insert(ElementHasher element) override;
        void printEntropy() override;
        bool tryInsert(Cuckoo element);
        bool construct() override;
        bool construct(std::vector<std::vector<std::pair<uint64_t, unsigned char>>> &hashFunctionRetrieval);
        std::string name() override;
        std::size_t size();
};


#endif //TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDCUCKOO_H
