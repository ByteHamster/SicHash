#pragma once

#include <SicHash.h>
#include "Contender.h"

template<bool minimal, size_t ribbonWidth, int minimalFanoLowerBits = 3>
class SicHashContender : public Contender {
    public:
        sichash::SicHash<minimal, ribbonWidth, minimalFanoLowerBits> *perfectHashing = nullptr;
        sichash::SicHashConfig config;

        SicHashContender(size_t N, double loadFactor, sichash::SicHashConfig config)
                : Contender(N, minimal ? 1.0 : loadFactor), config(config) {
            this->config.loadFactor = loadFactor;
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
                   + (config.x >= 0 ? " spaceBudgetX=" + std::to_string(config.x) : "")
                   + " ribbonWidth=" + std::to_string(ribbonWidth)
                   + " minimalFanoLowerBits=" + std::to_string(minimalFanoLowerBits)
                   + " bucketSize=" + std::to_string(config.smallTableSize);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new sichash::SicHash<minimal, ribbonWidth, minimalFanoLowerBits>(keys, config);
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

template <size_t ribbonWidth>
void sicHashContenderRunner(size_t N, double loadFactor) {
    for (float spaceBudget = 1.35; spaceBudget < 3.0; spaceBudget += 0.03) {
        for (float x = 0.0; x < 1.0; x += 0.2) {
            {SicHashContender<false, ribbonWidth>(N, loadFactor, sichash::SicHashConfig().spaceBudget(spaceBudget, x)).run();}

            if (loadFactor < 0.89) {
                {SicHashContender<true, ribbonWidth, 3>(N, loadFactor, sichash::SicHashConfig().spaceBudget(spaceBudget, x)).run();}
            } else if (loadFactor < 0.94) {
                {SicHashContender<true, ribbonWidth, 4>(N, loadFactor, sichash::SicHashConfig().spaceBudget(spaceBudget, x)).run();}
            } else {
                {SicHashContender<true, ribbonWidth, 5>(N, loadFactor, sichash::SicHashConfig().spaceBudget(spaceBudget, x)).run();}
            }
        }
    }
}
