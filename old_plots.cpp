#include <chrono>
#include "hashAssignment/HashAssignmentGreedy.h"
#include "hashAssignment/HashAssignmentMatching.h"
#include "hdc/HdcSlidingWindow.h"
#include "hdc/HdcGreedy.h"
#include "hdc/HdcMax.h"
#include "util/GaussianElimination.h"
#include "util/VariableLengthRetrieval.h"
#include "hashAssignment/HashAssignmentThreshold.h"
#include "hashAssignment/HashAssignmentThresholdMatching.h"
#include "hashAssignment/HashAssignmentThresholdGreedy.h"
#include "hashAssignment/HashAssignmentThresholdCuckoo.h"

#define FUNCTION(X) std::pair<std::function<int(int)>,std::string>([](int x) { return X;}, #X)

void plotMatchingFunctions() {
    int seed = time(nullptr);

    srand(seed);
    const int M = (1 << 16);
    HashAssignmentGreedy greedy(M, (int)(0.98 * M));
    greedy.insertRandomStrings();
    greedy.construct();
    greedy.printHistogramHeader();
    greedy.printHistogram();
    std::cout << std::endl;


    std::pair<std::function<int(int)>, std::string> costFunctions[] = {
            FUNCTION(1),
            FUNCTION(x),
            FUNCTION(x * x),
            FUNCTION(std::min(1e6, pow(2, x))),
            FUNCTION(x < 10 ? 0 : 10000)
    };

    for (const auto &costFunction : costFunctions) {
        srand(seed);
        HashAssignmentMatching matching(M, (int)(0.98 * M), costFunction.first, costFunction.second, false);
        matching.insertRandomStrings();
        matching.construct();
        matching.printHistogram();
        std::cout << std::endl;
    }
}

void plotSlidingWindowFunctions() {
    int seed = time(nullptr);
    const int M = 1 << 14;
    {
        srand(seed);
        HdcSlidingWindow bucket(M, (int) (0.98 * M), 2, 1000, 1);
        bucket.insertRandomStrings();
        bucket.construct();
        bucket.printHeader();
        bucket.print();
    }
    {
        srand(seed);
        HdcSlidingWindow bucket(M, (int) (0.8 * M), 2, 1000, 1);
        bucket.insertRandomStrings();
        bucket.construct();
        bucket.print();
    }
    {
        srand(seed);
        HdcSlidingWindow bucket(M, (int)(0.5 * M), 2, 1000, 1);
        bucket.insertRandomStrings();
        bucket.construct();
        bucket.print();
    }
}

void plotHdcVsSliding() {
    int seed = time(nullptr);
    const int M = 1<<14;
    const int N = (int)(1.5 * M); // Overloading

    srand(seed);
    HdcSlidingWindow hdcSliding(M, N, 4, 1000, 1);
    hdcSliding.insertRandomStrings();
    hdcSliding.construct();
    hdcSliding.printPlacedPercentage();
    std::cout<<std::endl;

    srand(seed);
    HdcGreedy hdcNormal(M, N, 4, 1000, 1);
    hdcNormal.insertRandomStrings();
    hdcNormal.construct(true);
    hdcNormal.printPlacedPercentage();
    std::cout<<std::endl;
}

void plotSlidingBucketSize() {
    int seed = time(nullptr);
    const int M = 1<<14;
    const int N = (int)(1.5 * M); // Overloading

    for (int size = 2; size <= 20; size++) {
        srand(seed);
        HdcSlidingWindow hdcSliding(M, N, size, 1000, 1);
        hdcSliding.insertRandomStrings();
        hdcSliding.construct();
        hdcSliding.printPlacedPercentage();
        std::cout<<std::endl;
    }
}

void plotSlidingWindowSingleHash(int bucketSize, int K) {
    int seed = time(nullptr);
    const int M = 1<<14;
    const int N = (int)(1.5 * M); // Overloading

    srand(seed);
    HdcSlidingWindow hdcSliding(M, N, bucketSize, 1, K);
    hdcSliding.insertRandomStrings();
    hdcSliding.construct();
    hdcSliding.printPlacedPercentage();
    std::cout<<std::endl;

    srand(seed);
    HdcGreedy hdcNormal(M, N, bucketSize, 1, K);
    hdcNormal.insertRandomStrings();
    hdcNormal.construct(false);
    hdcNormal.printPlacedPercentage();
    std::cout<<std::endl;

    srand(seed);
    HdcGreedy hdcSorted(M, N, bucketSize, 1, K);
    hdcSorted.insertRandomStrings();
    hdcSorted.construct(true);
    hdcSorted.printPlacedPercentage();
}

void plotGaussianTest() {
    GaussianElimination elimination(5, 3);
    elimination.addEquation(new int[3]{2, 3, 4}, 0);
    elimination.addEquation(new int[3]{1, 2, 3}, 1);
    elimination.addEquation(new int[3]{1, 2, 4}, 0);
    elimination.addEquation(new int[3]{1, 3, 4}, 1);
    elimination.addEquation(new int[3]{0, 2, 4}, 1);
    bool success = elimination.solve();
    assert(success);
    elimination.printAssignment();
}

void plotVariableLengthRetrievalTest() {
    srand(time(nullptr));
    int savedElems = 100;
    std::map<std::string, int> check;
    VariableLengthRetrieval retrieval(165);
    for (int i = 0; i < savedElems; i++) {
        std::string key = "Elem" + std::to_string(i);
        int value = (rand() % 2) * (rand() % 3);
        std::cout << "Storing: " << key << "=" << value << std::endl;
        retrieval.insert(key, value);
        check.insert({key, value});
    }
    bool success = retrieval.construct();
    assert(success);
    for (int i = 0; i < savedElems; i++) {
        std::string key = "Elem" + std::to_string(i);
        std::cout << "Retrieving: " << key << "=" << retrieval.retrieve(key) << std::endl;
        assert(check[key] == retrieval.retrieve(key));
    }
}

void plotVariableLengthRetrievalTestK() {
    int seed = time(nullptr);

    std::cout<<"k;threshold;success"<<std::endl;
    for (int k = 2; k < 8; k++) {
        for (float threshold = 1.35; threshold < 1.8; threshold += 0.01) {
            srand(seed);

            int tries = 50;
            int successfulTries = 0;
            for (int i = 0; i < tries; i++) {
                const int savedElems = 200;
                VariableLengthRetrieval retrieval((int) (threshold * savedElems), k);
                for (int e = 0; e < savedElems; e++) {
                    retrieval.insert(std::to_string(rand()), (rand() % 2) * (rand() % 3));
                }
                if (retrieval.construct()) {
                    successfulTries++;
                }
            }
            std::cout << k << ";" << threshold << ";" << (100 * successfulTries / tries) << std::endl;
        }
    }
}

void plotPhfMatchingVariableRetrieval() {
    srand(time(nullptr));
    #ifdef REMEMBER_ELEMENT_MAPPING
    auto costFunction = FUNCTION(x*x);
    const int M = (1 << 9);
    HashAssignmentMatching matching(M, (int)(M / 1.23), costFunction.first, costFunction.second, false);
    matching.insertRandomStrings();
    std::cout<<"Constructing matching..."<<std::endl;
    matching.construct();
    std::cout<<"Inserting retrieval..."<<std::endl;
    VariableLengthRetrieval retrieval((int) (M * 1.3));
    for (const auto &x : matching.hashFunctions) {
        retrieval.insert(x.first, x.second);
    }
    std::cout<<"Constructing retrieval..."<<std::endl;
    bool success = retrieval.construct();
    std::cout<<"Constructed: "<<success<<std::endl;
    assert(success);
    #endif
}

void plotPhfMatchingVariableRetrievalSolveableT(int M, int N, int retrievalK, RetrievalCoding coding, std::string name) {
    #ifdef REMEMBER_ELEMENT_MAPPING
    for (float fraction = 1.40; fraction <= 1.7; fraction += 0.02) {
        int attempts = 30;
        int successfulAttempts = 0;
        for (int i = 0; i < attempts; i++) {
            auto costFunction = FUNCTION(x*x);
            HashAssignmentMatching matching(M, N, costFunction.first, costFunction.second, false);
            matching.insertRandomStrings();
            matching.construct();
            VariableLengthRetrieval retrieval((int) (N * fraction), retrievalK, coding);
            for (const auto &x : matching.hashFunctions) {
                retrieval.insert(x.first, x.second);
            }
            if (retrieval.construct()) {
                successfulAttempts++;
            }
        }
        std::cout<<name<<";"<<fraction<<";"<<(100 * successfulAttempts / attempts)<<std::endl;
    }
    #endif
}

void plotPhfMatchingVariableRetrievalSolveable() {
    srand(time(nullptr));
    const int M = (1 << 8);
    const int N = (int)(M / 1.23);

    std::cout<<"name;fraction;success"<<std::endl;
    plotPhfMatchingVariableRetrievalSolveableT(M, N, 4, RetrievalCodingUnary, "K=4, Unary");
    plotPhfMatchingVariableRetrievalSolveableT(M, N, 5, RetrievalCodingUnary, "K=5, Unary");
    plotPhfMatchingVariableRetrievalSolveableT(M, N, 4, RetrievalCodingCutoff4, "K=4, Cutoff");
    plotPhfMatchingVariableRetrievalSolveableT(M, N, 5, RetrievalCodingCutoff4, "K=5, Cutoff");

}

void plotMatchingFixpoint() {
    int seed = time(nullptr);
    #ifndef COUNT_HASH_FUNCTION_USAGE
        std::cout<<"Hash function usage count not enabled"<<std::endl;
        return;
    #endif

    std::map<int, int> hashFunctionUsage;
    const int M = 1 << 13;
    for (int i = 0; i < 20; i++) {
        srand(seed);
        char name[100];
        sprintf(name, "Iteration %02d", i + 1);
        HashAssignmentMatching matching(M, (int)(M * 0.98), [&](int x) {
            if (i == 0) {
                return x * x;
            } else if (hashFunctionUsage.count(x) == 1) {
                return (int) (-1000.0 * log2((double) hashFunctionUsage.at(x) / M));
            } else {
                return 100000;
            }
        }, std::string(name), false);
        matching.insertRandomStrings();
        matching.construct();
        if (i == 0) {
            matching.printHistogramHeader();
        }
        matching.printHistogram();
        std::cout << std::endl;
        hashFunctionUsage = matching.hashFunctionUsageCounter;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
void plotAssignmentThreshold() {
    srand(time(nullptr));

    std::cout<<"perc1;perc2;succ;cost"<<std::endl;
    // Some threads have nearly no work, so when using 8 threads,
    // there will be one left in the end that works alone.
    // Create more threads than processors to reduce the sequential part in the end.
    #pragma omp parallel for num_threads(20) default(none) shared(std::cout, COST_PER_CLASS)
    for (int percentage1bit = 0; percentage1bit <= 100; percentage1bit++) {
        for (int percentage2bit = 0; percentage2bit <= 100 - percentage1bit; percentage2bit++) {
            if (2 * percentage1bit + percentage2bit < 80) {
                continue; // Successful anyway
            } else if (2.8 * percentage1bit + percentage2bit > 210) {
                continue; // Fails anyway
            }
            int samplesSuccessful = 0;
            const int M = 1 << 12;
            for (int i = 0; i < 40; i++) {
                HashAssignmentThresholdMatching varMatching(M, (int)(M * 0.98),
                                new int[NUM_THRESHOLDS]{percentage1bit, percentage2bit});
                varMatching.insertRandomStrings();
                if (varMatching.construct()) {
                    samplesSuccessful++;
                }
            }
            double cost = (COST_PER_CLASS[0] * percentage1bit
                    + COST_PER_CLASS[1] * percentage2bit
                    + COST_PER_CLASS[2] * (100 - percentage1bit - percentage2bit)
                    ) / 100.0;
            std::cout<<percentage1bit<<"; "<<percentage2bit<<"; "<<samplesSuccessful<<"; "<<cost<<std::endl;
        }
    }
}
#pragma clang diagnostic pop

void plotAssignmentThresholdEntropy() {
    int seed = time(nullptr);

    srand(seed);
    const int M = 1 << 10;
    HashAssignmentThresholdMatching varMatching(M, (int)(M * 0.81), new int[NUM_THRESHOLDS]{60, 40});
    varMatching.insertRandomStrings();
    varMatching.construct();
    varMatching.printEntropy();
}

void plotAssignmentThresholdLastLayerVariable() {
    #ifdef REMEMBER_ELEMENT_MAPPING
    srand(time(nullptr));

    const int M = 1 << 11;
    const int N = (int)(M * 0.98);
    int percentage1bit = 52;
    int percentage2bit = 15;
    HashAssignmentThresholdMatching varMatching(M, N, new int[NUM_THRESHOLDS]{percentage1bit, percentage2bit});
    varMatching.insertRandomStrings();
    int cSuccess = varMatching.construct();
    std::cout<<"Constructed matching: "<<cSuccess<<std::endl;
    assert(cSuccess);

    double cost = (COST_PER_CLASS[0] * percentage1bit
                   + COST_PER_CLASS[1] * percentage2bit
                   + COST_PER_CLASS[2] * (100 - percentage1bit - percentage2bit)
                  ) / 100.0;
    std::cout<<"Normal cost: 0."<<percentage1bit<<"*1 + 0."<<percentage2bit
            <<"*2 + 0."<<(100-percentage1bit-percentage2bit)<<"*3 = "<<cost<<std::endl;

    double retrievalLayerCost = 2.92;
    VariableLengthRetrieval retrieval( (int) (retrievalLayerCost * varMatching.classes[2].size()));
    for (const std::string& element : varMatching.classes[2]) {
        int hashFunction = varMatching.hashFunctions.at(element);
        retrieval.insert(element, hashFunction);
    }
    std::cout<<"Constructing retrieval..."<<std::endl;
    bool success = retrieval.construct();
    std::cout<<"Constructed: "<<success<<std::endl;
    assert(success);

    double costCheaper = (COST_PER_CLASS[0] * percentage1bit
                   + COST_PER_CLASS[1] * percentage2bit
                   + retrievalLayerCost * (100 - percentage1bit - percentage2bit)
                  ) / 100.0;
    std::cout<<"Compressed cost: 0."<<percentage1bit<<"*1 + 0."<<percentage2bit
             <<"*2 + 0."<<(100-percentage1bit-percentage2bit)<<"*"<<retrievalLayerCost<<" = "<<costCheaper<<std::endl;
    #endif
}

void plotAssignmentThresholdCuckoo() {
    int seed = time(nullptr);

    const int M = 1 << 14;
    const int N = (int) (M * 0.98);
    int percentage1bit = 52;
    int percentage2bit = 15;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    {
        srand(seed);
        HashAssignmentThresholdCuckoo cuckoo(M, N, new int[NUM_THRESHOLDS]{percentage1bit, percentage2bit});
        cuckoo.insertRandomStrings();
        assert(cuckoo.construct());
        cuckoo.printEntropy();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout<<"==> Cuckoo: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()<<" ms"<<std::endl;
    std::cout<<std::endl;

    begin = std::chrono::steady_clock::now();
    {
        srand(seed);
        HashAssignmentThresholdMatching matching(M, N, new int[NUM_THRESHOLDS]{percentage1bit, percentage2bit});
        matching.insertRandomStrings();
        assert(matching.construct());
        matching.printEntropy();
    }
    end = std::chrono::steady_clock::now();
    std::cout<<"==> Matching: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()<<" ms"<<std::endl;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
void plotAssignmentThresholdCuckooDistribution() {
    srand(time(nullptr));
    std::cout<<"run;elementClass;classCost;percentage"<<std::endl;

    const int M = 1 << 20;
    const int N = (int) (M * 0.98);

    #pragma omp parallel for num_threads(8) default(none) shared(std::cout, COST_PER_CLASS, HASH_FUNCTIONS_PER_CLASS)
    for (int run = 0; run < 100; run++) {
        HashAssignmentThresholdCuckoo cuckoo(M, N,true);
        cuckoo.insertRandomStrings();
        assert(cuckoo.construct());

        for (int clazz = 0; clazz < NUM_THRESHOLDS; clazz++) {
            int sum = 0;
            for (int k = 0; k < HASH_FUNCTIONS_PER_CLASS[clazz]; k++) {
                sum += cuckoo.hashFunctionUsagePerBucket[clazz][k];
            }
            std::cout << run << ";" << COST_PER_CLASS[clazz] << ";" << COST_PER_CLASS[clazz] << ";" << (100.0 * sum / cuckoo.placedItems) << std::endl;
        }
        std::cout << run << ";" << "bump;3;" << (100.0 * cuckoo.bumped.size() / cuckoo.placedItems) << std::endl;
        std::cout << run << ";" << "bumpR;0;" << (100.0 * cuckoo.bumpedRemainder.size() / cuckoo.placedItems) << std::endl;
        std::cout << "# Achieved fill rate: " << (100.0 * cuckoo.placedItems / M) << std::endl;
    }
}
#pragma clang diagnostic pop

void plotAssignmentThresholdCuckooTryPercentagesLinear() {
    srand(time(nullptr));

    const int M = 1 << 11;
    const int N = (int) (M * 0.98);
    int thresholds[NUM_THRESHOLDS] = {0};
    const int delta = 1;

    thresholds[NUM_THRESHOLDS - 1] = 100;
    int lookAt = 0;
    while (lookAt < NUM_THRESHOLDS - 1) {
        int successful;
        int sumThresholds;
        do {
            thresholds[lookAt] += delta;
            thresholds[NUM_THRESHOLDS - 1] -= delta;
            successful = 0;
            sumThresholds = 0;
            double cost = 0;
            std::cout << "Thresholds:";
            for (int i = 0; i < NUM_THRESHOLDS; i++) {
                if (i == lookAt) {
                    std::cout << "   \t»" << thresholds[i];
                } else {
                    std::cout << "   \t" << thresholds[i];
                }
                sumThresholds += thresholds[i];
                cost += 0.01 * thresholds[i] * COST_PER_CLASS[i];
            }
            for (int i = 0; i < 40; i++) {
                HashAssignmentThresholdCuckoo cuckoo(M, N, thresholds);
                cuckoo.insertRandomStrings();
                if (cuckoo.construct()) {
                    successful++;
                }
            }
            std::cout << ",   \tSuccessful: " << successful << ",\tCost: " << cost << std::endl;
        } while (successful >= 22 && sumThresholds <= 100);
        thresholds[lookAt] -= 3 * delta;
        thresholds[NUM_THRESHOLDS - 1] += 2 * delta;
        lookAt++;
    }
}

double costAt(double thresholds[NUM_THRESHOLDS]) {
    const int M = 1 << 10;
    const int N = (int) (M * 0.98);

    int thresholdsInt[NUM_THRESHOLDS] = {0};
    int sum = 0;
    for (int i = 0; i < NUM_THRESHOLDS; i++) {
        thresholdsInt[i] = thresholds[i];
        sum += thresholdsInt[i];
        if (thresholdsInt[i] < 0) {
            return 10000-thresholdsInt[i];
        }
    }
    if (sum > 100) {
        return 10000+sum;
    }

    int successful = 0;
    for (int i = 0; i < 20; i++) {
        HashAssignmentThresholdCuckoo cuckoo(M, N, thresholdsInt);
        cuckoo.insertRandomStrings();
        if (cuckoo.construct()) {
            successful++;
        }
    }

    double cost = 0;
    for (int i = 0; i < NUM_THRESHOLDS; i++) {
        cost += 0.01 * thresholds[i] * COST_PER_CLASS[i];
    }

    if (successful <= 11) {
        return 100 + cost;
    }

    return cost;
}

void plotAssignmentThresholdCuckooTryPercentagesGradient() {
    srand(time(nullptr));
    double thresholds[NUM_THRESHOLDS] = {0};
    thresholds[NUM_THRESHOLDS - 1] = 100;

    for (int i = 0; i < NUM_THRESHOLDS; i++) {
        std::cout << "threshold" << i << "; ";
    }
    std::cout << "cost" << std::endl;

    for (int k = 0; k < 100; k++) {
        double h = 2;
        double gradient[NUM_THRESHOLDS - 1] = {0};

        double f_x0 = costAt(thresholds);
        for (int i = 0; i < NUM_THRESHOLDS; i++) {
            std::cout << (int)thresholds[i] << ";   \t";
        }
        std::cout << f_x0 << std::endl;

        double max = 0;
        for (int i = 0; i < NUM_THRESHOLDS - 1; i++) {
            double x0_h[NUM_THRESHOLDS];
            memcpy(x0_h, thresholds, NUM_THRESHOLDS*sizeof(double));
            x0_h[i] += h;
            x0_h[NUM_THRESHOLDS - 1] -= h;
            double f_x0_h = costAt(x0_h);
            gradient[i] = (f_x0 - f_x0_h) / h;
            max = std::max(max, std::abs(gradient[i]));
        }
        assert(max > 0);
        for (int i = 0; i < NUM_THRESHOLDS - 1; i++) {
            gradient[i] /= max;
            assert(gradient[i] <= 1);
            assert(gradient[i] >= -1);
        }
        double sum = 0;
        for (int i = 0; i < NUM_THRESHOLDS - 1; i++) {
            thresholds[i] += gradient[i];
            sum += thresholds[i];
        }
        thresholds[NUM_THRESHOLDS - 1] = 100.0 - sum;
    }
}

void plotHdcMax() {
    int seed = time(nullptr);
    const int M = 1<<7;
    const int N = M*1.5;
    const int BucketSize = 4;

    srand(seed);
    HdcGreedy hdc(M, N, BucketSize, 1, 1);
    hdc.insertRandomStrings();
    hdc.construct(false);
    hdc.printPlacedPercentage();
    std::cout<<std::endl;

    srand(seed);
    HdcGreedy hdcSort(M, N, BucketSize, 1, 1);
    hdcSort.insertRandomStrings();
    hdcSort.construct(true);
    hdcSort.printPlacedPercentage();
    std::cout<<std::endl;

    srand(seed);
    HdcMax hdcMax(M, N, BucketSize);
    hdcMax.insertRandomStrings();
    hdcMax.construct();
    hdcMax.printPlacedPercentage();
}

void plotHdcSlidingVariableRetrieval() {
    srand(time(nullptr));

    const int M = (1 << 9);
    const int N = (int) (1.1 * M); // Overfilling
    HdcSlidingWindow hdc(M, N, 4, 8, 1);
    hdc.insertRandomStrings();
    std::cout<<"Constructing matching..."<<std::endl;
    hdc.construct();
    hdc.printPlacedPercentage();
    std::cout<<"Inserting retrieval..."<<std::endl;
    VariableLengthRetrieval retrieval((int) (N * 0.9));
    for (int i = 0; i < hdc.NumBuckets; i++) {
        retrieval.insert(std::to_string(i), hdc.buckets[i].hashFunction + 1);
    }
    std::cout<<"Constructing retrieval..."<<std::endl;
    bool success = retrieval.construct();
    std::cout<<"Constructed: "<<success<<std::endl;
    assert(success);
}


void plotConstructionSuccessByN() {
    /*uint64_t threshold1 = UINT64_MAX / 100 * 100;
    uint64_t threshold2 = UINT64_MAX / 100 * 100;
    std::vector<std::string> keys = Comparison::generateInputData(1<<22);
    for (size_t M = (1<<12); M <= keys.size(); M *= 8) {
        for (size_t N = 0.4 * M; N <= 0.6 * M; N += 0.001 * M) {
            size_t successfulSeeds = 0;
            for (size_t seed = 0; seed < 40; seed++) {
                HeterogeneousCuckooHashTable binaryCuckooHashTable(N, threshold1, threshold2);
                for (size_t i = 0; i < N; i++) {
                    binaryCuckooHashTable.prepare(HashedKey(keys[i], seed));
                }
                if (binaryCuckooHashTable.construct(M)) {
                    successfulSeeds++;
                }
            }
            std::cout << "RESULT"
                      << " N=" << N
                      << " M=" << M
                      << " success=" << successfulSeeds
                      << std::endl;
        }
    }*/
}

int main() {
    //plotMatchingFunctions();
    //plotHdcVsSliding();
    //plotSlidingBucketSize();
    //plotSlidingWindowFunctions();
    //plotGaussianTest();
    //plotVariableLengthRetrievalTest();
    //plotVariableLengthRetrievalTestK();
    //plotPhfMatchingVariableRetrieval();
    //plotPhfMatchingVariableRetrievalSolveable();
    //plotMatchingFixpoint();
    //plotAssignmentThreshold();
    //plotAssignmentThresholdEntropy();
    //plotAssignmentThresholdLastLayerVariable();
    //plotSlidingWindowSingleHash(3, 1);
    //plotSlidingWindowSingleHash(8, 8);
    //plotHdcMax();
    //plotHdcSlidingVariableRetrieval();
    //plotAssignmentThresholdCuckoo();
    //plotAssignmentThresholdCuckooDistribution();
    //plotAssignmentThresholdCuckooTryPercentagesLinear();
    //plotAssignmentThresholdCuckooTryPercentagesGradient();
    return 0;
}
