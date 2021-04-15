#ifndef TESTCOMPARISON_HASH_H
#define TESTCOMPARISON_HASH_H

#include <iostream>
#include <functional>
#include <string>
#include "fastrange.h"
#include "sux/function/RecSplit.hpp"

#define HASH_FUNCTION_MHC (-999)

class Hash {
    private:
        static inline uint64_t MurmurHash64A(const void * key, int len) {
            const uint64_t m = 0xc6a4a7935bd1e995;
            const size_t seed = 1203989050u;
            const int r = 47;

            uint64_t h = seed ^ (len * m);

            const uint64_t * data = (const uint64_t *) key;
            const uint64_t * end = data + (len/8);

            while(data != end)
            {
                uint64_t k = *data++;

                k *= m;
                k ^= k >> r;
                k *= m;

                h ^= k;
                h *= m;
            }

            const unsigned char * data2 = (const unsigned char*) data;

            switch(len & 7)
            {
                case 7: h ^= uint64_t(data2[6]) << 48; // fallthrough
                case 6: h ^= uint64_t(data2[5]) << 40; // fallthrough
                case 5: h ^= uint64_t(data2[4]) << 32; // fallthrough
                case 4: h ^= uint64_t(data2[3]) << 24; // fallthrough
                case 3: h ^= uint64_t(data2[2]) << 16; // fallthrough
                case 2: h ^= uint64_t(data2[1]) << 8;  // fallthrough
                case 1: h ^= uint64_t(data2[0]);
                    h *= m;
            };

            h ^= h >> r;
            h *= m;
            h ^= h >> r;

            return h;
        }
    public:
        static inline std::size_t hash(const void * key, int len, int hashFunctionIndex, size_t range) {
            uint64_t stringHash = MurmurHash64A(key, len);
            uint64_t modified = stringHash + hashFunctionIndex;
            return fastrange64(MurmurHash64A(&modified, sizeof(uint64_t)), range);
        }

        static inline std::size_t hash(const std::string &element, int hashFunctionIndex, size_t range) {
            uint64_t stringHash = MurmurHash64A(element.data(), element.length());
            uint64_t modified = stringHash + hashFunctionIndex;
            return fastrange64(MurmurHash64A(&modified, sizeof(uint64_t)), range);
        }

        static inline std::size_t hash(uint64_t mhc, int hashFunctionIndex, size_t range) {
            uint64_t h2 =  MurmurHash64A(&hashFunctionIndex, sizeof(int));
            uint64_t modified = mhc ^ h2;
            return fastrange64(MurmurHash64A(&modified, sizeof(uint64_t)), range);
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
        return fastrange64(sux::function::remix(mhc + hashFunctionIndex), range);
    }
};

#endif //TESTCOMPARISON_HASH_H
