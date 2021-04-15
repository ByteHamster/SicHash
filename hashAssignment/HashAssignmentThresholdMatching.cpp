#include "HashAssignmentThresholdMatching.h"

HashAssignmentThresholdMatching::HashAssignmentThresholdMatching(int M, int N, int thresholds[NUM_THRESHOLDS])
        : HashAssignmentThreshold(M, N, thresholds) {
    elements.reserve(M);
    edges.reserve(M);
}

void HashAssignmentThresholdMatching::insert(ElementHasher element) {
    // Add to graph
    int elementNumber = elements.size();
    elements.push_back(element);

    int elementClass = getElementClass(element);
    for (int i = 0; i < HASH_FUNCTIONS_PER_CLASS[elementClass]; i++) {
        int hashTarget = element.hash(i, M);
        edges.emplace_back(elementNumber, hashTarget, i*i);
    }
}

bool HashAssignmentThresholdMatching::construct() {
    // If there are fewer elements than locations,
    // add "empty" pseudo elements that can be assigned to any location for free
    int numElementsAssigned = elements.size();
    for (int pseudoElem = numElementsAssigned; pseudoElem < M; pseudoElem++) {
        for (int i = 0; i < M; i++) {
            edges.emplace_back(pseudoElem, i, 1);
        }
    }

    // Calculate minimum matching and call HashAssignment::insert(element, hashFunction)
    std::vector<int> matching = hungarianMinimumWeightPerfectMatching(M, edges);
    if (matching.size() == 0) {
        return false;
    }
    for (int e = 0; e < matching.size(); e++) {
        int elementIndex = e;
        int locationIndex = matching.at(e);

        if (elementIndex >= elements.size()) {
            // std::cout<<"Ignoring matching for empty-element "<<elementIndex<<std::endl;
            continue;
        }
        ElementHasher element = elements.at(elementIndex);
        // Search hash function index
        int i = 0;
        for (; i < MAX_HASH_FUNCTION; i++) {
            if (locationIndex == element.hash(i, M)) {
                HashAssignmentThreshold::notifyInserted(element, i);
                break;
            }
        }
        if (i == MAX_HASH_FUNCTION) {
            assert(false && "Did not find hash function again");
        }
    }
    return true;
}

std::string HashAssignmentThresholdMatching::name() {
    return "Variable matching";
}
