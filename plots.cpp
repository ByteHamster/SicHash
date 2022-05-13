#include <chrono>
#include "util/HeterogeneousCuckooHashTable.h"
#include "util/Util.h"

std::vector<std::string> generateInputData(size_t N) {
    std::vector<std::string> inputData;
    inputData.reserve(N);
    XorShift64 prng(time(nullptr));
    char string[200];
    for (size_t i = 0; i < N; i++) {
        if ((i % (N/7)) == 0) {
            std::cout<<"\rGenerating input: "<<100l*i/N<<"%"<<std::flush;
        }
        int length = 10 + prng((30 - 10) * 2);
        for (std::size_t k = 0; k < (length + sizeof(uint64_t))/sizeof(uint64_t); ++k) {
            ((uint64_t*) string)[k] = prng();
        }
        // Repair null bytes
        for (std::size_t k = 0; k < length; ++k) {
            if (string[k] == 0) {
                string[k] = 1 + prng(255);
            }
        }
        string[length] = 0;
        inputData.emplace_back(string, length);
    }
    std::cout<<"\rInput generation complete."<<std::endl;
    return inputData;
}

void plotConstructionSuccessByN() {
    uint64_t threshold1 = UINT64_MAX / 100 * 100;
    uint64_t threshold2 = UINT64_MAX / 100 * 100;
    std::vector<std::string> keys = generateInputData(1<<22);
    for (size_t M = (1<<12); M <= keys.size(); M *= 8) {
        for (size_t N = 0.4 * M; N <= 0.6 * M; N += 0.001 * M) {
            HeterogeneousCuckooHashTable binaryCuckooHashTable(N, threshold1, threshold2);
            for (size_t i = 0; i < N; i++) {
                binaryCuckooHashTable.prepare(HashedKey(keys[i]));
            }
            size_t successfulSeeds = 0;
            for (size_t seed = 0; seed < 40; seed++) {
                if (binaryCuckooHashTable.construct(M, seed)) {
                    successfulSeeds++;
                }
            }
            std::cout << "RESULT"
                      << " N=" << N
                      << " M=" << M
                      << " success=" << successfulSeeds
                      << std::endl;
        }
    }
}

template <typename HashTable>
void plotConstructionPerformanceByLoadFactor() {
    uint64_t threshold1 = UINT64_MAX / 100 * 50;
    uint64_t threshold2 = UINT64_MAX / 100 * 75;
    size_t M = 5000;
    std::vector<std::string> keys = generateInputData(M);
    for (double loadFactor = 0.8; loadFactor <= 0.98; loadFactor += 0.002) {
        size_t N = loadFactor * M;
        HashTable hashTable(N, threshold1, threshold2);
        for (size_t i = 0; i < N; i++) {
            hashTable.prepare(HashedKey(keys[i]));
        }
        // Rough estimate to balance time needed for each test iteration
        const size_t requiredNumberOfSuccessfulConstructions = 100000 * (1.05 - loadFactor) * (1.05 - loadFactor);
        size_t iterations = 0;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (size_t seed = 0;; seed++) {
            if (hashTable.construct(M, seed)) {
                iterations++;
            }
            if (iterations == requiredNumberOfSuccessfulConstructions) {
                break;
            }
            if (seed >= requiredNumberOfSuccessfulConstructions * 10) {
                std::cout<<"Unable to construct at this load factor."<<std::endl;
                return;
            }
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        long constructionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
        std::cout << "RESULT"
                  << " method=" << HashTable::name()
                  << " N=" << N
                  << " M=" << M
                  << " constructionTimeMicros=" << 0.001 * constructionTime / requiredNumberOfSuccessfulConstructions
                  << " totalTimeMillis=" << constructionTime / 1000000
                  << std::endl;
    }
}

int main() {
    //plotConstructionSuccessByN();
    plotConstructionPerformanceByLoadFactor<RandomWalkCuckooHashTable>();
    plotConstructionPerformanceByLoadFactor<HopcroftKarpMatchingCuckooHashTable>();
    return 0;
}
