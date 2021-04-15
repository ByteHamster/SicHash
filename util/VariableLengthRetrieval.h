#ifndef TESTCOMPARISON_VARIABLELENGTHRETRIEVAL_H
#define TESTCOMPARISON_VARIABLELENGTHRETRIEVAL_H

#include <string>
#include <set>
#include <utility>
#include "GaussianElimination.h"
#include "Hash.h"

#define HASH_FUNCTION_RETRIEVAL (-1000)
#define MAX_VALUE 10
#define DECODE_ERROR -1

typedef struct RetrievalCoding {
    std::function<std::string(int)> encode;
    std::function<int(std::string)> decode;

    RetrievalCoding(std::function<std::string(int)> anEncode, std::function<int(std::string)> decode)
        : encode(std::move(anEncode)), decode(std::move(decode)) {

    }
} RetrievalCoding;

static const RetrievalCoding RetrievalCodingUnary = RetrievalCoding(
        [](int symbol) {
            std::string code;
            while (symbol > 0) {
                code += "0";
                symbol--;
            }
            return code + "1";
        }, [](std::string code) {
            size_t pos = code.find('1');
            if (pos == std::string::npos) {
                return DECODE_ERROR;
            } else {
                return (int) pos;
            }
        });

static const RetrievalCoding RetrievalCodingCutoff4 = RetrievalCoding(
        [](int symbol) {
            assert(symbol < 5);
            switch (symbol) {
                case 0: return "1";
                case 1: return "01";
                case 2: return "001";
                case 3: return "0001";
                case 4: return "0000";
                default:
                    assert(false && "Code does not know symbol");
                    return "This is a long string that makes the system unsolvable";
            }
        }, [](std::string code) {
            if (code == "1") {
                return 0;
            } else if (code == "01") {
                return 1;
            } else if (code == "001") {
                return 2;
            } else if (code == "0001") {
                return 3;
            } else if (code == "0000") {
                return 4;
            } else {
                return DECODE_ERROR;
            }
        });

// Stores a compressed function where the size depends on value entropy.
// Based on "Storing a Compressed Function with Constant Time Access" by
// Hreinsson, Krøyer, Pagh
class VariableLengthRetrieval {
    private:
        GaussianElimination gauss;
        int Size;
        int K;
        RetrievalCoding coding;
        int *createHashes(std::string &key);
    public:
        explicit VariableLengthRetrieval(int Size, int K = 5, RetrievalCoding coding = RetrievalCodingUnary);

        void insert(std::string key, int value);
        bool construct();
        int retrieve(std::string key);
};


#endif //TESTCOMPARISON_VARIABLELENGTHRETRIEVAL_H
