#pragma once

#include <SicHash.h>
#include "Contender.h"

template<bool minimal, size_t ribbonWidth>
class SicHashContender : public Contender {
    public:
        sichash::SicHash<minimal, ribbonWidth> *perfectHashing = nullptr;
        sichash::SicHashConfig config;

        SicHashContender(size_t N, double loadFactor, int threshold1, int threshold2)
                : Contender(N, minimal ? 1.0 : loadFactor) {
            config.thresholdsPercentage(threshold1, threshold2);
            config.loadFactor = loadFactor;
        }

        ~SicHashContender() {
            delete perfectHashing;
        }

        std::string name() override {
            return std::string("SicHash")
                   + " minimal=" + std::to_string(minimal)
                   + " lf=" + std::to_string(config.loadFactor) // Internal load factor
                   + " t1=" + std::to_string(config.class1Percentage())
                   + " t2=" + std::to_string(config.class2Percentage())
                   + " bucketSize=" + std::to_string(config.smallTableSize);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new sichash::SicHash<minimal, ribbonWidth>(keys, config);
        }

        size_t sizeBits() override {
            return perfectHashing->spaceUsage();
        }

        void performQueries(const std::vector<std::string> &keys) override {
            doPerformQueries(keys, *perfectHashing);
        }

        void performTest(const std::vector<std::string> &keys) override {
            doPerformTest(keys, *perfectHashing);
        }
};

void sicHashContenderRunner(size_t N, double loadFactor) {
    int jStepSize;
    int iStepSize;
    for (int i = 25; i <= 70; i += iStepSize) {
        iStepSize = i > 60 ? 2 : 4;
        jStepSize = i > 55 ? 3 : 8;
        for (int j = 20; j <= 45 && i + j <= 100; j += jStepSize) {
            {SicHashContender<false, 32>(N, loadFactor, i, j).run();}
            {SicHashContender<false, 64>(N, loadFactor, i, j).run();}
            {SicHashContender<true, 32>(N, loadFactor, i, j).run();}
            {SicHashContender<true, 64>(N, loadFactor, i, j).run();}
        }
    }
}
