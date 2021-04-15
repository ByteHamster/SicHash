#ifndef TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDGREEDY_H
#define TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDGREEDY_H

#include <functional>

#include "HashAssignmentThreshold.h"
#include "hungarian.h"

class HashAssignmentThresholdGreedy : public HashAssignmentThreshold {
    private:
        std::vector<std::vector<ElementHasher>> elements;
    public:
        HashAssignmentThresholdGreedy(int M, int N, int thresholds[NUM_THRESHOLDS]);
        void insert(ElementHasher element) override;
        bool construct() override;
        std::string name() override;
};


#endif //TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDGREEDY_H
