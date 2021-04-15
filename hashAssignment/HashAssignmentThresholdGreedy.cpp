#include "HashAssignmentThresholdGreedy.h"

HashAssignmentThresholdGreedy::HashAssignmentThresholdGreedy(int M, int N, int thresholds[NUM_THRESHOLDS])
        : HashAssignmentThreshold(M, N, thresholds) {
    elements.emplace_back();
    elements.emplace_back();
    elements.emplace_back();
}

void HashAssignmentThresholdGreedy::insert(ElementHasher element) {
    elements.at(getElementClass(element)).push_back(element);
}

bool HashAssignmentThresholdGreedy::construct() {
    // Place elements with few hash functions first
    for (int elementClass = 0;  elementClass < elements.size(); elementClass++) {
        for (ElementHasher element : elements.at(elementClass)) {
            std::size_t position;
            int hashFunction = -1;
            do {
                hashFunction++;
                position = element.hash(hashFunction, M);
            } while (placed[position] && hashFunction < HASH_FUNCTIONS_PER_CLASS[elementClass]);

            if (hashFunction == HASH_FUNCTIONS_PER_CLASS[elementClass]) {
                return false;
            }
            HashAssignmentThreshold::notifyInserted(element, hashFunction);
        }
    }
    return true;
}

std::string HashAssignmentThresholdGreedy::name() {
    return "VariableGreedy";
}
