#pragma once

extern "C" {
#include <mphf.h>
}
#include "Contender.h"

class MphfWbpmContender : public Contender {
    public:
        MPHFQuerier *mphfq;
        MPHFParameters sParams;

        MphfWbpmContender(size_t N, MPHFParameters sParams)
                : Contender(N, 1.0), sParams(sParams) {
        }

        ~MphfWbpmContender() {
            MPHFQuerierFree(mphfq);
        }

        std::string name() override {
            return std::string("MphfWbpm")
                + " blockSize=" + std::to_string(sParams.nEltsPerBlock)
                + " litsPerRow=" + std::to_string(sParams.xsfp.nLitsPerRow)
                + " efficiency=" + std::to_string(sParams.xsfp.fEfficiency);
        }

        void construct(const std::vector<std::string> &keys) override {
            MPHFBuilder *mphfb = MPHFBuilderAlloc(N);
            for (const std::string &key : keys) {
                if(MPHFBuilderAddElement(mphfb, key.data(), key.length()) != 0) {
                    fprintf(stderr, "Element insertion failed...exiting\n");
                    return;
                }
            }
            mphfq = MPHFBuilderFinalize(mphfb,sParams, 1);
            MPHFBuilderFree(mphfb);
        }

        size_t sizeBits() override {
            return MPHFSize(mphfq);
        }

        void performQueries(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return MPHFQuery(mphfq, key.data(), key.length());
            };
            doPerformQueries(keys, x);
        }

        void performTest(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return MPHFQuery(mphfq, key.data(), key.length());
            };
            doPerformTest(keys, x);
        }
};

void mphfWbpmContenderRunner(size_t N) {
    {MphfWbpmContender(N, MPHFDWPaperParameters).run();}
    {MphfWbpmContender(N, MPHFPaperParameters).run();}

    // The "fast" methods are somehow significantly slower
    //{MphfWbpmContender(N, MPHFDWFastParameters).run();}
    //{MphfWbpmContender(N, MPHFFastParameters).run();}
}