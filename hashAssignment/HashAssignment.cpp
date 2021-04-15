#include "HashAssignment.h"

HashAssignment::HashAssignment(const int M, const int N) : M(M), N(N) {
    placed = new char[M];
    memset(placed, 0, M * sizeof(char));
}

HashAssignment::~HashAssignment() {
    delete [] placed;
}

void HashAssignment::notifyInserted(ElementHasher element, int hashFunction) {
    #ifdef REMEMBER_ELEMENT_MAPPING
    hashFunctions.insert({element, hashFunction});
    #endif
    assert(placed[element.hash(hashFunction, M)] == false);
    placed[element.hash(hashFunction, M)] = true;

    #ifdef COUNT_HASH_FUNCTION_USAGE
        if (hashFunctionUsageCounter.count(hashFunction)) {
            hashFunctionUsageCounter[hashFunction]++;
        } else {
            hashFunctionUsageCounter.insert({hashFunction, 1});
        }
    #endif
    placedItems++;
}

void HashAssignment::insertRandomStrings() {
    for (int i = 0; i < N; i++) {
        std::string randomString = std::to_string(i) + "-" + std::to_string(rand() % 100);
        insert(ElementHasher(randomString));
    }
}

void HashAssignment::printHistogramHeader() {
    std::cout<<"ID;\t\ti;\tFunction;\tProbabil;\tN"<<std::endl;
}

void HashAssignment::printHistogram() {
    assert(placedItems > N / 2);
    #ifndef COUNT_HASH_FUNCTION_USAGE
        std::cout<<"Hash function usage count not enabled"<<std::endl;
        return;
    #endif
    int i = 0;
    for (auto const& entry: hashFunctionUsageCounter) {
        double probability = (double) entry.second / (double) placedItems;
        std::cout<<name()<<";\t"<<i<<";\t";
        std::cout<<entry.first<<";  \t\t"<<(int) (100*probability)<<"%;  \t\t"<<entry.second<<std::endl;
        i++;
    }
}

void HashAssignment::printEntropy() {
    assert(placedItems > N / 2);
    #ifndef COUNT_HASH_FUNCTION_USAGE
        std::cout<<"Hash function usage count not enabled"<<std::endl;
        return;
    #endif
    std::cout<<name()<<std::endl;
    std::cout<<"Table size: "<<M<<std::endl;
    std::cout<<"Inserted items: "<<placedItems<<std::endl;
    double entropy = 0;
    for (auto const& entry: hashFunctionUsageCounter) {
        double probability = (double) entry.second / (double) M;
        entropy -= probability * log2(probability);
    }
    std::cout<<"Entropy: "<<entropy<<std::endl;
    std::cout<<"Bits total: "<<entropy * M<<std::endl;
    std::cout<<"Bits per placed item: "<<(entropy * M)/placedItems<<std::endl;
}
