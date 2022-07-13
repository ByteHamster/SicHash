#include <HeterogeneousCuckooPerfectHashing.h>
#include "benchmark/Contender.h"

template <bool minimal>
void performTest(size_t N, double loadFactor, int threshold1, int threshold2) {
    std::cout<<"Testing N="<<N<<", loadFactor="<<loadFactor<<", minimal="<<minimal
                <<", t1="<<threshold1<<", t2="<<threshold2<<std::endl;
    HeterogeneousPerfectHashingConfig config;
    config.thresholdsPercentage(threshold1, threshold2);
    config.loadFactor = loadFactor;
    std::cout.setstate(std::ios_base::failbit); // Suppress output
    std::vector<std::string> keys = Contender::generateInputData(N);
    HeterogeneousCuckooPerfectHashing<minimal> perfectHashing(keys, config);
    std::cout.clear();
    size_t M = minimal ? N : (N / loadFactor);
    std::vector<bool> taken(M);
    for (size_t i = 0; i < keys.size(); i++) {
        size_t retrieved = perfectHashing(keys[i]);
        if (retrieved >= M) {
            std::cerr << "Error: Range wrong. Hash function returned " << retrieved << std::endl;
            exit(1);
        }
        if (taken[retrieved]) {
            std::cerr<<"Failed: Key #"<<i<<"/"<<N<<": "<<keys[i]<<std::endl;
            exit(1);
        }
        taken[retrieved] = true;
    }
}

int main() {
    for (size_t N = 1e4; N < 2e7; N *= 3) {
        for (double loadFactor = 0.8; loadFactor <= 0.95; loadFactor += 0.05) {
            for (int i = 35; i <= 50; i += 5) {
                for (int j = 15; j <= 25; j += 5) {
                    performTest<true>(N, loadFactor, i, j);
                    performTest<false>(N, loadFactor, i, j);
                }
            }
        }
    }
    return 0;
}
