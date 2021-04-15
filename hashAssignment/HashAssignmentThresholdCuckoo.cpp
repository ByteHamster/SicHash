#include "HashAssignmentThresholdCuckoo.h"

HashAssignmentThresholdCuckoo::HashAssignmentThresholdCuckoo(int M, int N, int thresholds[NUM_THRESHOLDS], bool greedyBump)
        : HashAssignmentThreshold(M, N, thresholds) {
    this->greedyBump = greedyBump;
    storage.resize(M);
    stack.resize(insertTtl);
}

int noThresholds[NUM_THRESHOLDS] = {0};
HashAssignmentThresholdCuckoo::HashAssignmentThresholdCuckoo(int M, int N, bool greedyBump)
        : HashAssignmentThresholdCuckoo(M, N, noThresholds, greedyBump) {
    stack.resize(insertTtl);
}

void HashAssignmentThresholdCuckoo::insert(ElementHasher element) {
    elements.push_back(element);
}

uint32_t x32 = 314159265;
uint32_t xorshift32()
{
    x32 ^= x32 << 13;
    x32 ^= x32 >> 17;
    x32 ^= x32 << 5;
    return x32;
}

std::vector<std::pair<int, HashAssignmentThresholdCuckoo::Cuckoo>> HashAssignmentThresholdCuckoo::stack;

bool HashAssignmentThresholdCuckoo::tryInsert(Cuckoo element) {
    int stackSize = 0;

    while (stackSize < insertTtl) {
        int randomWalkDirection = xorshift32() & HASH_FUNCTIONS_PER_CLASS_MASK[element.elementClass];
        int position = element.element.hash(randomWalkDirection, M);
        Cuckoo oldElement = storage[position];

        if (oldElement.isPlaceholder) {
            element.hashFunctionIndex = randomWalkDirection;
            storage[position] = element;
            return true;
        } else {
            if (greedyBump) {
                stack.at(stackSize) = std::make_pair(position, oldElement);
            }
            element.hashFunctionIndex = randomWalkDirection;
            storage[position] = element;
            element = oldElement;
            stackSize++;
        }
    }

    // Roll-back in case we are doing incremental inserts
    if (greedyBump) {
        stackSize--;
        while (stackSize >= 0) {
            std::pair<int, Cuckoo> &old = stack.at(stackSize);
            storage.at(old.first) = old.second;
            stackSize--;
        }
    }
    return false;
}

bool HashAssignmentThresholdCuckoo::construct() {
    std::vector<std::vector<std::pair<uint64_t, unsigned char>>> hashFunctionRetrieval;
    hashFunctionRetrieval.resize(NUM_THRESHOLDS);
    if (!this->construct(hashFunctionRetrieval)) {
        return false;
    }
    retrieval1 = std::make_unique<SimpleRibbon<1>>(hashFunctionRetrieval.at(0));
    retrieval2 = std::make_unique<SimpleRibbon<2>>(hashFunctionRetrieval.at(1));
    retrieval3 = std::make_unique<SimpleRibbon<3>>(hashFunctionRetrieval.at(2));
    return true;
}

bool HashAssignmentThresholdCuckoo::construct(std::vector<std::vector<std::pair<uint64_t, unsigned char>>> &hashFunctionRetrieval) {
    int elementClass = 0;
    int bumpedInARow = 0;

    // Place elements
    for (int i = 0; i < elements.size(); i++) {
        Cuckoo newElement = {elements.at(i)};
        newElement.isPlaceholder = false;

        if (greedyBump) { // Ignore pre-defined thresholds and switch to next class when insertion fails
            bool success = false;
            if (bumpedInARow >= 0.008 * M) {
                // Bump all remaining based on threshold
                bumpedRemainder.push_back(newElement);
                success = true;
            }
            while (!success) {
                newElement.elementClass = elementClass;
                success = tryInsert(newElement);
                if (success) {
                    bumpedInARow = 0;

                    if (earlyStop) {
                        int classPercentageSum = 0;
                        for (int d = 0; d <= elementClass; d++) {
                            classPercentageSum += thresholds[d];
                        }
                        if ((double) i / N > 0.01 * classPercentageSum) {
                            // Early increase of class
                            elementClass++;
                        }
                    }
                } else {
                    if (elementClass < NUM_THRESHOLDS - 1) {
                        elementClass++; // Try again
                    } else {
                        bumpedInARow++;
                        bumped.push_back(newElement);
                        success = true;
                    }
                }
            }
        } else {
            newElement.elementClass = getElementClass(newElement.element);
            bool success = tryInsert(newElement);
            if (!success) {
                return false;
            }
        }
    }

    // Find locations of elements
    for (int i = 0; i < M; i++) {
        if (!storage.at(i).isPlaceholder) {
            int hashFunction = storage.at(i).hashFunctionIndex;
            assert(hashFunction >= 0);
            // This ignores the parent class.
            // The direct parent class assumes that the class can be obtained by calling getClass,
            // which is wrong when CUCKOO_GREEDY_BUMP is set.
            HashAssignment::notifyInserted(storage.at(i).element, hashFunction);
            if (!greedyBump) {
                hashFunctionRetrieval.at(storage.at(i).elementClass).emplace_back(storage.at(i).element.mhc, hashFunction);
            }

            hashFunctionUsagePerBucket[storage.at(i).elementClass][hashFunction]++;
            classes[storage.at(i).elementClass].push_back(storage.at(i).element);
        }
    }
    return true;
}

size_t HashAssignmentThresholdCuckoo::size() {
    assert(bumped.size() == 0);
    return 8*(retrieval1->size() + retrieval2->size() + retrieval3->size());
}

int HashAssignmentThresholdCuckoo::hash(std::string &key) {
    if (greedyBump) {
        assert(false && "Not implemented");
        return -1;
    }
    return HashAssignmentThreshold::hash(key);
}

int HashAssignmentThresholdCuckoo::hash(uint64_t mhc) {
    if (greedyBump) {
        assert(false && "Not implemented");
        return -1;
    }
    ElementHasher element(mhc, seed);
    int elementClass = getElementClass(element);
    int hashFunction;
    switch (elementClass) {
        case 0:
            hashFunction = retrieval1->retrieve(element.mhc);
            break;
        case 1:
            hashFunction = retrieval2->retrieve(element.mhc);
            break;
        case 2:
            hashFunction = retrieval3->retrieve(element.mhc);
            break;
        default:
            assert(false);
    }
    return element.hash(hashFunction, M);
}

void HashAssignmentThresholdCuckoo::printEntropy() {
    HashAssignmentThreshold::printEntropy();

    if (greedyBump) {
        std::cout<<"Bumped: "<<bumped.size()<<" ("<<(100*bumped.size()/N)<<"%)"<<std::endl;
        std::cout<<"Bumped remainder: "<<bumpedRemainder.size()<<" ("<<(100*bumpedRemainder.size()/N)<<"%)"<<std::endl;
    }
}

std::string HashAssignmentThresholdCuckoo::name() {
    return "HashAssignmentThresholdCuckoo";
}
