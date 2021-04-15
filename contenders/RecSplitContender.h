#pragma once

#include "sux/function/RecSplit.hpp"
#include "Contender.h"

template<int l>
class RecSplitContender : public Contender {
    public:
        size_t bucketSize;
        sux::function::RecSplit<l> *recSplit = nullptr;

        RecSplitContender(size_t N, double loadFactor, size_t bucketSize)
                : Contender(N, loadFactor), bucketSize(bucketSize) {
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
void recSplitTestMulti(size_t N, double loadFactor) {
    {RecSplitContender<l>(N, loadFactor, 50).run();}
    {RecSplitContender<l>(N, loadFactor, 100).run();}
    {RecSplitContender<l>(N, loadFactor, 250).run();}
    {RecSplitContender<l>(N, loadFactor, 500).run();}
    {RecSplitContender<l>(N, loadFactor, 750).run();}
    {RecSplitContender<l>(N, loadFactor, 1000).run();}
    {RecSplitContender<l>(N, loadFactor, 1500).run();}
    {RecSplitContender<l>(N, loadFactor, 2000).run();}
}

void recSplitContenderRunner(size_t N, double loadFactor) {
    recSplitTestMulti<5>(N, loadFactor);
    recSplitTestMulti<6>(N, loadFactor);
    recSplitTestMulti<7>(N, loadFactor);
    recSplitTestMulti<8>(N, loadFactor);
}
