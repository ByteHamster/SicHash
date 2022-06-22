#include "contenders/BBHashContender.h"
#include "contenders/CmphContender.h"
#include "contenders/HeterogeneousContender.h"
#include "contenders/PTHashContender.h"
#include "contenders/RecSplitContender.h"

void runContenders(size_t N, double loadFactor) {
    bbHashContenderRunner(N, loadFactor);
    cmphContenderRunner(N, loadFactor);
    heterogeneousContenderRunner(N, loadFactor);
    ptHashContenderRunner(N, loadFactor);
}

int main() {
    size_t N = 5e6;
    runContenders(N, 0.8);
    runContenders(N, 0.85);
    runContenders(N, 0.9);
    runContenders(N, 0.95);
    runContenders(N, 0.97);
    recSplitContenderRunner(N);
    return 0;
}
