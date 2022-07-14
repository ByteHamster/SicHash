#pragma once

#include <BooPHF.h>
#include <MurmurHash64.h>
#include "Contender.h"

class BBHashContender : public Contender {
    public:
        typedef boomphf::SingleHashFunctor<u_int64_t> hasher_t;
        typedef boomphf::mphf<u_int64_t, hasher_t> boophf_t;
        boophf_t * bbhash = nullptr;
        std::vector<u_int64_t> mhcs;
        double gamma;
        double perc_elem_loaded;
        double lf;

        BBHashContender(size_t N, double loadFactor, double gamma, double perc_elem_loaded)
                : Contender(N, 1.0), gamma(gamma), perc_elem_loaded(perc_elem_loaded), lf(loadFactor) {
        }

        ~BBHashContender() {
            delete bbhash;
        }

        std::string name() override {
            return std::string("BBHash")
                + " lf=" + std::to_string(lf)
                + " gamma=" + std::to_string(gamma)
                + " perc_elem_loaded=" + std::to_string(perc_elem_loaded);
        }

        void beforeConstruction(const std::vector<std::string> &keys) override {
            for (const std::string &s : keys) {
                mhcs.emplace_back(util::MurmurHash64(s));
            }
        }

        void construct(const std::vector<std::string> &keys) override {
            (void) keys;
            bbhash = new boomphf::mphf<u_int64_t,hasher_t>(mhcs.size(), mhcs,
                    /* num_thread */ 1, /* gamma */ gamma, /* writeEach */ true,
                    /* progress */ false, perc_elem_loaded);
        }

        size_t sizeBits() override {
            return bbhash->totalBitSize();
        }

        void performQueries(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return bbhash->lookup(util::MurmurHash64(key));
            };
            doPerformQueries(keys, x);
        }

        void performTest(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return bbhash->lookup(util::MurmurHash64(key));
            };
            doPerformTest(keys, x);
        }
};

void bbHashContenderRunner(size_t N, double loadFactor) {
    for (double gamma = 1.0; gamma <= 3.0; gamma += 0.15) {
        for (double perc_elem_loaded = 0.05; perc_elem_loaded < 1.0; perc_elem_loaded += 0.8) {
            BBHashContender(N, loadFactor, gamma, perc_elem_loaded).run();
        }
        BBHashContender(N, loadFactor, gamma, 0).run();
    }
}