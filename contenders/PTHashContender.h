#pragma once

#include "../extlib/pthash/include/pthash.hpp"
#include "Contender.h"

template <bool minimal>
class PTHashContender : public Contender {
    public:
        size_t c;
        double internalLoadFactor;
        pthash::single_phf<pthash::murmurhash2_64, pthash::elias_fano, minimal> pthashFunction;

        PTHashContender(size_t N, double loadFactor, size_t c)
                : Contender(N, minimal ? 1.0 : loadFactor), c(c), internalLoadFactor(loadFactor) {
        }

        std::string name() override {
            return "PTHash c=" + std::to_string(c) + " lf=" + std::to_string(internalLoadFactor) + " minimal=" + std::to_string(minimal);
        }

        void beforeConstruction(const std::vector<std::string> &keys) override {
        }

        void construct(const std::vector<std::string> &keys) override {
            pthash::build_configuration config;
            config.c = c;
            config.alpha = internalLoadFactor;
            config.num_threads = 1;
            config.minimal_output = minimal;
            config.verbose_output = false;
            pthashFunction.build_in_internal_memory(keys.begin(), keys.size(), config);
        }

        size_t sizeBits() override {
            return pthashFunction.num_bits();
        }

        void performQueries(const std::vector<std::string> &keys) override {
            doPerformQueries(keys, pthashFunction); // SIGILL (Illegal instruction) on some devices
        }

        void performTest(const std::vector<std::string> &keys) override {
            doPerformTest(keys, pthashFunction); // SIGILL (Illegal instruction) on some devices
        }
};

void ptHashContenderRunner(size_t N, double loadFactor) {
    for (double c = 3.0; c < 8.1; c += 0.5) {
        PTHashContender<false>(N, loadFactor, c).run();
    }
    for (double c = 3.0; c < 8.1; c += 0.5) {
        for (double loadFactorBeforeCompact = 0.6; loadFactorBeforeCompact <= 0.9; loadFactorBeforeCompact += 0.05) {
            PTHashContender<true>(N, loadFactorBeforeCompact, c).run();
        }
    }
}
