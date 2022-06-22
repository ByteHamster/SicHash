#pragma once

#include "Contender.h"
#include "../util/HeterogeneousCuckooPerfectHashing.h"

template<bool minimal, size_t ribbonWidth>
class HeterogeneousContender : public Contender {
    public:
        HeterogeneousCuckooPerfectHashing<minimal, ribbonWidth> *perfectHashing = nullptr;
        HeterogeneousPerfectHashingConfig config;

        HeterogeneousContender(size_t N, double loadFactor, int threshold1, int threshold2)
                : Contender(N, minimal ? 1.0 : loadFactor) {
            config.thresholdsPercentage(threshold1, threshold2);
            config.loadFactor = loadFactor;
        }

        ~HeterogeneousContender() {
            delete perfectHashing;
        }

        std::string name() override {
            return std::string("Heterogeneous") + (minimal ? "Minimal" : "")
                   + " t1=" + std::to_string(config.class1Percentage())
                   + " t2=" + std::to_string(config.class2Percentage())
                   + " bucketSize=" + std::to_string(config.smallTableSize);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new HeterogeneousCuckooPerfectHashing<minimal, ribbonWidth>(keys, config);
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

void heterogeneousContenderRunner(size_t N, double loadFactor) {
    for (int i = 25; i <= 75; i += 3) {
        for (int j = 20; j <= 45 && i + j <= 100; j += 3) {
            {HeterogeneousContender<false, 32>(N, loadFactor, i, j).run();}
            {HeterogeneousContender<false, 64>(N, loadFactor, i, j).run();}
            {HeterogeneousContender<true, 32>(N, loadFactor, i, j).run();}
            {HeterogeneousContender<true, 64>(N, loadFactor, i, j).run();}
        }
    }
}
