#include <chrono>
#include <tlx/cmdline_parser.hpp>
#include "util/HeterogeneousCuckooPerfectHashing.h"
#include "util/benchmark_data.h"

int main(int argc, char** argv) {
    HeterogeneousPerfectHashingConfig config;
    size_t numReps = 1;
    size_t N = 1e7;

    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Total number of keys to use");
    cmd.add_bytes('r', "repetitions", numReps, "Number of tries to execute for each configuration");
    cmd.add_bytes('b', "bucketSize", config.smallTableSize, "Size of the small buckets (cuckoo hash tables)");
    cmd.add_double('l', "loadFactor", config.loadFactor, "Load factor of the table, usually between 0.8 and 0.99");
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    std::vector<std::string> keys = generateInputData(N);

    for (int i = 50; i <= 75; i += 1) {
        for (int j = 20; j <= 45 && i + j <= 100; j += 1) {
            config.thresholdsPercentage(i, j);
            size_t spaceUsage;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            try {
                for (size_t reps = 0; reps < numReps; reps++) {
                    HeterogeneousCuckooPerfectHashing perfectHashing(keys, config);
                    spaceUsage = perfectHashing.spaceUsage();
                }
            } catch (const std::exception& e) {
                std::cout<<"Error: "<<e.what()<<std::endl;
                j = 1000; // Larger j are just harder to construct, so we can skip the inner loop here
                continue;
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "RESULT"
                      << " t1=" << config.class1Percentage()
                      << " t2=" << config.class2Percentage()
                      << " bucketSize=" << config.smallTableSize
                      << " spaceUsage=" << (double) spaceUsage / keys.size()
                      << " constructionTimeMillis=" << constructionTime / numReps
                      << std::endl;
        }
    }

    return 0;
}
