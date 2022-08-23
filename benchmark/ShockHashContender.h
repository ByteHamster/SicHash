#pragma once

#include <ShockHash.h>
#include "Contender.h"

template<size_t bucketSize, size_t expectedSeed>
class ShockHashContender : public Contender {
    public:
        sichash::ShockHash<bucketSize, expectedSeed> *perfectHashing = nullptr;

        ShockHashContender(size_t N) : Contender(N, 1.0) {
        }

        ~ShockHashContender() {
            delete perfectHashing;
        }

        std::string name() override {
            return std::string("ShockHash")
                   + " bucketSize=" + std::to_string(bucketSize);
        }

        void construct(const std::vector<std::string> &keys) override {
            perfectHashing = new sichash::ShockHash<bucketSize, expectedSeed>(keys);
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

void shockHashContenderRunner(size_t N) {
    {ShockHashContender<15, 75>(N).run();}
    {ShockHashContender<17, 150>(N).run();}
    {ShockHashContender<20, 400>(N).run();}
    {ShockHashContender<23, 1200>(N).run();}
    {ShockHashContender<25, 2100>(N).run();}
    {ShockHashContender<28, 6000>(N).run();}
}
