#ifndef TESTCOMPARISON_LEAFASSIGNMENT_H
#define TESTCOMPARISON_LEAFASSIGNMENT_H

#include <random>
#include <iomanip>

template <int leafSize>
class LeafAssignment {
    public:
        inline static const char* COLOR_RED = "\033[1;31m";
        inline static const char* COLOR_NORMAL = "\033[0m";

        inline virtual int assign(std::vector<uint64_t> &keys) = 0;
        inline virtual int hash(uint64_t key, int hashfunction) = 0;
        inline virtual std::string name() = 0;

        void benchmark(std::vector<std::vector<uint64_t>> tasks, int numTests) {
            std::vector<int> hashfunctions;
            std::cout<<name()<<"\t"<<std::flush;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (int i = 0; i < numTests; i++) {
                int hashfunction = assign(tasks.at(i));
                hashfunctions.push_back(hashfunction);
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            double functionSum = 0;
            for (int i = 0; i < numTests; i++) {
                int func = hashfunctions.at(i);
                functionSum += func;
                char used[leafSize] = {0};
                for (int k = 0; k < leafSize; k++) {
                    int location = hash(tasks.at(i).at(k), func);
                    if (used[location] != 0) {
                        std::cout << COLOR_RED <<" Error: Double hashed!" << COLOR_NORMAL << std::endl;
                        return;
                    }
                    if (location >= leafSize) {
                        std::cout << COLOR_RED << " Error: Too large!" << COLOR_NORMAL << std::endl;
                        return;
                    }
                    used[location] = 1;
                }
            }
            std::cout<<std::fixed<<std::setprecision(2)<<std::setw(8);
            std::cout<<(1e-3) * std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()/numTests<<" us/leaf"<<std::flush;
            std::cout<<", \t average function: "<<(int)(functionSum/numTests)<<std::endl;
        }
};

#endif //TESTCOMPARISON_LEAFASSIGNMENT_H
