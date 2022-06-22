#ifndef TESTCOMPARISON_HASH_H
#define TESTCOMPARISON_HASH_H

#include <iostream>
#include <functional>
#include <string>
#include "sux/function/RecSplit.hpp"
#include <Function.h>
#include <MurmurHash64.h>

#define HASH_FUNCTION_MHC (-999)

class Hash {
    public:
        static inline std::size_t hash(const void * key, int len, int hashFunctionIndex, size_t range) {
            uint64_t stringHash = util::MurmurHash64(key, len);
            uint64_t modified = stringHash + hashFunctionIndex;
            return util::fastrange64(util::MurmurHash64(&modified, sizeof(uint64_t)), range);
        }

        static inline std::size_t hash(const std::string &element, int hashFunctionIndex, size_t range) {
            uint64_t stringHash = util::MurmurHash64(element.data(), element.length());
            uint64_t modified = stringHash + hashFunctionIndex;
            return util::fastrange64(util::MurmurHash64(&modified, sizeof(uint64_t)), range);
        }

        static inline std::size_t hash(uint64_t mhc, int hashFunctionIndex, size_t range) {
            uint64_t h2 =  util::MurmurHash64(&hashFunctionIndex, sizeof(int));
            uint64_t modified = mhc ^ h2;
            return util::fastrange64(util::MurmurHash64(&modified, sizeof(uint64_t)), range);
        }
};

struct ElementHasher {
    uint64_t mhc;

    ElementHasher(uint64_t mhc, uint32_t seed = 0) {
        this->mhc = mhc;
        if (seed != 0) {
            this->mhc = Hash::hash(mhc + seed, 999, UINT64_MAX);
        }
    }

    explicit ElementHasher(std::string element, uint32_t seed = 0) {
        mhc = Hash::hash(element, 999, UINT64_MAX);
        if (seed != 0) {
            this->mhc = Hash::hash(mhc + seed, 999, UINT64_MAX);
        }
    }

    inline uint64_t hash(int hashFunctionIndex, size_t range) const {
        return util::fastrange64(sux::function::remix(mhc + hashFunctionIndex), range);
    }
};

#endif //TESTCOMPARISON_HASH_H
