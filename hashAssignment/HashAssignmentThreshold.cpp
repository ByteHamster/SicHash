#include "HashAssignmentThreshold.h"

HashAssignmentThreshold::HashAssignmentThreshold(int M, int N, int thresholds[NUM_THRESHOLDS]) : HashAssignment(M, N) {
    memcpy(this->thresholds, thresholds, NUM_THRESHOLDS * sizeof(int));
    classes.resize(NUM_THRESHOLDS);
    hashFunctionRetrieval.resize(NUM_THRESHOLDS);
}

int HashAssignmentThreshold::getElementClass(ElementHasher element) const {
    int classify = element.hash(HASH_FUNCTION_CLASSIFY, 100);

    int threshold = 0;
    for (int i = 0; i < NUM_THRESHOLDS; i++) {
        threshold += thresholds[i];
        if (classify < threshold) {
            return i;
        }
    }
    return NUM_THRESHOLDS - 1;
}

void HashAssignmentThreshold::printEntropy() {
    HashAssignment::printEntropy();

    for (int clazz = 0; clazz < NUM_THRESHOLDS; clazz++) {
        int sum = 0;
        for (int i = 0; i < HASH_FUNCTIONS_PER_CLASS[clazz]; i++) {
            sum += hashFunctionUsagePerBucket[clazz][i];
        }
        std::cout<<"Structure with class "<<clazz<<" ("<<(100*sum/placedItems)<<"%)"<<std::endl;
        if (sum > 0) {
            for (int i = 0; i < HASH_FUNCTIONS_PER_CLASS[clazz]; i++) {
                int n = hashFunctionUsagePerBucket[clazz][i];
                std::cout<<"  Hash function "<<i<<": "<<n<<" ("<<(100*n/sum)<<"%)"<<std::endl;
            }
        }
    }
}

void HashAssignmentThreshold::notifyInserted(ElementHasher element, int hashFunction) {
    HashAssignment::notifyInserted(element, hashFunction);
    int elementClass = getElementClass(element);
    assert(hashFunction < HASH_FUNCTIONS_PER_CLASS[elementClass]);
    hashFunctionUsagePerBucket[elementClass][hashFunction]++;
    classes[elementClass].push_back(element);
    hashFunctionRetrieval.at(elementClass).emplace(element.mhc, hashFunction);
}

int HashAssignmentThreshold::hash(std::string &key) {
    ElementHasher element(key, seed);
    int elementClass = getElementClass(element);
    int hashFunction = hashFunctionRetrieval.at(elementClass).at(element.mhc);
    return element.hash(hashFunction, M);
}

std::string HashAssignmentThreshold::name() {
    return "Variable";
}
