#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/execution/Lc.h"
#include "../../src/module/execution/Mux.h"
#include "../../src/module/ClkSys.h"
#include "../../src/sim/Debug.h"
#include "../../src/module/Registry.h"
#include "../../src/module/Profiler.h"
#include "../../src/sim/graph.h"
#include "../../src/sim/control_tree.h"

namespace DFSimTest
{
    using namespace DFSim;

    class FFT_Test
    {
    public:
        static void fft_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;
        static const uint64_t fft_size;

        static vector<int> real;
        static vector<int> img;
        static vector<int> real_twid;
        static vector<int> img_twid;

        static const uint64_t real_BaseAddr;
        static const uint64_t img_BaseAddr;
        static const uint64_t real_twid_BaseAddr;
        static const uint64_t img_twid_BaseAddr;
    };
}