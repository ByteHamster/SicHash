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
    bool recsplit = false;
    bool mphfWbpm = false;
    bool bbhash = false;
    bool sichash = false;
    bool pthash = false;
    bool chd = false;
    bool bdz = false;
    bool bmz = false;
    bool chm = false;
    bool fch = false;

    tlx::CmdlineParser cmd;
    cmd.add_double('l', "loadFactor", loadFactor, "Load Factor");
    cmd.add_bytes('n', "numKeys", N, "Number of objects");
    cmd.add_bytes('q', "numQueries", Contender::numQueries, "Number of queries to perform");

    cmd.add_flag('r', "recsplit", recsplit, "Execute RecSplit benchmark");
    cmd.add_flag('m', "mphfWbpm", mphfWbpm, "Execute mphfWbpm benchmark");
    cmd.add_flag('b', "bbhash", bbhash, "Execute bbhash benchmark");
    cmd.add_flag('s', "sichash", sichash, "Execute sichash benchmark");
    cmd.add_flag('p', "pthash", pthash, "Execute pthash benchmark");
    cmd.add_flag('c', "chd", chd, "Execute chd benchmark");
    cmd.add_flag('d', "bdz", bdz, "Execute bdz benchmark");
    cmd.add_flag('z', "bmz", bmz, "Execute bmz benchmark");
    cmd.add_flag('x', "chm", chm, "Execute chm benchmark");
    cmd.add_flag('f', "fch", fch, "Execute fch benchmark");

    if (!cmd.process(argc, argv)) {
        return 1;
    }

    if (recsplit) {
        recSplitContenderRunner(N);
    }
    if (mphfWbpm) {
        mphfWbpmContenderRunner(N);
    }
    if (bbhash) {
        bbHashContenderRunner(N, loadFactor);
    }
    if (chd) {
        chdContenderRunner(N, loadFactor);
    }
    if (bdz) {
        bdzContenderRunner(N, loadFactor);
    }
    if (bmz) {
        bmzContenderRunner(N, loadFactor);
    }
    if (chm) {
        chmContenderRunner(N, loadFactor);
    }
    if (fch) {
        fchContenderRunner(N, loadFactor);
    }
    if (sichash) {
        sicHashContenderRunner(N, loadFactor);
    }
    if (pthash) {
        ptHashContenderRunner(N, loadFactor);
    }
    return 0;
}
