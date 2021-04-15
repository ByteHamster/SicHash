#ifndef TESTCOMPARISON_HASHASSIGNMENTMATCHING_H
#define TESTCOMPARISON_HASHASSIGNMENTMATCHING_H

#include <functional>

#include "HashAssignment.h"
#include "hungarian.h"

#define BUMP_SLOTS 100
#define BUMP_COST_INDEX 10

// Assign each item a hash function
// by building a minimum cost perfect matching instance
// that prefers hash functions with lower index
// (depending on the cost function)
// Pretty much the same as "Constructing Minimal Perfect Hash Functions Using SAT Technology"
// by Weaver, Heule. They even use unary coding like we do here.
class HashAssignmentMatching : public HashAssignment {
    private:
        std::vector<WeightedBipartiteEdge> edges;
        std::vector<ElementHasher> elements;
        const std::string functionName;
        const bool bumping;
    public:
        const std::function<int(int)> &costFunction;

        HashAssignmentMatching(int M, int N,
                               const std::function<int(int)> &costFunction,
                               const std::string &functionName,
                               bool bumping);
        void insert(ElementHasher element) override;
        bool construct() override;
        std::string name() override;
};


#endif //TESTCOMPARISON_HASHASSIGNMENTMATCHING_H
