#include "./gcn.h"
#include "../../src/util/ReadFile.hpp"

using namespace DFSimTest;

Dfg GCN_Test::dfg;

vector<int> GCN_Test::indPtr;
vector<int> GCN_Test::indices;
vector<int> GCN_Test::feat;

uint64_t GCN_Test::vertex_num;
uint64_t GCN_Test::ngh_num;

// Address map: 
//const uint64_t GCN_Test::indPtr_BaseAddr = 0;
//const uint64_t GCN_Test::indices_BaseAddr = 0;
const uint64_t GCN_Test::feat_BaseAddr = 0;

//string GCN_Test::dataset_name = "cora";
//string GCN_Test::dataset_name = "citeseer";
//string GCN_Test::dataset_name = "ogbn_arxiv";
//string GCN_Test::dataset_name = "ogbn_mag";
//string GCN_Test::dataset_name = "pubmed";
//string GCN_Test::dataset_name = "amazon_comp";

//string GCN_Test::arch_name = "hygcn";
//string GCN_Test::arch_name = "awb-gcn";
//string GCN_Test::arch_name = "i-gcn";
//string GCN_Test::arch_name = "regnn";
//string GCN_Test::arch_name = "delta-gnn";
//string GCN_Test::arch_name = "delta-gnn-opt";


uint64_t GCN_Test::feat_length;

// Performance parameter
uint64_t sp = 32;
uint64_t GCN_Test::speedup_aggr = sp;
uint64_t GCN_Test::speedup_combine = sp;
uint64_t GCN_Test::speedup_active = sp;

uint64_t GCN_Test::buffer_access_cnt = 0;
uint64_t GCN_Test::deg_th = 0;

// Hardware parameter
// Systolic array
uint64_t GCN_Test::systolic_array_width = 32;
uint64_t GCN_Test::systolic_array_length = 128;

//void GCN_Test::generateData()
//{
//    //// Load dataset
//    //string filePath;
//    //filePath = "./dataset/graph/" + dataset_name + "/" + dataset_name + "_indptr.txt";
//    //ReadFile::readFile(indPtr, filePath);
//
//    //filePath = "./dataset/graph/" + dataset_name + "/" + dataset_name + "_indices.txt";
//    //ReadFile::readFile(indices, filePath);
//
//    //vertex_num = indPtr.size() - 1;  // The last data in indPtr is not a vertex
//
//    // Set feature length
//    if (GCN_Test::dataset_name == "cora")
//    {
//        feat_length = 1433;
//    }
//    else if (GCN_Test::dataset_name == "citeseer")
//    {
//        feat_length = 3703;
//    }
//    else if (GCN_Test::dataset_name == "ogbn_arxiv")
//    {
//        feat_length = 128;
//    }
//    else if (GCN_Test::dataset_name == "ogbn_mag")
//    {
//        feat_length = 128;
//    }
//    else if (GCN_Test::dataset_name == "pubmed")
//    {
//        feat_length = 500;
//    }
//    else if (GCN_Test::dataset_name == "amazon_comp")
//    {
//        feat_length = 767;
//    }
//
//    feat.resize(vertex_num * feat_length);
//}

void GCN_Test::generateData(const string& dataset)
{
    // Set feature length
    if (dataset == "cora")
    {
        feat_length = 1433;
    }
    else if (dataset == "citeseer")
    {
        feat_length = 3703;
    }
    else if (dataset == "ogbn_arxiv")
    {
        feat_length = 128;
    }
    else if (dataset == "ogbn_mag")
    {
        feat_length = 128;
    }
    else if (dataset == "pubmed")
    {
        feat_length = 500;
    }
    else if (dataset == "amazon_comp")
    {
        feat_length = 767;
    }

    feat.resize(vertex_num * feat_length);
}

void GCN_Test::readMemTrace(deque<uint64_t>& queue, const string& filePath)
{
    ReadFile::readFile(queue, filePath);
}

void GCN_Test::readMemTraceByCol(vector<deque<uint64_t>>& queues, const string& filePath)
/*
* Read file by column (each column corresponding to one queues)
*/
{
    ReadFile::readFileByColumn(queues, filePath);
}

bool GCN_Test::readMemTraceByCol_blocked(vector<deque<uint64_t>>& queues, const string& filePath, const uint64_t& block_line_size, uint64_t& line_id)
/*
* Read file blocked for saving DRAM 
* Read file by column (each column corresponding to one queues)
*/
{
    return ReadFile::readFileByColumn_blocked(queues, filePath, block_line_size, line_id);
}

void GCN_Test::injectMemTrace(Channel* producerChan, Channel* consumerLse, deque<uint64_t>& addr_q)
/*
* Inject memory trace (addr) to Lse. To emulate accessing specific address.
*/
{
    if (!consumerLse->bp[0] && producerChan->valid)
    {
        if (!addr_q.empty())
        {
            producerChan->value = addr_q.front();
            addr_q.pop_front();
            //addr_q.shrink_to_fit();
        }
    }
}

void GCN_Test::bindDelay(Channel* producerChan, Channel* consumeChan, deque<uint64_t>& delay_q, const string& arch_name)
/*
* Add delay (cycle) to data. To emulate synchronize cost
*/
{
    if (!consumeChan->bp[0] && producerChan->valid)
    {
        if (!delay_q.empty())
        {
            if (arch_name == "awb-gcn")
            {
                // Awb-gcn has workload balance
                ++buffer_access_cnt;
            }
            else if (arch_name == "delta_gnn_opt")
            {
                if (delay_q.front() > deg_th)  // SIMD mode: increase buffer accesses
                {
                    ++buffer_access_cnt;
                }
                else  // Systolic mode: increase synchronize delay
                {
                    consumeChan->cycle += delay_q.front();
                }
            }
            else
            {
                consumeChan->cycle += delay_q.front();
                ++buffer_access_cnt;
            }
            delay_q.pop_front();
        }
    }
}

void GCN_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_i", "Loop", "Null"),
        make_tuple<string, string, string>("loop_j", "Loop", "Null"),
        });

    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_j" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    /*dfg.addNode("row_size", "Const", GCN_Test::matrix_width);*/

    // loop_i
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "i" });
    dfg.addNode("i", "Loop_head", {}, { "begin" });
    dfg.addNode("i_lc", "Nop", { "i" }, {});

    // Aggregation
    dfg.addNode("traverse_root", "Nop", { "i_lc" });
    dfg.addNode("ld_ngh", "Load", { "traverse_root" }, &feat, feat_BaseAddr);

    ////** Aggregation
    //// Traverse vertex
    //dfg.addNode("indptr", "Nop", { "i_lc" });
    //dfg.addNode("access_indptr", "Load", { "indptr" }, &indPtr, indPtr_BaseAddr);
    //dfg.addNode("ngh_ind_base", "Nop", { "access_indptr" });

    //// loop_j
    //dfg.addNode("j", "Loop_head", {}, { "ngh_ind_base" });
    //dfg.addNode("j_lc", "Nop", { "j" }, {});
    //// Access neighbor
    //dfg.addNode("indices", "Add", { "j_lc", "ngh_ind_base" });
    //dfg.addNode("access_ngh", "Load", { "indices" }, &indices, indices_BaseAddr);
    //// Ld feature
    //dfg.addNode("ngh_ind", "Nop", { "access_ngh" });
    //dfg.addNode("ld_feat", "Load", { "ngh_ind" }, &feat, feat_BaseAddr);

    //** Combination
    dfg.addNode("systolic", "Nop", { "ld_ngh" });  // Delay is added in the runtime

    //** Activation
    dfg.addNode("active", "Relu", { "systolic" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    /*dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc", "indptr", "access_indptr", "ngh_ind_base", "active" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "j_lc", "indices", "access_ngh", "ngh_ind", "ld_feat", "combine" });*/
    dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc", "traverse_root", "ld_ngh", "systolic", "active" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_i", "active");
    //dfg.setTheTailNode("loop_j", "combine");

    dfg.plotDot();
}


void GCN_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{

}


