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

    class Gesummv_Test
    {
    public:
        static void gesummv_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;

        static vector<int> A;
        static vector<int> B;
        static vector<int> x;
        static vector<int> y;

        static const uint64_t A_BaseAddr;
        static const uint64_t B_BaseAddr;
        static const uint64_t x_BaseAddr;
        static const uint64_t y_BaseAddr;

        static const uint64_t _PB_N;
        static const uint64_t alpha;
        static const uint64_t beta;
    };
}