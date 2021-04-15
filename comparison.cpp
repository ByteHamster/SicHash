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
    recSplitContenderRunner(N, loadFactor);
}

int main() {
    runContenders(5e6, 0.8);
    runContenders(5e6, 0.85);
    runContenders(5e6, 0.9);
    runContenders(5e6, 0.95);
    runContenders(5e6, 0.97);
    runContenders(5e6, 0.99);
    return 0;
}
