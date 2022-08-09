#include <chrono>
#include <tlx/cmdline_parser.hpp>
#include <SicHash.h>
#include "benchmark/BenchmarkData.h"

int main(int argc, char** argv) {
    sichash::SicHashConfig config;
    size_t N = 1e7;
    int t1 = 50;
    int t2 = 10;

    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Total number of keys to use");
    cmd.add_double('l', "loadFactor", config.loadFactor, "Load factor of the table, usually between 0.8 and 0.99");
    cmd.add_int('1', "percentage2", t1, "Threshold for objects with 2 choices");
    cmd.add_int('2', "percentage4", t2, "Threshold for objects with 4 choices");
    cmd.add_bytes('b', "bucketSize", config.smallTableSize, "Size of the small buckets (cuckoo hash tables)");
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    config.thresholdsPercentage(t1, t2);
    std::vector<std::string> keys = generateInputData(N);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    sichash::SicHash sicHashTable(keys, config);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "RESULT"
              << " loadFactor=" << config.loadFactor
              << " N=" << N
              << " t1=" << config.class1Percentage()
              << " t2=" << config.class2Percentage()
              << " spaceUsage=" << (double) sicHashTable.spaceUsage() / keys.size()
              << " spaceUsageTheory=" << (double) sicHashTable.spaceUsageTheory() / keys.size()
              << " bucketSize=" << config.smallTableSize
              << " averageTries=" << (double) sicHashTable.unnecessaryConstructions / sicHashTable.numSmallTables
              << " constructionTimeMillis=" << constructionTime
              << std::endl;
    return 0;
}
