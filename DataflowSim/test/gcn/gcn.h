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
        /*static void gcn_Base_trace_systolic(Debug* debug);*/
        static void gcn_Base_trace_systolic(Debug* debug, const string& input_file_path, const string& dataset, const string& arch_name, const string& deg_th);

    private:
        //static void generateData();  // Generate benchmark data
        static void generateData(const string& dataset);  // Generate benchmark data
        static void generateDfg();  // Generate Dfg with control tree
        static void graphPartition(ChanGraph& chanGraph, int partitionNum);

        static void readMemTrace(deque<uint64_t>& queue, const string& filePath);  // Read memory access trace
        static void readMemTraceByCol(vector<deque<uint64_t>>& queue, const string& filePath);
        static bool readMemTraceByCol_blocked(vector<deque<uint64_t>>& queue, const string& filePath, const uint64_t& block_line_size, uint64_t& line_id);
        static void injectMemTrace(Channel* producerChan, Channel* consumerLse, deque<uint64_t>& addr_q);  // Inject mem trace into Lse
        static void bindDelay(Channel* producerChan, Channel* consumeChan, deque<uint64_t>& delay_qm, const string& arch_name);  // Bind delay (cycle) to data

        // Construct DFG and chanGraph
        static Dfg dfg;

        // Graph in CSR format
        static vector<int> indPtr;
        static vector<int> indices;
        static vector<int> feat;
        static uint64_t vertex_num;
        static uint64_t ngh_num;

        static const uint64_t indPtr_BaseAddr;
        static const uint64_t indices_BaseAddr;
        static const uint64_t feat_BaseAddr;
        static uint64_t feat_length;

        //static string dataset_name;
        //static string arch_name;

        // Performance parameter
        // Base
        static uint64_t speedup_aggr;
        static uint64_t speedup_combine;
        static uint64_t speedup_active;
        
        static uint64_t buffer_access_cnt;
        static uint64_t deg_th;

        // Systolic array parameter
        // PE number = width * length
        static uint64_t systolic_array_width;
        static uint64_t systolic_array_length;
    };
}