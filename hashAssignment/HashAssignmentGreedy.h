#ifndef TESTCOMPARISON_HASHASSIGNMENTGREEDY_H
#define TESTCOMPARISON_HASHASSIGNMENTGREEDY_H

#include "HashAssignment.h"

//#define GREEDY_TABLE_SIZE (1 << 16)

// Assign each item a hash function
// by greedily looking for the first hash function
// where the resulting value is not yet taken
// Minimum: 0.89n bits for m=1.23n
// (source: Practical perfect hashing in nearly optimal space)
class HashAssignmentGreedy : public HashAssignment {
    public:
        HashAssignmentGreedy(int M, int N);
        void insert(ElementHasher element) override;
        bool construct() override;
        std::string name() override;
};


#endif //TESTCOMPARISON_HASHASSIGNMENTGREEDY_H
