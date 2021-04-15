#ifndef TESTCOMPARISON_HDC_H
#define TESTCOMPARISON_HDC_H

#include <cstring>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include <map>
#include "../util/Hash.h"
extern "C" {
#include "../cmph/src/compressed_seq.h"
}

#define CHD_MAX_BUCKET_SIZE 25
struct Bucket {
    uint64_t elementsMhc[CHD_MAX_BUCKET_SIZE] = {};
    int hashFunction = 0;
    int size = 0;
};

#define HASH_FUNCTION_BUCKET (-42)
#define HASH_FUNCTION_BUMP (INT32_MAX - 1)
#define BUCKET_SLIDING_SIZE 512
// For a bucket size of 5 elements, the quality starts to drop below a window size of 125 (depending on fill percentage)

// Assign each bucket a hash function like in
// "Hash, displace, and compress" (Belazzougui, Botelho, Dietzfelbinger)
class Hdc {
    public:
        Bucket *buckets;
        uint8_t *placed;
        const int M;
        const int N;
        const int BucketSize;
        const int K;
        const int NumBuckets;
        const int NumHashFunctions;
        std::map<int, int> hashFunctionUsageCounter;
        compressed_seq_t *compressed;

        Hdc (int M, int N, int BucketSize, int NumHashFunctions, int K);
        ~Hdc();

        void insertRandomStrings();
        void insert(std::string &string);
        void printPlacedPercentage();

        void printHeader();
        void print();
        void sort(std::vector<int> &insertionOrder);
        size_t compress();

        inline bool tryPlace(int &i, const int bucket, const int hashFunction, const int start, const int size) const {
            const int bucketSize = buckets[bucket].size;
            for (; i < bucketSize; i++) {
                const size_t hash = Hash::hash(buckets[bucket].elementsMhc[i], hashFunction, size);
                uint8_t numElements = placed[start + hash];
                numElements++;
                if (numElements > K) {
                    i--;
                    return false;
                }
                placed[start + hash] = numElements;
            }
            return true;
        }

        inline void undoPlace(int &i, const int bucket, const int hashFunction, const int start, const int size) const {
            for (; i >= 0; i--) {
                size_t hash = Hash::hash(buckets[bucket].elementsMhc[i], hashFunction, size);
                placed[start + hash]--;
            }
        }

        inline bool place(int bucket, int hashFunction, int start, int size) {
            int i = 0;
            bool success = tryPlace(i, bucket, hashFunction, start, size);

            if (success) {
                hashFunctionUsageCounter[hashFunction]++;
                return true;
            } else {
                undoPlace(i, bucket, hashFunction, start, size);
                return false;
            }
        }
};

#endif //TESTCOMPARISON_HDC_H
