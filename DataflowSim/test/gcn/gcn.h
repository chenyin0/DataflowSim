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

    class GCN_Test
    {
    public:
        static void gcn_Base(Debug* debug);
        static void gcn_Base_trace(Debug* debug);
        static void gcn_Base_trace_systolic(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static void readMemTrace(deque<uint>& queue, const string& filePath);  // Read memory access trace
        static void readMemTraceByCol(vector<deque<uint>>& queue, const string& filePath);
        static bool readMemTraceByCol_blocked(vector<deque<uint>>& queue, const string& filePath, const uint& block_line_size, uint& line_id);
        static void injectMemTrace(Channel* producerChan, Channel* consumerLse, deque<uint>& addr_q);  // Inject mem trace into Lse
        static void bindDelay(Channel* producerChan, Channel* consumerChan, deque<uint>& delay_q);  // Bind delay (cycle) to data

        // Construct DFG and chanGraph
        static Dfg dfg;

        // Graph in CSR format
        static vector<int> indPtr;
        static vector<int> indices;
        static vector<int> feat;
        static uint vertex_num;
        static uint ngh_num;

        static const uint indPtr_BaseAddr;
        static const uint indices_BaseAddr;
        static const uint feat_BaseAddr;
        static uint feat_length;

        static string dataset_name;
        static string arch_name;

        // Performance parameter
        // Base
        static uint speedup_aggr;
        static uint speedup_combine;
        static uint speedup_active;
    };
}