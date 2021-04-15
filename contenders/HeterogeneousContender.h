#pragma once

#include "Contender.h"
#include "../util/HeterogeneousCuckooPerfectHashing.h"

template<size_t ribbonWidth>
class HeterogeneousContender : public Contender {
    public:
        HeterogeneousCuckooPerfectHashing<ribbonWidth> *perfectHashing = nullptr;
        int threshold1;
        int threshold2;

        HeterogeneousContender(size_t N, double loadFactor, int threshold1, int threshold2)
                : Contender(N, loadFactor), threshold1(threshold1), threshold2(threshold2) {
        }

        ~HeterogeneousContender() {
            delete perfectHashing;
        }

        std::string name() override {
            return "Heterogeneous t1=" + std::to_string(threshold1) + " t2=" + std::to_string(threshold2);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new HeterogeneousCuckooPerfectHashing<ribbonWidth>(keys, loadFactor, threshold1, threshold2);
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
            {HeterogeneousContender<32>(N, loadFactor, i, j).run();}
            {HeterogeneousContender<64>(N, loadFactor, i, j).run();}
        }
    }
}
