#include "VariableLengthRetrieval.h"

#include <utility>

VariableLengthRetrieval::VariableLengthRetrieval(int Size, int K, RetrievalCoding coding)
        : gauss(Size + MAX_VALUE, K), Size(Size), K(K), coding(std::move(coding)) {

}

int *VariableLengthRetrieval::createHashes(std::string &key) {
    int *list = new int[K];
    for (int i = 0; i < K; i++) {
        list[i] = Hash::hash(key, HASH_FUNCTION_RETRIEVAL + i, Size);
    }
    return list;
}

void VariableLengthRetrieval::insert(std::string key, int value) {
    assert(value >= 0 && value < MAX_VALUE);

    std::string code = coding.encode(value);
    int *hashes = createHashes(key);
    for (int i = 0; i < code.length(); i++) {
        char expectedResult = (code.at(i) == '0') ? 0 : 1;
        gauss.addEquation(hashes, expectedResult);

        for (int k = 0; k < K; k++) {
            hashes[k]++;
        }
    }
    delete [] hashes;
}

bool VariableLengthRetrieval::construct() {
    return gauss.solve();
}

int VariableLengthRetrieval::retrieve(std::string key) {
    int *hashes = createHashes(key);

    std::string retrieved;
    int bitsRead = 0;
    do {
        char bit = 0;
        std::set<int> usedFunctions;
        for (int i = 0; i < K; i++) {
            if (usedFunctions.find(hashes[i]) != usedFunctions.end()) {
                continue; // Don't use same hash function twice
            }
            bit = bit ^ gauss.getAssignment(hashes[i] + bitsRead);
            usedFunctions.insert(hashes[i]);
        }
        retrieved += (bit == 0) ? "0" : "1";
        bitsRead++;
    } while (coding.decode(retrieved) == DECODE_ERROR);
    delete [] hashes;
    return coding.decode(retrieved);
}
