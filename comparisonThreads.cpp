#include <tlx/cmdline_parser.hpp>
#include "benchmark/CmphContender.h"
#include "benchmark/SicHashContender.h"
#include "benchmark/PTHashContender.h"
#include "benchmark/RecSplitContender.h"
#include "benchmark/BBHashContender.h"

size_t numThreads = 1;
/**
 * Simulation for multi-threading. Constructs independent hash functions in each thread.
 */
void runMultiThread(std::function<Contender*()> generator) {
    std::vector<Contender*> contenders;
    std::vector<std::thread> threadPool;
    for (size_t i = 0; i < numThreads; i++) {
        contenders.push_back(generator());
    }
    for (size_t i = 0; i < numThreads; i++) {
        threadPool.push_back(std::thread([&] () {
            contenders.at(i)->run(false);
        }));
    }
    for (std::thread &thread : threadPool) {
        thread.join();
    }
    for (size_t i = 0; i < numThreads; i++) {
        contenders.at(i)->printResult(" threads=" + std::to_string(numThreads));
        delete contenders.at(i);
    }
}

int main(int argc, char** argv) {
    size_t N = 5e6;
    size_t iterations = 1;
    double pthashParameter = 3.7;
    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Number of objects");
    cmd.add_bytes('i', "iterations", iterations, "Number of iterations to execute");
    cmd.add_bytes('t', "numThreads", numThreads, "Number of threads to use for construction. This is an emulation only. Constructs the exact same hash function multiple times.");

    if (!cmd.process(argc, argv)) {
        return 1;
    }

    Contender::numQueries = 0;
    for (size_t i = 0; i < iterations; i++) {
        // Queries of PTHash and SicHash have quite a bit of noise in the measurements.
        // Run more queries to work around that.
        runMultiThread([&] () {
            return new PTHashContender<false, pthash::elias_fano>(N, 0.95, 3.95); });
        runMultiThread([&] () {
            return new PTHashContender<true, pthash::elias_fano>(N, 0.95, 3.95); });
        runMultiThread([&] () {
            return new SicHashContender<false, 64>(N, 0.95, 46, 32); });
        runMultiThread([&] () {
            return new SicHashContender<true, 64>(N, 0.95, 37, 44); });
        runMultiThread([&] () {
            return new RecSplitContender<4>(N, 100); });
        runMultiThread([&] () {
            return new CmphContender(N, 0.95, "CHD", CMPH_CHD_PH, 0.95, 5, false); });
        runMultiThread([&] () {
            return new BBHashContender(N, 2.3, 0); });
    }
    return 0;
}
