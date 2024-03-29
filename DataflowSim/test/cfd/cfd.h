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

    class Cfd_Test
    {
    public:
        static void cfd_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;

        static uint64_t nelr;
        static uint64_t VAR_DENSITY;
        static uint64_t VAR_MOMENTUM;
        static uint64_t VAR_DENSITY_ENERGY;
        static uint64_t block_length;

        static uint64_t variables_BaseAddr;
        static uint64_t normals_BaseAddr;
        static vector<int> variables;
        static vector<int> normals;
    };
}