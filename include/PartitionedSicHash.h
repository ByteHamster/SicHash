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
            children.resize(numThreads);
            if (numThreads == 1) {
                children[0] = new SicHash<minimal, ribbonWidth, minimalFanoLowerBits>(keys, config);
                childOffsets.push_back(0);
                childOffsets.push_back(children[0]->M);
                return;
            }
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
            std::vector<std::thread> threads;
            std::atomic<bool> hadException = false;
            for (size_t i = 0; i < numThreads; i++) {
                threads.emplace_back([&, i]() {
                    try {
                        children[i] = new SicHash<minimal, ribbonWidth, minimalFanoLowerBits>(childInput[i], config);
                    } catch (const std::exception& e) {
                        std::cout<<"Error: "<<e.what()<<std::endl;
                        hadException = true;
                    }
                });
            }
            for (size_t i = 0; i < numThreads; i++) {
                threads[i].join();
            }
            if (hadException) {
                throw std::logic_error("One construction thread experienced a problem. Read output for details.");
            }
            uint64_t childOffset = 0;
            for (size_t i = 0; i < numThreads; i++) {
                childOffsets.push_back(childOffset);
                childOffset += children[i]->M;
            }
        }

        ~PartitionedSicHash() {
            for (auto &child : children) {
                delete child;
            }
        }

        /** Estimate for the space usage of this structure, in bits */
        [[nodiscard]] size_t spaceUsage() const {
            size_t spaceUsage = sizeof(*this) * 8;
            for (auto &child : children) {
                spaceUsage += child->spaceUsage();
            }
            return spaceUsage;
        }

        size_t operator() (std::string &key) const {
            HashedKey hash = HashedKey(key);
            size_t child = hash.hash(HASH_FUNCTION_CHILD_ASSIGNMENT, numThreads);
            return children[child]->operator()(hash) + childOffsets[child];
        }
};
} // Namespace sichash
