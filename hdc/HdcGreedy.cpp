#include "HdcGreedy.h"

HdcGreedy::HdcGreedy (int M, int N, int BucketSize, int NumHashFunctions, int K)
    : Hdc(M, N, BucketSize, NumHashFunctions, K) {
}

int HdcGreedy::hash(std::string &key) {
    std::size_t mhc = Hash::hash(key, HASH_FUNCTION_MHC, INT64_MAX);
    int bucket = Hash::hash(mhc, HASH_FUNCTION_BUCKET, NumBuckets);
    int hashFunction = buckets[bucket].hashFunction;
    if (compressed) {
        hashFunction = compressed_seq_query(compressed, bucket);
    }
    if (hashFunction == HASH_FUNCTION_BUMP || hashFunction > NumHashFunctions) {
        return -1;
    }
    return Hash::hash(mhc, hashFunction, M);
}

void HdcGreedy::construct(bool sorted) {
    std::vector<int> bucketInsertionOrder(NumBuckets);

    if (sorted) {
        sort(bucketInsertionOrder);
    } else {
        for (int i = 0; i < NumBuckets; i++) {
            bucketInsertionOrder[i] = i;
        }
    }

    for (int i = 0; i < NumBuckets; i++) {
        int bucket = bucketInsertionOrder[i];
        // Assign hash function to this bucket
        int hashFunction = -1;
        bool placed = false;
        while (!placed && hashFunction < NumHashFunctions) {
            hashFunction++;
            placed = place(bucket, hashFunction, 0, M);
        }
        if (hashFunction == NumHashFunctions) {
            buckets[bucket].hashFunction = HASH_FUNCTION_BUMP;
            hashFunctionUsageCounter[HASH_FUNCTION_BUMP]++;
        } else {
            buckets[bucket].hashFunction = hashFunction;
        }
    }
}
