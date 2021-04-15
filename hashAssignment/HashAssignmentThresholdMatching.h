#ifndef TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDMATCHING_H
#define TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDMATCHING_H

#include <functional>

#include "HashAssignment.h"
#include "HashAssignmentThreshold.h"
#include "hungarian.h"

class HashAssignmentThresholdMatching : public HashAssignmentThreshold {
    private:
        std::vector<WeightedBipartiteEdge> edges;
        std::vector<ElementHasher> elements;
    public:
        HashAssignmentThresholdMatching(int M, int N, int thresholds[NUM_THRESHOLDS]);
        void insert(ElementHasher element) override;
        bool construct() override;
        std::string name() override;
};


#endif //TESTCOMPARISON_HASHASSIGNMENTTHRESHOLDMATCHING_H
