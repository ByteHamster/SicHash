#pragma once

#include "cmph.h"
#include "Contender.h"

class CmphContender : public Contender {
    public:
        cmph_t *mphf = nullptr;
        cmph_io_adapter_t *source;
        const char **data;
        CMPH_ALGO algo;
        double c;
        int b;
        std::string nameP;

        CmphContender(size_t N, double loadFactor, std::string name, CMPH_ALGO algo, double c, int b, bool minimal)
                : Contender(N, minimal ? 1.0 : loadFactor), algo(algo), c(c), b(b), nameP(name) {
            data = static_cast<const char **>(malloc(N * sizeof(char*)));
        }

        ~CmphContender() {
            if (mphf != nullptr) {
                cmph_destroy(mphf);
            }
            free(source);
            free(data);
        }

        std::string name() override {
            return std::string("cmph-" + nameP)
                    + " lf=" + std::to_string(c);
        }

        void beforeConstruction(const std::vector<std::string> &keys) override {
            std::cout << "Converting input" << std::endl;
            for (size_t i = 0; i < N; i++) {
                data[i] = keys.at(i).c_str();
            }
            source = cmph_io_vector_adapter((char **)data, N); // They even do the const cast in their readme file
        }

        void construct(const std::vector<std::string> &keys) override {
            //Create mphf
            cmph_config_t *config = cmph_config_new(source);
            cmph_config_set_algo(config, algo);
            cmph_config_set_verbosity(config, 0);
            //cmph_config_set_keys_per_bin(config, 2); // k-perfect
            cmph_config_set_graphsize(config, c);
            cmph_config_set_b(config, b);
            mphf = cmph_new(config);

            cmph_config_destroy(config);
            if (mphf == nullptr) {
                throw std::logic_error("Unable to create minimum perfect hashing function");
            }
        }

        size_t sizeBits() override {
            return 8 * cmph_packed_size(mphf);
        }

        void performQueries(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return cmph_search(mphf, key.c_str(), key.length());
            };
            doPerformQueries(keys, x);
        }

        void performTest(const std::vector<std::string> &keys) override {
            auto x = [&] (std::string &key) {
                return cmph_search(mphf, key.c_str(), key.length());
            };
            doPerformTest(keys, x);
        }
};

void cmphContenderRunner(size_t N, double loadFactor) {
    for (int b = 1; b < 8; b++) {
        {CmphContender(N, loadFactor, "CHD", CMPH_CHD_PH, loadFactor, b, false).run();} // b=keys_per_bucket
        {CmphContender(N, loadFactor, "CHD", CMPH_CHD, loadFactor, b, true).run();} // b=keys_per_bucket
        if (loadFactor <= 0.8) {CmphContender(N, loadFactor, "BDZ", CMPH_BDZ, 1.0/loadFactor, b, true).run();} // b=number of bits of k
        {CmphContender(N, loadFactor, "BRZ", CMPH_BRZ, 0, b, true).run();} // b=???
    }
    if (loadFactor <= 0.8) {CmphContender(N, loadFactor, "BDZ", CMPH_BDZ_PH, 1.0/loadFactor, 0, false).run();} // b ignored
    {CmphContender(N, loadFactor, "BMZ", CMPH_BMZ, loadFactor, 0, true).run();} // b ignored
    {CmphContender(N, loadFactor, "CHM", CMPH_CHM, loadFactor, 0, true).run();} // b ignored
    {CmphContender(N, loadFactor, "FCH", CMPH_FCH, loadFactor, 0, true).run();} // b ignored // Hangs
}
