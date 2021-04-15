#include "Hdc.h"

Hdc::Hdc(int M, int N, int BucketSize, int NumHashFunctions, int K)
        : M(M), N(N), BucketSize(BucketSize), K(K), NumBuckets(K*N / BucketSize), NumHashFunctions(NumHashFunctions) {
    placed = new uint8_t[M + BUCKET_SLIDING_SIZE];
    memset(placed, 0, (M + BUCKET_SLIDING_SIZE) * sizeof(uint8_t));
    buckets = new Bucket[NumBuckets];

    for (int i = HASH_FUNCTION_BUMP; i < NumHashFunctions; i++) {
        hashFunctionUsageCounter.insert({i, 0});
    }
}

Hdc::~Hdc() {
    delete [] placed;
    delete [] buckets;
}

void Hdc::insert(std::string &string) {
    uint64_t mhc = Hash::hash(string, HASH_FUNCTION_MHC, INT64_MAX);
    size_t bucket = Hash::hash(mhc, HASH_FUNCTION_BUCKET, NumBuckets);
    assert(buckets[bucket].size + 1 < CHD_MAX_BUCKET_SIZE);
    int size = buckets[bucket].size;
    buckets[bucket].elementsMhc[size] = mhc;
    buckets[bucket].size = size + 1;
}

void Hdc::insertRandomStrings() {
    for (int i = 0; i < N * K; i++) {
        std::string randomString = "El" + std::to_string(rand());
        insert(randomString);
    }
}

void Hdc::sort(std::vector<int> &bucketInsertionOrder) {
    // Insert buckets in order of descending size. Sort with radix sort.
    /*std::sort(bucketInsertionOrder.begin(), bucketInsertionOrder.end(), [&](int lhs, int rhs) {
        return buckets[lhs].elements.size() > buckets[rhs].elements.size();
    });*/
    std::vector<std::vector<int>> radix(BucketSize * BucketSize * BucketSize); // This is way too much space but let's be sure
    for (int i = 0; i < NumBuckets; i++) {
        radix[buckets[i].size].push_back(i);
    }
    auto out = bucketInsertionOrder.begin();
    for (int i = radix.size() - 1; i >= 0; i--) {
        out = std::copy(radix[i].begin(), radix[i].end(), out);
    }
}

void Hdc::printPlacedPercentage() {
    int placedElems = 0;
    for (int i = 0; i < M + BUCKET_SLIDING_SIZE; i++) {
        placedElems += placed[i];
    }
    int bitsPerBucket = ceil(log2(NumHashFunctions + 1)); // +1 bump
    std::cout<<"Placed: "<<placedElems<<" "<<(100*placedElems/(N*K))<<"%"<<std::endl;
    std::cout<<"Bits per bucket: "<<bitsPerBucket<<std::endl;
    std::cout<<"Bits per placed item: "<<((float)(bitsPerBucket*NumBuckets)/placedElems)<<std::endl;
    std::cout<<"Target domain usage: "<<((100.0*placedElems)/(M*K))<<"%"<<std::endl;

    double entropy = 0;
    for (auto const& entry: hashFunctionUsageCounter) {
        if (entry.second == 0) {
            continue;
        }
        double probability = (double) entry.second / (double) NumBuckets;
        entropy -= probability * log2(probability);
    }
    std::cout<<"Entropy (within buckets): "<<entropy<<std::endl;
    std::cout<<"Divided by elements: "<<(entropy*NumBuckets)/placedElems<<std::endl;
}

void Hdc::printHeader() {
    std::cout<<"ID;Bucket;Elements;Hash"<<std::endl;
}

void Hdc::print() {
    for (int bucket = 0; bucket < NumBuckets; bucket++) {
        int hash = buckets[bucket].hashFunction;
        std::cout<<round((float)N/M * 100)/100;
        std::cout<<"; \t"<<bucket<<";   \t"<<buckets[bucket].size
                 <<";  \t"<<hash<<std::endl;
    }
}

size_t Hdc::compress() {
    cmph_uint32 *bucketHashes = (cmph_uint32 *) calloc(1, sizeof(cmph_uint32) * NumBuckets);
    for (int i = 0; i < NumBuckets; i++) {
        bucketHashes[i] = buckets[i].hashFunction;
    }
    compressed = (compressed_seq_t *) calloc(1, sizeof(compressed_seq_t));
    compressed_seq_init(compressed);
    cmph_uint32 nBuckets =(cmph_uint32) NumBuckets;
    compressed_seq_generate(compressed, bucketHashes, nBuckets);
    uint32_t spaceUsage = compressed_seq_get_space_usage(compressed);
    return spaceUsage;
}

