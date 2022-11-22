#pragma once
#include <SicHash.h>
#include <thread>

namespace sichash {
/**
 * Multiple SicHash perfect hash function, one for each construction thread.
 * If you use one single thread, this is slower than simply using SicHash directly.
 */
template<bool minimal=false, size_t ribbonWidth=64, int minimalFanoLowerBits = 3>
class PartitionedSicHash {
    private:
        size_t numThreads;
        static constexpr size_t HASH_FUNCTION_CHILD_ASSIGNMENT = 43;
        std::vector<SicHash<minimal, ribbonWidth, minimalFanoLowerBits>*> children;
        std::vector<uint64_t> childOffsets;

    public:
        PartitionedSicHash(const std::vector<std::string> &keys, SicHashConfig config, size_t numThreads)
                : numThreads(numThreads) {
            std::vector<std::vector<HashedKey>> childInput;
            childInput.resize(numThreads);
            const size_t N = keys.size();
            for (auto &singleChildInput : childInput) {
                singleChildInput.reserve(N / numThreads);
            }
            for (const std::string &key : keys) {
                HashedKey hash = HashedKey(key);
                size_t child = hash.hash(HASH_FUNCTION_CHILD_ASSIGNMENT, numThreads);
                childInput[child].push_back(hash);
            }
            children.resize(numThreads);
            std::vector<std::thread> threads;
            uint64_t childOffset = 0;
            for (size_t i = 0; i < numThreads; i++) {
                childOffsets.push_back(childOffset);
                childOffset += childInput[i].size();
                threads.emplace_back([&, i]() {
                    children[i] = new SicHash<minimal, ribbonWidth, minimalFanoLowerBits>(childInput[i], config);
                });
            }
            for (std::thread &thread : threads) {
                thread.join();
            }
        }

        ~PartitionedSicHash() {
            for (auto &child : children) {
                delete child;
            }
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            size_t spaceUsage = sizeof(this) * 8;
            for (auto &child : children) {
                spaceUsage += child->spaceUsage();
            }
            return spaceUsage;
        }

        size_t operator() (std::string &key) const {
            HashedKey hash = HashedKey(key);
            size_t child = hash.hash(HASH_FUNCTION_CHILD_ASSIGNMENT, numThreads);
            return children[child]->operator()(key) + childOffsets[child];
        }
};
} // Namespace sichash
