#include <tlx/cmdline_parser.hpp>
#include "benchmark/CmphContender.h"
#include "benchmark/SicHashContender.h"
#include "benchmark/PTHashContender.h"
#include "benchmark/RecSplitContender.h"
#include "benchmark/MphfWbpmContender.h"

int main(int argc, char** argv) {
    size_t N = 5e6;
    size_t iterations = 1;
    bool mphfWbpmOnly = false;
    double pthashParameter = 3.7;
    tlx::CmdlineParser cmd;
    cmd.add_bytes('n', "numKeys", N, "Number of objects");
    cmd.add_bytes('i', "iterations", iterations, "Number of iterations to execute");
    cmd.add_flag('m', "mphfWbpmOnly", mphfWbpmOnly, "Execute only mphfWbpm benchmark");
    cmd.add_double('p', "pthashParameter", pthashParameter, "Parameter of the pthash method");
    if (!cmd.process(argc, argv)) {
        return 1;
    }
    for (size_t i = 0; i < iterations; i++) {
        if (mphfWbpmOnly) {
            {MphfWbpmContender(N, MPHFFastParameters).run();}
        } else {
            // Queries of PTHash and SicHash have quite a bit of noise in the measurements.
            // Run more queries to work around that.
            Contender::numQueries = 10e7;
            {PTHashContender<false, pthash::elias_fano>(N, 0.95, pthashParameter).run();}
            {SicHashContender<false, 64>(N, 0.95, 46, 32).run();}
            Contender::numQueries = 5e7;
            {RecSplitContender<7>(N, 250).run();}
            {CmphContender(N, 0.95, "CHD", CMPH_CHD_PH, 0.95, 5, false).run();}
        }
    }
    return 0;
}
