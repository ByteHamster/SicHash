#include <chrono>
#include <sichash/IrregularCuckooHashTable.h>
#include <sichash/IrregularCuckooHashTableHK.h>
#include "BenchmarkData.h"

template <typename HashTable>
void plotConstructionPerformanceByLoadFactor() {
    size_t M = 5000;
    std::vector<std::string> keys = generateInputData(M);
    for (double loadFactor = 0.8; loadFactor <= 0.98; loadFactor += 0.002) {
        size_t N = loadFactor * M;
        sichash::IrregularCuckooHashTableConfig config;
        config.maxEntries = N;
        config.threshold1 = UINT64_MAX / 100 * 50;
        config.threshold2 = UINT64_MAX / 100 * 75;
        HashTable hashTable(config);
        for (size_t i = 0; i < N; i++) {
            hashTable.prepare(sichash::HashedKey(keys[i]));
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
    plotConstructionPerformanceByLoadFactor<sichash::IrregularCuckooHashTable>();
    plotConstructionPerformanceByLoadFactor<sichash::HopcroftKarpMatchingCuckooHashTable>();
    return 0;
}
