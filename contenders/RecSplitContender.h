#pragma once

#include "sux/function/RecSplit.hpp"
#include "Contender.h"

template<int l>
class RecSplitContender : public Contender {
    public:
        size_t bucketSize;
        sux::function::RecSplit<l> *recSplit = nullptr;

        RecSplitContender(size_t N, size_t bucketSize)
                : Contender(N, 1.0), bucketSize(bucketSize) {
        }

        ~RecSplitContender() {
            delete recSplit;
        }

        std::string name() override {
            return "RecSplit l="+std::to_string(l)+" b="+std::to_string(bucketSize);
        }

        void beforeConstruction(const std::vector<std::string> &keys) override {
        }

        void construct(const std::vector<std::string> &keys) override {
            recSplit = new sux::function::RecSplit<l>(keys, bucketSize);
        }

        size_t sizeBits() override {
            return recSplit->getBits();
        }

        void performQueries(const std::vector<std::string> &keys) override {
            doPerformQueries(keys, *recSplit);
        }

        void performTest(const std::vector<std::string> &keys) override {
            doPerformTest(keys, *recSplit);
        }
};

template <int l>
void recSplitTestMulti(size_t N) {
    {RecSplitContender<l>(N, 50).run();}
    {RecSplitContender<l>(N, 100).run();}
    {RecSplitContender<l>(N, 250).run();}
    {RecSplitContender<l>(N, 500).run();}
    {RecSplitContender<l>(N, 750).run();}
    {RecSplitContender<l>(N, 1000).run();}
    {RecSplitContender<l>(N, 1500).run();}
    {RecSplitContender<l>(N, 2000).run();}
}

void recSplitContenderRunner(size_t N) {
    recSplitTestMulti<5>(N);
    recSplitTestMulti<6>(N);
    recSplitTestMulti<7>(N);
    recSplitTestMulti<8>(N);
}
