#include <tlx/cmdline_parser.hpp>
#include <HeterogeneousCuckooHashTableTest.h>
#include "benchmark/BenchmarkData.h"

/**
 * Uses the implementation that is focused on flexibility and statistics.
 * The implementation is NOT meant for performance statistics.
 */
int main(int argc, char** argv) {
    size_t M = 3e5;
    std::string name = "";
    tlx::CmdlineParser cmd;
    cmd.add_string('l', "name", name, "Name for identifying the output");
    cmd.add_bytes('m', "numLocations", M, "Size of the hash table");
    size_t thresholds_[9] = {0};
    for (size_t i = 2; i <= 8; i++) {
        cmd.add_size_t('0' + i, "percentage" + std::to_string(i), thresholds_[i], "Percentage of items to have this number of hash functions");
    }
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    std::vector<std::pair<uint64_t, uint8_t>> thresholds;
    size_t thresholdSum = 0;
    for (size_t i = 2; i <= 8; i++) {
        thresholdSum += thresholds_[i];
        if (thresholdSum > 100) {
            std::cerr<<"Thresholds are more than 100%"<<std::endl;
            return 1;
        }
        thresholds.emplace_back(UINT64_MAX / 100 * thresholdSum, i);
    }
    std::vector<std::string> keys = generateInputData(M);
    HeterogeneousCuckooHashTableTest hashTable(M, thresholds, M);
    size_t N = 0;
    while (hashTable.insert(HashedKey(keys[N])) && N < M) {
        N++;
        if ((N % (M/42)) == 0 && N >= 0.7 * M) { // 0.3*42=12 steps displayed
            std::cout<<"\rInserting: "<<100l*N/M<<"%"<<std::flush;
        }
    }
    std::cout<<std::endl;
    std::cout << "RESULT";
    if (!name.empty()) {
        std::cout << " name=" << name;
    }
    std::cout << " loadFactor=" << (double)N / (double)M
              << " M=" << M;
    for (size_t i = 2; i <= 8; i++) {
        std::cout << " percentage" << i << "=" << thresholds_[i];
    }
    std::cout << std::endl;
    return 0;
}
