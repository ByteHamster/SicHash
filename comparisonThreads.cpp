#include <tlx/cmdline_parser.hpp>
#include <thread>
#include "benchmark/CmphContender.h"
#include "benchmark/SicHashContender.h"
#include "benchmark/PTHashContender.h"
#include "benchmark/RecSplitContender.h"

size_t numThreads = 1;
size_t objectsPerThread = 5e6;
/**
 * Simulation for multi-threading. Constructs independent hash functions in each thread.
 */
void runMultiThread(const std::function<Contender*()>& generator) {
    std::vector<std::thread> threadPool;
    std::vector<Contender*> contenders(numThreads);
    std::vector<std::vector<std::string>> inputData(numThreads);
    for (size_t i = 0; i < numThreads; i++) {
        threadPool.emplace_back([&contenders, &inputData, i, &generator] () {
            inputData.at(i) = generateInputData(objectsPerThread);
            contenders.at(i) = generator();
            contenders.at(i)->beforeConstruction(inputData.at(i));
        });
    }
    for (std::thread &thread : threadPool) {
        thread.join();
    }
    threadPool.clear();
    std::cout << "Cooldown" << std::endl;
    usleep(5000*1000);

    std::cout << "Constructing in parallel" << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (size_t i = 0; i < numThreads; i++) {
        threadPool.emplace_back([&contenders, &inputData, i] () {
            contenders.at(i)->construct(inputData.at(i));
        });
    }
    for (std::thread &thread : threadPool) {
        thread.join();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "RESULT"
              << " name=" << contenders.at(0)->name()
              << " constructionTimeMilliseconds=" << constructionTime
              << " objectsPerThread=" << objectsPerThread
              << " threads=" << numThreads
              << std::endl;
    for (size_t i = 0; i < numThreads; i++) {
        delete contenders.at(i);
    }
}

int main(int argc, char** argv) {
    size_t iterations = 1;
    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", objectsPerThread, "Number of objects per thread");
    cmd.add_bytes('i', "iterations", iterations, "Number of iterations to execute");
    cmd.add_bytes('t', "numThreads", numThreads, "Number of threads to use for construction. "
                             "This is an emulation only. Constructs independent hash functions in each thread.");

    if (!cmd.process(argc, argv)) {
        return 1;
    }

    Contender::numQueries = 0;
    for (size_t i = 0; i < iterations; i++) {
        runMultiThread([&] () {
            return new PTHashContender<false, pthash::elias_fano>(objectsPerThread, 0.95, 3.95); });
        runMultiThread([&] () {
            return new PTHashContender<true, pthash::elias_fano>(objectsPerThread, 0.95, 3.95); });
        runMultiThread([&] () {
            return new SicHashContender<false, 64>(objectsPerThread, 0.95, 46, 32); });
        runMultiThread([&] () {
            return new SicHashContender<true, 64, 4>(objectsPerThread, 0.95, 46, 32); });
        runMultiThread([&] () {
            return new RecSplitContender<4>(objectsPerThread, 100); });
        runMultiThread([&] () {
            return new CmphContender(objectsPerThread, 0.95, "CHD", CMPH_CHD_PH, 0.95, 5, false); });

        // The built-in multi-thread solution of BBHash does not work with our simulation
        // because then every thread locks the same file
        // runMultiThread([&] () {
        //    return new BBHashContender(objectsPerThread, 2.3, 0); });
    }
    return 0;
}
