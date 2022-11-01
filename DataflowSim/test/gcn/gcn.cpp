#include "./gcn.h"
#include "../../src/util/ReadFile.hpp"

using namespace DFSimTest;

Dfg GCN_Test::dfg;

vector<int> GCN_Test::indPtr;
vector<int> GCN_Test::indices;
vector<int> GCN_Test::feat;

uint GCN_Test::vertex_num;
uint GCN_Test::ngh_num;

// Address map: 
const uint GCN_Test::indPtr_BaseAddr = 0;
const uint GCN_Test::indices_BaseAddr = 0;
const uint GCN_Test::feat_BaseAddr = 0;

string GCN_Test::dataset_name = "cora";
//string GCN_Test::dataset_name = "citeseer";
//string GCN_Test::dataset_name = "pubmed";
//string GCN_Test::dataset_name = "amazon_comp";

uint GCN_Test::feat_length;

// Performance parameter
uint GCN_Test::speedup_aggr = 16;
uint GCN_Test::speedup_combine = 16;
uint GCN_Test::speedup_active = 16;

void GCN_Test::generateData()
{
    // Load dataset
    string filePath;
    filePath = "./dataset/graph/" + dataset_name + "/" + dataset_name + "_indptr.txt";
    ReadFile::readFile(indPtr, filePath);

    filePath = "./dataset/graph/" + dataset_name + "/" + dataset_name + "_indices.txt";
    ReadFile::readFile(indices, filePath);

    vertex_num = indPtr.size() - 1;  // The last data in indPtr is not a vertex

    // Set feature length
    if (GCN_Test::dataset_name == "cora")
    {
        feat_length = 1433;
    }
    else if (GCN_Test::dataset_name == "citeseer")
    {
        feat_length = 3703;
    }
    else if (GCN_Test::dataset_name == "pubmed")
    {
        feat_length = 500;
    }
    else if (GCN_Test::dataset_name == "amazon_comp")
    {
        feat_length = 767;
    }

    feat.resize(vertex_num * feat_length);
}

void GCN_Test::readMemTrace(deque<uint>& queue, const string& filePath)
{
    ReadFile::readFile(queue, filePath);
}

void GCN_Test::memTraceInjection(Channel* producerChan, Channel* consumerLse, deque<uint>& queue)
{
    if (!consumerLse->bp[0])
    {
        if (!queue.empty())
        {
            producerChan->value = queue.front();
            queue.pop_front();
            queue.shrink_to_fit();
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

    //** Aggregation
    // Traverse vertex
    dfg.addNode("indptr", "Nop", { "i_lc" });
    dfg.addNode("access_indptr", "Load", { "indptr" }, &indPtr, indPtr_BaseAddr);
    dfg.addNode("ngh_ind_base", "Nop", { "access_indptr" });

    // loop_j
    dfg.addNode("j", "Loop_head", {}, { "ngh_ind_base" });
    dfg.addNode("j_lc", "Nop", { "j" }, {});
    // Access neighbor
    dfg.addNode("indices", "Add", { "j_lc", "ngh_ind_base" });
    dfg.addNode("access_ngh", "Load", { "indices" }, &indices, indices_BaseAddr);
    // Ld feature
    dfg.addNode("ngh_ind", "Nop", { "access_ngh" });
    dfg.addNode("ld_feat", "Load", { "ngh_ind" }, &feat, feat_BaseAddr);

    //** Combination
    dfg.addNode("combine", "Mac", { "ld_feat" });

    //** Activation
    dfg.addNode("active", "Relu", { "combine" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc", "indptr", "access_indptr", "ngh_ind_base", "active" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "j_lc", "indices", "access_ngh", "ngh_ind", "ld_feat", "combine" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_i", "active");
    dfg.setTheTailNode("loop_j", "combine");

    dfg.plotDot();
}


void GCN_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{

}


