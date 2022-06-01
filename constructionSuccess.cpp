#include "util/HeterogeneousCuckooHashTableTest.h"
#include "util/Util.h"
#include "util/benchmark_data.h"
#include <tlx/cmdline_parser.hpp>

int main(int argc, char** argv) {
    size_t iterations = 10;
    size_t N = 1e5;
    size_t M = 3e5;
    std::string name = "?";
    tlx::CmdlineParser cmd;
    cmd.add_string('l', "name", name, "Name for identifying the output");
    cmd.add_bytes('i', "iterations", iterations, "Number of times to try construction");
    cmd.add_bytes('n', "numKeys", N, "Number of keys to store");
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
    std::vector<std::string> keys = generateInputData(N);
    HeterogeneousCuckooHashTableTest hashTable(thresholds, N);
    for (size_t i = 0; i < N; i++) {
        hashTable.prepare(HashedKey(keys[i]));
    }
    size_t successfulSeeds = 0;
    for (size_t seed = 0; seed < iterations; seed++) {
        if (hashTable.construct(M, seed)) {
            successfulSeeds++;
        }
    }
    std::cout << "RESULT"
              << " name=" << name
              << " N=" << N
              << " M=" << M
              << " success=" << successfulSeeds;
    for (size_t i = 2; i <= 8; i++) {
        std::cout << " percentage" << i << "=" << thresholds_[i];
    }
    std::cout << std::endl;
    return 0;
}
