#pragma once

#include "../extlib/BBHash/BooPHF.h"
#include "Contender.h"
#include "../util/Hash.h"

class BBHashContender : public Contender {
    public:
        typedef boomphf::SingleHashFunctor<u_int64_t> hasher_t;
        typedef boomphf::mphf<u_int64_t, hasher_t> boophf_t;
        boophf_t * bbhash = nullptr;
        std::vector<u_int64_t> mhcs;
        double gamma;
        double perc_elem_loaded;

        BBHashContender(size_t N, double loadFactor, double gamma, double perc_elem_loaded)
                : Contender(N, loadFactor), gamma(gamma), perc_elem_loaded(perc_elem_loaded) {
        }

        ~BBHashContender() {
            delete bbhash;
        }

        std::string name() override {
            return "BBHash";
        }

        void beforeConstruction(const std::vector<std::string> &keys) override {
            for (const std::string &s : keys) {
                mhcs.emplace_back(ElementHasher(s).mhc);
            }
        }

        void construct(const std::vector<std::string> &keys) override {
            bbhash = new boomphf::mphf<u_int64_t,hasher_t>(mhcs.size(), mhcs,
                    /* num_thread */ 1, /* gamma */ gamma, /* writeEach */ true,
                    /* progress */ false, perc_elem_loaded);
        }

        size_t sizeBits() override {
            return bbhash->totalBitSize();
        }

        void performQueries(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return bbhash->lookup(ElementHasher(key).mhc);
            };
            doPerformQueries(keys, x);
        }

        void performTest(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return bbhash->lookup(ElementHasher(key).mhc);
            };
            doPerformTest(keys, x);
        }
};

void bbHashContenderRunner(size_t N, double loadFactor) {
    for (double gamma = 1.0; gamma <= 2.0; gamma += 0.1) {
        for (double perc_elem_loaded = 0.05; perc_elem_loaded < 1.0; perc_elem_loaded += 0.3) {
            BBHashContender(N, loadFactor, gamma, perc_elem_loaded).run();
        }
    }
}