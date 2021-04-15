#ifndef TESTCOMPARISON_HDCSLIDINGWINDOW_H
#define TESTCOMPARISON_HDCSLIDINGWINDOW_H

#include <cstring>
#include <algorithm>
#include <thread>
#include "HdcMax.h"

// Assign each bucket a hash function like in
// "Hash, displace, and compress" (Belazzougui, Botelho, Dietzfelbinger)
// but do not hash each bucket into whole domain (only sliding window),
// which enables parallel construction.
// "Efficient Minimal Perfect Hashing in Nearly Minimal Space" (Hagerup Tholey)
// Uses a similar idea, too, but they use the sliding window to achieve better compression,
// not faster construction.
class HdcSlidingWindow : public Hdc {
    public:
        int numThreads;
        HdcSlidingWindow(int M, int N, int BucketSize, int NumHashFunctions, int K);

        void construct();
        void handleBucket(int bucket);

        inline int hash(std::string &key) {
            std::size_t mhc = Hash::hash(key, HASH_FUNCTION_MHC, INT64_MAX);
            int bucket = Hash::hash(mhc, HASH_FUNCTION_BUCKET, NumBuckets);
            cmph_uint32 hashFunction = compressed_seq_query(compressed, bucket); //buckets[bucket].hashFunction;
            int slidingWindowStart = bucket * ((double) M / NumBuckets);
            if (hashFunction == HASH_FUNCTION_BUMP || hashFunction > NumHashFunctions + 1) {
                return -1;
            }
            return slidingWindowStart + Hash::hash(mhc, hashFunction, BUCKET_SLIDING_SIZE);
        }

        inline int operator ()(std::string &key) {
            return hash(key);
        }
};

#endif //TESTCOMPARISON_HDCSLIDINGWINDOW_H
