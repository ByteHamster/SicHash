#ifndef TESTCOMPARISON_HDCMAX_H
#define TESTCOMPARISON_HDCMAX_H

#include <cstring>
#include <algorithm>
#include <set>
#include <cassert>
#include "Hdc.h"

// Assign each bucket a hash function like in
// "Hash, displace, and compress" (Belazzougui, Botelho, Dietzfelbinger)
// but only choose one hash function for matching or none at all.
// For choosing the hash function, use a branch+bound technique.
// Related: "Constructing Minimal Perfect Hash Functions Using SAT Technology"
// by Weaver, Heule uses SAT solving on a per-element basis to achieve a MPHF
// with N=40 and ~1.4 bits per element.
class HdcMax : public Hdc {
    private:
        char *taken;
        char *best;
        int currentTakenElements = 0;
        int bestTakenElements = 0;
        int elementsLeft = 0;
        std::vector<std::set<int>> collisions = std::vector<std::set<int>>(NumBuckets);
        std::vector<int> bucketInsertionOrder;
    public:
        HdcMax(int M, int N, int BucketSize);
        ~HdcMax();

        bool bucketsCollide(int i, int j);
        void construct();
        void branchAndBound(int nextBucketOrder);
};

#endif //TESTCOMPARISON_HDCMAX_H
