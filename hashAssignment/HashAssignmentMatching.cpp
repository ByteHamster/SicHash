#include "HashAssignmentMatching.h"

HashAssignmentMatching::HashAssignmentMatching(int M, int N,
                       const std::function<int(int)> &costFunction,
                       const std::string &functionName,
                       bool bumping)
        : costFunction(costFunction), functionName(functionName), bumping(bumping), HashAssignment(M, N) {
    elements.reserve(M);
    edges.reserve(M * MAX_HASH_FUNCTION);
}

void HashAssignmentMatching::insert(ElementHasher element) {
    // Add to graph
    int elementNumber = elements.size();
    elements.push_back(element);

    for (int i = 0; i < MAX_HASH_FUNCTION; i++) {
        int hashTarget = element.hash(i, M);
        edges.emplace_back(elementNumber, hashTarget, costFunction(i));
    }
    if (bumping) {
        for (int i = 0; i < BUMP_SLOTS; i++) {
            edges.emplace_back(elementNumber, M + i, costFunction(BUMP_COST_INDEX));
        }
    }
}

bool HashAssignmentMatching::construct() {
    // If there are fewer elements than locations,
    // add "empty" pseudo elements that can be assigned to any location for free
    int numElementsAssigned = elements.size();
    for (int pseudoElem = numElementsAssigned; pseudoElem < M; pseudoElem++) {
        for (int i = 0; i < M; i++) {
            edges.emplace_back(pseudoElem, i, costFunction(0));
        }
    }

    // Assign bump elements to bump locations and to element locations
    // (if bumping is disabled, they will be assigned to bump locations)
    for (int bumpElem = 0; bumpElem < BUMP_SLOTS; bumpElem++) {
        for (int i = 0; i < M; i++) {
            edges.emplace_back(M + bumpElem, i, costFunction(0));
        }
        for (int i = 0; i < BUMP_SLOTS; i++) {
            edges.emplace_back(M + bumpElem, M + i, costFunction(0));
        }
    }

    // Calculate minimum matching and call HashAssignment::insert(element, hashFunction)
    std::vector<int> matching = hungarianMinimumWeightPerfectMatching(M + BUMP_SLOTS, edges);
    if (matching.size() == 0) {
        assert(matching.size() > 0);
        return false;
    }
    for (int e = 0; e < matching.size(); e++) {
        int elementIndex = e;
        int locationIndex = matching.at(e);

        if (elementIndex >= elements.size()) {
            // std::cout<<"Ignoring matching for empty-element "<<elementIndex<<std::endl;
            continue;
        } else if (locationIndex >= M) {
            // std::cout<<"Bumped element "<<elementIndex<<std::endl;
            continue;
        }
        ElementHasher element = elements.at(elementIndex);
        // Search hash function index
        int i = 0;
        for (; i < MAX_HASH_FUNCTION; i++) {
            if (locationIndex == element.hash(i, M)) {
                HashAssignment::notifyInserted(element, i);
                break;
            }
        }
        if (i == MAX_HASH_FUNCTION) {
            assert(false && "Did not find hash function again");
        }
    }
    return true;
}

std::string HashAssignmentMatching::name() {
    return "Matching (" + functionName + (bumping ? ") Bumping" : ")");
}
