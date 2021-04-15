#ifndef TESTCOMPARISON_HDCGREEDY_H
#define TESTCOMPARISON_HDCGREEDY_H

#include <cstring>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include "../util/Hash.h"
#include "Hdc.h"

// Assign each bucket a hash function like in
// "Hash, displace, and compress" (Belazzougui, Botelho, Dietzfelbinger)
// but only choose one hash function for matching or none at all.
// For choosing the hash function, greedily assign it.
class HdcGreedy : public Hdc {
    public:
        HdcGreedy(int M, int N, int BucketSize, int NumHashFunctions, int K);
        void construct(bool sorted);
        int hash(std::string &key);
};

#endif //TESTCOMPARISON_HDCGREEDY_H
