#pragma once

#include <random>
#include <iostream>
#include <chrono>
#include "../util/Util.h"

#define DO_NOT_OPTIMIZE(value) asm volatile ("" : : "r,m"(value) : "memory")

class Contender {
    public:
        const size_t N;
        const double loadFactor;
        const double mByN;
        const size_t M;

        Contender(size_t N, double loadFactor)
                : N(N), loadFactor(loadFactor), mByN(1.0 / loadFactor), M(N * mByN) {
        }

        virtual std::string name() = 0;
        virtual size_t sizeBits() = 0;
        virtual void construct(const std::vector<std::string> &keys) = 0;

        virtual void beforeConstruction(const std::vector<std::string> &keys) {
        }
        virtual void performQueries(const std::vector<std::string> &keys) = 0;
        virtual void performTest(const std::vector<std::string> &keys) = 0;

        void run() {
            std::cout << name() << std::endl;
            std::vector<std::string> keys = generateInputData(N);
            beforeConstruction(keys);

            std::cout << "Constructing" << std::endl;
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            try {
                construct(keys);
            } catch (const std::exception& e) {
                std::cout<<"Error: "<<e.what()<<std::endl;
                return;
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            long constructionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            std::cout<<"Testing"<<std::endl;
            performTest(keys);
            std::cout<<"Querying"<<std::endl;
            begin = std::chrono::steady_clock::now();
            performQueries(keys);
            end = std::chrono::steady_clock::now();
            long queryTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            printResult((double) sizeBits() / N, constructionTime, queryTime);
        }

        static std::vector<std::string> generateInputData(size_t N) {
            std::vector<std::string> inputData;
            inputData.reserve(N);
            XorShift64 prng(time(nullptr));
            char string[200];
            for (size_t i = 0; i < N; i++) {
                if ((i % (N/7)) == 0) {
                    std::cout<<"\rGenerating input: "<<100l*i/N<<"%"<<std::flush;
                }
                int length = 10 + prng((30 - 10) * 2);
                for (std::size_t k = 0; k < (length + sizeof(uint64_t))/sizeof(uint64_t); ++k) {
                    ((uint64_t*) string)[k] = prng();
                }
                // Repair null bytes
                for (std::size_t k = 0; k < length; ++k) {
                    if (string[k] == 0) {
                        string[k] = 1 + prng(255);
                    }
                }
                string[length] = 0;
                inputData.emplace_back(string, length);
            }
            std::cout<<"\rInput generation complete."<<std::endl;
            return inputData;
        }

        void printResult(double bitsPerElement, long constructionTimeMilliseconds, long queryTimeMilliseconds) {
            std::cout << "RESULT"
                      << " name=" << name()
                      << " bitsPerElement=" << bitsPerElement
                      << " constructionTimeMilliseconds=" << constructionTimeMilliseconds
                      << " queryTimeMilliseconds=" << queryTimeMilliseconds
                      << " N=" << N
                      << " loadFactor=" << loadFactor
                      << std::endl;
        }

    protected:
        template<typename F>
        void doPerformQueries(const std::vector<std::string> &keys, F &hashFunction) {
            for (const std::string &key : keys) {
                size_t retrieved = hashFunction(const_cast<std::string &>(key));
                // Some contenders expect non-const keys but actually use them as const.
                DO_NOT_OPTIMIZE(retrieved);
            }
        }

        template<typename F>
        void doPerformTest(const std::vector<std::string> &keys, F &hashFunction) {
            int numWorked = 0;
            std::vector<bool> taken(M);
            for (const std::string &key : keys) {
                size_t retrieved = hashFunction(const_cast<std::string &>(key));
                // Some contenders expect non-const keys but actually use them as const.
                if (retrieved > M) {
                    std::cout << "Error: Range wrong. Hash function returned " << retrieved << std::endl;
                    return;
                }
                if (taken[retrieved]) {
                    std::cout<<"Failed: Key #"<<numWorked<<"/"<<N<<": "<<key<<std::endl;
                    std::cout<<"Aborting query"<<std::endl;
                    return;
                }
                taken[retrieved] = true;
                numWorked++;
            }
        }
};