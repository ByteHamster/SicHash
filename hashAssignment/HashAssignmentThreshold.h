#ifndef TESTCOMPARISON_HASHASSIGNMENTTHRESHOLD_H
#define TESTCOMPARISON_HASHASSIGNMENTTHRESHOLD_H

#include <functional>

#include "HashAssignment.h"
#include "hungarian.h"

#define HASH_FUNCTION_CLASSIFY -11

#define NUM_THRESHOLDS 3
static const int HASH_FUNCTIONS_PER_CLASS[NUM_THRESHOLDS] = {2, 4, 8};
static const int HASH_FUNCTIONS_PER_CLASS_MASK[NUM_THRESHOLDS] = {0b1, 0b11, 0b111};
static const float COST_PER_CLASS[NUM_THRESHOLDS] = {1, 2, 3};

// For the greedy cuckoo-based hashing with bumping
//#define NUM_THRESHOLDS 6
//static int HASH_FUNCTIONS_PER_CLASS[NUM_THRESHOLDS] = {1, 2, 3, 4, 5, 7};
//static float COST_PER_CLASS[NUM_THRESHOLDS] = {0, 1, 1.6, 2, 2.3, 3};

// Last one is a retrieval data structure that stores
// 5 values from [1, 3] using  8 bits ==> 1.60 bits per value
// 3 values from [1, 5] using  7 bits ==> 2.33 bits per value
// 6 values from [1, 7] using 17 bits ==> 2.83 bits per value
// 5 values from [1, 9] using 16 bits ==> 3.20 bits per value
// Does not work like XOR but solving is possible if the number of possible
// values is prime (which happens a lot around 1/2/3 bits)
// plot frac(log2(7^round(x))) for x from 0 to 20
//static int HASH_FUNCTIONS_PER_CLASS[NUM_THRESHOLDS] = {2, 5, 8};
//static float COST_PER_CLASS[NUM_THRESHOLDS] = {1, 2.33, 3};

// Build 3 retrieval data structures with 1, 2 and 3 bits.
// Hash all elements to one of the structures (depending on thresholds, input partitioning)
// and store the hash function index that does not cause collisions
// in its data structure. The idea is that 2 hash functions (1 bit)
// can already fill the target domain to 50%. The other elements have more
// choices and can therefore fill the domain more.
class HashAssignmentThreshold : public HashAssignment {
    public:
        int thresholds[NUM_THRESHOLDS];
        int hashFunctionUsagePerBucket[NUM_THRESHOLDS][8] = {0};
        std::vector<std::vector<ElementHasher>> classes;
        std::vector<std::unordered_map<uint64_t, int>> hashFunctionRetrieval;

        HashAssignmentThreshold(int M, int N, int thresholds[NUM_THRESHOLDS]);
        int getElementClass(ElementHasher element) const;
        virtual int hash(std::string &key);
        void printEntropy() override;
        void notifyInserted(ElementHasher element, int hashFunction) override;
        std::string name() override;
};


#endif //TESTCOMPARISON_HASHASSIGNMENTTHRESHOLD_H
