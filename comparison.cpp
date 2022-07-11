#include "contenders/BBHashContender.h"
#include "contenders/CmphContender.h"
#include "contenders/HeterogeneousContender.h"
#include "contenders/PTHashContender.h"
#include "contenders/RecSplitContender.h"
#include <tlx/cmdline_parser.hpp>

int main(int argc, char** argv) {
    double loadFactor = 0.8;
    size_t N = 5e6;
    bool recsplit = false;

    tlx::CmdlineParser cmd;
    cmd.add_double('l', "loadFactor", loadFactor, "Load Factor");
    cmd.add_bytes('n', "numKeys", N, "Number of objects");
    cmd.add_flag('r', "recsplit", recsplit, "Execute RecSplit test");
    if (!cmd.process(argc, argv)) {
        return 1;
    }

    bbHashContenderRunner(N, loadFactor);
    cmphContenderRunner(N, loadFactor);
    heterogeneousContenderRunner(N, loadFactor);
    ptHashContenderRunner(N, loadFactor);
    if (recsplit) {
        recSplitContenderRunner(N);
    }
    return 0;
}
