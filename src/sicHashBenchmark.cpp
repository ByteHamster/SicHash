#include <chrono>
#include <tlx/cmdline_parser.hpp>
#include <bytehamster/util/XorShift64.h>
#include <SicHash.h>
#include "BenchmarkData.h"

#define DO_NOT_OPTIMIZE(value) asm volatile ("" : : "r,m"(value) : "memory")

sichash::SicHashConfig config;
size_t N = 1e7;
size_t numQueries = 0;
float t1 = 0.5;
float t2 = 0.1;
bool minimal = false;

template <class SicHashInstance>
void run() {
    config.percentages(t1, t2);
    std::vector<std::string> keys = generateInputData(N);
    std::cout << "Cooldown" << std::endl;
    usleep(1000*1000);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    SicHashInstance sicHashTable(keys, config);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    long queryTime = 0;
    if (numQueries > 0) {
        std::cout << "Checking" << std::endl;
        std::vector<bool> taken(minimal ? keys.size() : (keys.size() / sicHashTable.config.loadFactor + 100), false); // +100 for rounding
        for (std::string &key : keys) {
            size_t retrieved = sicHashTable(key);
            if (retrieved > taken.size()) {
                std::cerr << "Error: out of range" << std::endl;
                exit(1);
            } else if (taken[retrieved]) {
                std::cerr << "Error: collision" << std::endl;
                exit(1);
            }
            taken[retrieved] = true;
        }

        std::cout<<"Preparing query plan"<<std::endl;
        std::vector<std::string> queryPlan;
        queryPlan.reserve(numQueries);
        bytehamster::util::XorShift64 prng(time(nullptr));
        for (size_t i = 0; i < numQueries; i++) {
            queryPlan.push_back(keys[prng(N)]);
        }
        std::cout << "Cooldown" << std::endl;
        usleep(1000*1000);
        std::cout << "Querying" << std::endl;
        begin = std::chrono::steady_clock::now();
        for (std::string &key : queryPlan) {
            size_t retrieved = sicHashTable(key);
            DO_NOT_OPTIMIZE(retrieved);
        }
        end = std::chrono::steady_clock::now();
        queryTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }

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
              << " queryTimeMillis=" << queryTime
              << " numQueries=" << numQueries
              << " minimal=" << minimal
              << std::endl;
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Total number of keys to use");
    cmd.add_double('l', "loadFactor", config.loadFactor, "Load factor of the table, usually between 0.8 and 0.99");
    cmd.add_float('1', "percentage2", t1, "Threshold for objects with 2 choices");
    cmd.add_float('2', "percentage4", t2, "Threshold for objects with 4 choices");
    cmd.add_bytes('b', "bucketSize", config.smallTableSize, "Size of the small buckets (cuckoo hash tables)");
    cmd.add_bytes('q', "numQueries", numQueries, "Number of queries");
    cmd.add_bool('m', "minimal", minimal, "Construct minimal perfect hash function");
    if (!cmd.process(argc, argv)) {
        return 1;
    }

    if (minimal) {
        run<sichash::SicHash<true>>();
    } else {
        run<sichash::SicHash<false>>();
    }

    return 0;
}
