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

    class Viterbi_Test
    {
    public:
        static void viterbi_Base(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static Dfg dfg;

        static const uint64_t N_OBS;
        static const uint64_t N_STATES;
        static const uint64_t N_TOKENS;

        static vector<int> obs;
        static vector<int> llike;
        static vector<int> transition;
        static vector<int> emission;

        static const uint64_t llike_BaseAddr;
        static const uint64_t transition_BaseAddr;
        static const uint64_t emission_BaseAddr;
    };
}