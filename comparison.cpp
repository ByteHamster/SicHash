#include <tlx/cmdline_parser.hpp>
#include "benchmark/BBHashContender.h"
#include "benchmark/CmphContender.h"
#include "benchmark/SicHashContender.h"
#include "benchmark/PTHashContender.h"
#include "benchmark/RecSplitContender.h"
#include "benchmark/MphfWbpmContender.h"

int main(int argc, char** argv) {
    double loadFactor = 0.8;
    size_t N = 5e6;
    bool recsplitOnly = false;
    bool mphfWbpmOnly = false;

    tlx::CmdlineParser cmd;
    cmd.add_double('l', "loadFactor", loadFactor, "Load Factor");
    cmd.add_bytes('n', "numKeys", N, "Number of objects");

    cmd.add_flag('r', "recsplitOnly", recsplitOnly, "Execute only RecSplit benchmark");
    cmd.add_flag('m', "mphfWbpmOnly", mphfWbpmOnly, "Execute only mphfWbpm benchmark");
    if (!cmd.process(argc, argv)) {
        return 1;
    }

    if (recsplitOnly) {
        recSplitContenderRunner(N);
    } else if (mphfWbpmOnly) {
        mphfWbpmContenderRunner(N);
    } else {
        bbHashContenderRunner(N, loadFactor);
        cmphContenderRunner(N, loadFactor);
        sicHashContenderRunner(N, loadFactor);
        ptHashContenderRunner(N, loadFactor);
    }
    return 0;
}
