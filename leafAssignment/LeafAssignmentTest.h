#ifndef TESTCOMPARISON_LEAFASSIGNMENTTEST_H
#define TESTCOMPARISON_LEAFASSIGNMENTTEST_H

#include "LeafAssignmentRecSplit.h"
#include "LeafAssignmentRecSplitRotate.h"
#include "LeafAssignmentPermutation.h"

class LeafAssignmentTest {
    public:
        static std::vector<std::vector<uint64_t>> generateInputData(int numTests, int leafSize) {
            std::random_device rd;
            std::mt19937_64 eng(rd());
            std::uniform_int_distribution<unsigned long long> distr;

            std::vector<std::vector<uint64_t>> tasks;
            tasks.reserve(numTests);
            for (int i = 0; i < numTests; i++) {
                std::vector<uint64_t> task;
                task.reserve(leafSize);
                for (int k = 0; k < leafSize; k++) {
                    task.push_back(distr(eng));
                }
                tasks.push_back(task);
            }
            return tasks;
        }

        template <int leafSize>
        static void testLeafSize(std::vector<std::vector<uint64_t>> &tasks, int numTests) {
            std::cout << std::endl << "Leaf size " << leafSize << std::endl;

            LeafAssignmentRecSplitRotate<leafSize, true> rotLookup;
            rotLookup.benchmark(tasks, numTests);

            LeafAssignmentRecSplitRotate<leafSize, false> rot;
            rot.benchmark(tasks, numTests);

            LeafAssignmentRecSplit<leafSize> rs;
            rs.benchmark(tasks, numTests/(leafSize/2));
        }

        static void test() {
            int maxTests = 500000;
            std::vector<std::vector<uint64_t>> tasks = generateInputData(maxTests, 16);

            testLeafSize< 8>(tasks, maxTests/1);
            testLeafSize<10>(tasks, maxTests/8);
            testLeafSize<12>(tasks, maxTests/50);
            testLeafSize<14>(tasks, maxTests/300);
            testLeafSize<16>(tasks, maxTests/1200);

            //LeafAssignmentPermutation<8, 12> perm;
            //perm.benchmark(tasks, maxTests);
        }
};

#endif //TESTCOMPARISON_LEAFASSIGNMENTTEST_H
