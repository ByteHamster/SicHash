#include "HdcMax.h"

HdcMax::HdcMax(int M, int N, int BucketSize) : Hdc(M, N, BucketSize, 1, 1) {
    taken = new char[NumBuckets];
    best = new char[NumBuckets];
}

HdcMax::~HdcMax() {
    delete [] taken;
    delete [] best;
}

bool HdcMax::bucketsCollide(int i, int j) {
    std::set<int> positions;
    for (int mhc : buckets[i].elementsMhc) {
        int hash = mhc % M;
        if (positions.count(hash) != 0) {
            return true; // Self-collisions
        }
        positions.emplace(hash);
    }
    if (i == j) {
        // If no self-collisions are found before, there are none
        return false;
    }
    for (int mhc : buckets[j].elementsMhc) {
            int hash = mhc % M;
        if (positions.count(hash) != 0) {
            return true;
        }
    }
    return false;
}

void HdcMax::construct() {
    for (int i = 0; i < NumBuckets; i++) {
        for (int j = i; j < NumBuckets; j++) {
            if (bucketsCollide(i, j)) {
                collisions[j].emplace(i);
                collisions[i].emplace(j);
            }
        }
    }
    bucketInsertionOrder.reserve(NumBuckets);
    for (int i = 0; i < NumBuckets; i++) {
        bucketInsertionOrder.push_back(i);
    }
    // Insert buckets in order of ascending size
    std::sort(bucketInsertionOrder.begin(), bucketInsertionOrder.end(), [&](int lhs, int rhs) {
        return buckets[lhs].size > buckets[rhs].size;
    });

    memset(placed, 0, M);
    memset(taken, 0, NumBuckets);
    bestTakenElements = 0;
    elementsLeft = N;
    branchAndBound(0);

    memset(placed, 0, M);
    for (int i = 0; i < NumBuckets; i++) {
        if (best[i] == 0) {
            hashFunctionUsageCounter[HASH_FUNCTION_BUMP]++;
            continue;
        }
        place(i, 0, 0, M);
    }
}

void HdcMax::branchAndBound(int nextBucketOrder) {
    int nextBucket = bucketInsertionOrder[nextBucketOrder];
    if (currentTakenElements > bestTakenElements) {
        bestTakenElements = currentTakenElements;
        memcpy(best, taken, NumBuckets);
        //std::cout<<"Found new best: "<<currentTakenElements<<" elements placed"<<std::endl;
    }
    if (nextBucketOrder == NumBuckets) {
        return; // Reached leaf node
    }
    if (currentTakenElements + elementsLeft < bestTakenElements) {
        // Even when taking all remaining elements, this cannot beat the best
        return;
    }

    bool placeable = true;
    for (int other : collisions[nextBucket]) {
        if (taken[other] || nextBucket == other) {
            // No need to try adding this bucket. It is colliding with an already placed one or with itself.
            placeable = false;
        }
    }

    if (placeable) {
        // Place bucket
        taken[nextBucket] = 1;
        currentTakenElements += buckets[nextBucket].size;
        elementsLeft -= buckets[nextBucket].size;
        for (int mhc : buckets[nextBucket].elementsMhc) {
            int hash = mhc % M;
            assert(placed[hash] == 0);
            placed[hash] = 1;
        }
        // Try others
        branchAndBound(nextBucketOrder + 1);
        // Remove bucket again
        taken[nextBucket] = 0;
        currentTakenElements -= buckets[nextBucket].size;
        elementsLeft += buckets[nextBucket].size;
        for (int mhc : buckets[nextBucket].elementsMhc) {
            int hash = mhc % M;
            assert(placed[hash] == 1);
            placed[hash] = 0;
        }
    }

    // Don't place bucket
    taken[nextBucket] = 0;
    elementsLeft -= buckets[nextBucket].size;
    branchAndBound(nextBucketOrder + 1);
    elementsLeft += buckets[nextBucket].size;
}
