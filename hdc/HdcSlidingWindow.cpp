#include "HdcSlidingWindow.h"

HdcSlidingWindow::HdcSlidingWindow(int M, int N, int BucketSize, int NumHashFunctions, int K)
    : Hdc(M, N, BucketSize, NumHashFunctions, K) {

}

void HdcSlidingWindow::handleBucket(int bucket) {
    int slidingWindowStart = bucket * ((double) M / NumBuckets);
    // Assign hash function to this bucket
    int hashFunction = -1;
    bool placed = false;
    while (!placed && hashFunction < NumHashFunctions) {
        hashFunction++;
        placed = place(bucket, hashFunction, slidingWindowStart, BUCKET_SLIDING_SIZE);
    }
    if (hashFunction == NumHashFunctions) {
        buckets[bucket].hashFunction = HASH_FUNCTION_BUMP;
        hashFunctionUsageCounter[hashFunction]++;
    } else {
        buckets[bucket].hashFunction = hashFunction;
    }
}

void HdcSlidingWindow::construct() {
    std::vector<std::vector<int>> bucketsOfSize(BucketSize * BucketSize * BucketSize); // This is way too much space but let's be sure
    for (int i = 0; i < NumBuckets; i++) {
        bucketsOfSize[buckets[i].size].push_back(i);
    }

    for (int size = bucketsOfSize.size() - 1; size >= 0; size--) {
        int numBucketsWithSize = bucketsOfSize[size].size();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        if (numBucketsWithSize/numThreads < 500 || numThreads == 1) {
            // Sequential
            for (int i = 0; i < numBucketsWithSize; i++) {
                int bucket = bucketsOfSize[size][i];
                handleBucket(bucket);
            }
        } else {
            // Parallel
            std::vector<int> threadHandled(numThreads);
            std::vector<std::thread> threads;
            threads.reserve(numThreads);
            for (int i = 0; i < numThreads; i++) {
                threads.emplace_back([&, i_ = i] {
                    int startBucket = i_*(numBucketsWithSize/numThreads + 1);
                    int nextStartBucket = (i_+1)*(numBucketsWithSize/numThreads + 1);
                    int maxLocationWritable = nextStartBucket * ((double) M / NumBuckets);
                    int handledBucket;
                    for (handledBucket = startBucket; handledBucket < nextStartBucket; handledBucket++) {
                        int maxLocationWritten = handledBucket * ((double) M / NumBuckets) + BUCKET_SLIDING_SIZE;
                        if (maxLocationWritten >= maxLocationWritable) {
                            break;
                        }
                        handleBucket(bucketsOfSize[size].at(handledBucket));
                    }
                    threadHandled.at(i_) = handledBucket;
                });
            }
            for (int i = 0; i < numThreads; i++) {
                threads.at(i).join();
            }
            threads.clear();
            for (int i = 0; i < numThreads; i++) {
                threads.emplace_back([&, i_ = i] {
                    int startBucket = threadHandled.at(i_);
                    int nextStartBucket = std::min(numBucketsWithSize, (i_+1)*(numBucketsWithSize/numThreads + 1));
                    int handledBucket;
                    for (handledBucket = startBucket; handledBucket < nextStartBucket; handledBucket++) {
                        handleBucket(bucketsOfSize[size].at(handledBucket));
                    }
                });
            }
            for (int i = 0; i < numThreads; i++) {
                threads.at(i).join();
            }
        }
    }
}
