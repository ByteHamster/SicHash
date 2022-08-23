#pragma once

#include <ShockHash.h>
#include "Contender.h"

template<size_t bucketSize, size_t expectedSeed>
class ShockHashContender : public Contender {
    public:
        sichash::ShockHash<bucketSize, expectedSeed> *perfectHashing = nullptr;

        ShockHashContender(size_t N, float loadFactor) : Contender(N, loadFactor) {
        }

        ~ShockHashContender() {
            delete perfectHashing;
        }

        std::string name() override {
            return std::string("ShockHash")
                   + " bucketSize=" + std::to_string(bucketSize);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new sichash::ShockHash<bucketSize, expectedSeed>(keys, loadFactor);
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

void shockHashContenderRunner(size_t N, float loadFactor) {
    {ShockHashContender<15, 10>(N, loadFactor).run();}
    {ShockHashContender<17, 13>(N, loadFactor).run();}
    {ShockHashContender<20, 21>(N, loadFactor).run();}
    {ShockHashContender<23, 32>(N, loadFactor).run();}
    {ShockHashContender<25, 43>(N, loadFactor).run();}
    {ShockHashContender<28, 67>(N, loadFactor).run();}
}
