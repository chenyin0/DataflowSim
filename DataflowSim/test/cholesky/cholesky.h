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

namespace DFSimTest
{
    using namespace DFSim;

    class Cholesky_Test
    {
    public:
        static void cholesky_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;

        static vector<int> A;
        //static vector<int> x;
        static uint matrix_size;
        static uint baseAddr_A;
        //static uint baseAddr_x;
    };
}