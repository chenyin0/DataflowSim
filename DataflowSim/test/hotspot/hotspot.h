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

    class HotSpot_Test
    {
    public:
        static void hotSpot_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;
        static vector<int> temp;
        static vector<int> power;
        static const uint64_t CAP;
        static const uint64_t RX;
        static const uint64_t RY;
        static const uint64_t RZ;
        static const uint64_t block_size;

        static const uint64_t temp_BaseAddr;
        static const uint64_t power_BaseAddr;
    };
}