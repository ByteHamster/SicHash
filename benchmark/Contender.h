#pragma once

#include <random>
#include <iostream>
#include <chrono>
#include <XorShift64.h>
#include <unistd.h>
#include "BenchmarkData.h"

#define DO_NOT_OPTIMIZE(value) asm volatile ("" : : "r,m"(value) : "memory")

class Contender {
    public:
        const size_t N;
        const double loadFactor;
        const double mByN;
        const size_t M;
        static size_t numQueries;

        Contender(size_t N, double loadFactor)
                : N(N), loadFactor(loadFactor), mByN(1.0 / loadFactor), M(N * mByN) {
        }

        virtual std::string name() = 0;
        virtual size_t sizeBits() = 0;
        virtual void construct(const std::vector<std::string> &keys) = 0;

        virtual void beforeConstruction(const std::vector<std::string> &keys) {
            (void) keys;
        }
        virtual void performQueries(const std::vector<std::string> &keys) = 0;
        virtual void performTest(const std::vector<std::string> &keys) = 0;

        void run() {
            std::cout << name() << std::endl;
            std::vector<std::string> keys = generateInputData(N);
            beforeConstruction(keys);

            std::cout << "Cooldown" << std::endl;
            usleep(1000*1000);
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
            std::cout<<"Preparing query plan"<<std::endl;
            std::vector<std::string> queryPlan;
            queryPlan.reserve(numQueries);
            util::XorShift64 prng(time(nullptr));
            for (size_t i = 0; i < numQueries; i++) {
                queryPlan.push_back(keys[prng(N)]);
            }
            std::cout << "Cooldown" << std::endl;
            usleep(1000*1000);
            std::cout<<"Querying"<<std::endl;
            begin = std::chrono::steady_clock::now();
            performQueries(queryPlan);
            end = std::chrono::steady_clock::now();
            long queryTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            printResult((double) sizeBits() / N, constructionTime, queryTime, numQueries);
        }

        void printResult(double bitsPerElement, long constructionTimeMilliseconds,
                         long queryTimeMilliseconds, size_t numQueries) {
            std::cout << "RESULT"
                      << " name=" << name()
                      << " bitsPerElement=" << bitsPerElement
                      << " constructionTimeMilliseconds=" << constructionTimeMilliseconds
                      << " queryTimeMilliseconds=" << queryTimeMilliseconds
                      << " numQueries=" << numQueries
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
            double eps = 1.0001; // Rounding with load factor variables
            std::vector<bool> taken(M * eps);
            for (size_t i = 0; i < keys.size(); i++) {
                size_t retrieved = hashFunction(const_cast<std::string &>(keys[i]));
                // Some contenders expect non-const keys but actually use them as const.
                if (retrieved > M * eps) {
                    std::cout << "Error: Range wrong. Hash function returned " << retrieved << std::endl;
                    return;
                }
                if (taken[retrieved]) {
                    std::cout<<"Error: Collision: Key #"<<i<<"/"<<N<<" resulted in "<<retrieved<<": "<<keys[i]<<std::endl;
                    std::cout<<"Aborting query"<<std::endl;
                    return;
                }
                taken[retrieved] = true;
            }
        }
};
size_t Contender::numQueries = 5e7;
