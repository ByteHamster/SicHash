#include <chrono>
#include <tlx/cmdline_parser.hpp>
#include "util/HeterogeneousCuckooPerfectHashing.h"
#include "util/benchmark_data.h"

int main(int argc, char** argv) {
    HeterogeneousPerfectHashingConfig config;
    size_t N = 1e7;
    int t1 = 50;
    int t2 = 10;

    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Total number of keys to use");
    cmd.add_double('l', "loadFactor", config.loadFactor, "Load factor of the table, usually between 0.8 and 0.99");
    cmd.add_int('1', "percentage2", t1, "Threshold for objects with 2 choices");
    cmd.add_int('2', "percentage4", t2, "Threshold for objects with 4 choices");
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    config.thresholdsPercentage(t1, t2);
    std::vector<std::string> keys = generateInputData(N);

    size_t spaceUsage;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    HeterogeneousCuckooPerfectHashing perfectHashing(keys, config);
    spaceUsage = perfectHashing.spaceUsage();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::cout << "RESULT"
              << " loadFactor=" << config.loadFactor
              << " N=" << N
              << " t1=" << config.class1Percentage()
              << " t2=" << config.class2Percentage()
              << " spaceUsage=" << (double) spaceUsage / keys.size()
              << " constructionTimeMillis=" << constructionTime
              << std::endl;
    return 0;
}
