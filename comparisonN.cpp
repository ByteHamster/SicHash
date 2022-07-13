#include <tlx/cmdline_parser.hpp>
#include "benchmark/CmphContender.h"
#include "benchmark/SicHashContender.h"
#include "benchmark/PTHashContender.h"
#include "benchmark/RecSplitContender.h"

int main(int argc, char** argv) {
    size_t N = 5e6;
    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Number of objects");
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    {RecSplitContender<7>(N, 250).run();}
    PTHashContender<false>(N, 0.95, 3.7).run();
    {SicHashContender<false, 64>(N, 0.95, 46, 32).run();}
    {CmphContender(N, 0.95, "CHD", CMPH_CHD_PH, 0.95, 5, false).run();}
    return 0;
}
