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

/*
    Lud src code from Rodinia benchmark suite
*/

namespace DFSimTest
{
    using namespace DFSim;

    class LudTest
    {
    public:
        static void lud_Base(Debug* debug);
        static void lud_DGSF(Debug* debug);
        static void lud_Base_auto(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;

        static vector<int> matrix;
        static uint64_t matrix_size;
        static uint64_t baseAddr;

        // Performance parameter
        // Base
        static uint64_t Base_outer_loop_speedup;
        static uint64_t Base_inner_loop_speedup;

        //DGSF
        static uint64_t DGSF_outer_loop_speedup;
        static uint64_t DGSF_inner_loop_speedup;

        static uint64_t DGSF_outer_loop_buffer_size;
    };
}