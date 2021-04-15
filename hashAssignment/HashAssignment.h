#ifndef TESTCOMPARISON_HASHASSIGNMENT_H
#define TESTCOMPARISON_HASHASSIGNMENT_H

#include <bitset>
#include <csignal>
#include <map>
#include <cmath>
#include <cassert>
#include <cstring>

#include "../util/Hash.h"

#define MAX_HASH_FUNCTION 500
//#define REMEMBER_ELEMENT_MAPPING
//#define COUNT_HASH_FUNCTION_USAGE

// Determines an assignment g: item->hash function index
// so that f: h_{g(i)}(i) is a perfect hash function.
// Subclasses try to choose the indices in g as small as possible
// so that g can be stored efficiently.
// Minimum: log2(e) + (m/n-1)log2(1-n/m) [HDC Paper]
// Wolfram Alpha: log2(e)+(m/n-1)*log2(1-n/m) for n=10000 m=n*1.02
class HashAssignment {
    protected:
        char *placed;
    public:
        int seed = 0;
        int placedItems = 0;
        std::map<int, int> hashFunctionUsageCounter;
        #ifdef REMEMBER_ELEMENT_MAPPING
        std::map<ElementHasher, int> hashFunctions;
        #endif
        const int M; // Hash function target domain size
        const int N; // Number of elements to insert.

        HashAssignment(int M, int N);
        ~HashAssignment();

        virtual void insert(ElementHasher element) = 0;
        virtual bool construct() = 0;
        virtual std::string name() = 0;

        virtual void notifyInserted(ElementHasher element, int hashFunction);
        void insertRandomStrings();
        static void printHistogramHeader();
        void printHistogram();
        virtual void printEntropy();
};


#endif //TESTCOMPARISON_HASHASSIGNMENT_H
