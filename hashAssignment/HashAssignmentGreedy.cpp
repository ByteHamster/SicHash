#include "HashAssignmentGreedy.h"

HashAssignmentGreedy::HashAssignmentGreedy(int M, int N) : HashAssignment(M, N) {
}

void HashAssignmentGreedy::insert(ElementHasher element) {
    std::size_t position;
    int hashFunction = -1;
    do {
        hashFunction++;
        position = element.hash(hashFunction, M);
    } while (placed[position] && hashFunction < MAX_HASH_FUNCTION);

    HashAssignment::notifyInserted(element, hashFunction);
}

bool HashAssignmentGreedy::construct() {
    // Nothing to do, already performed during insertion
    return true;
}

std::string HashAssignmentGreedy::name() {
    return "Greedy";
}
