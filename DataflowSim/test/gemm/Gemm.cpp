#include "./Gemm.h"

using namespace DFSimTest;

Dfg GemmTest::dfg;
//ControlTree GemmTest::dfgControlTree;
//ChanGraph GemmTest::chanGraph;
//ControlTree GemmTest::chanGraphControlTree;

const uint GemmTest::matrix_width = 10;
const uint GemmTest::matrix_height = matrix_width;
const uint GemmTest::block_size = 5;

// Address map: 
// 0x00 -> matrix_m1 -> matrix_m2 -> matrix_paritial
const uint GemmTest::m1_BaseAddr = 0;
const uint GemmTest::m2_BaseAddr = 5280;// matrix_width* matrix_height;
const uint GemmTest::partialSum_BaseAddr = 26890;// matrix_width* matrix_height * 2;
const uint GemmTest::prod_BaseAddr = 26890;// matrix_width * matrix_height * 2;

vector<vector<int>> GemmTest::m1;
vector<vector<int>> GemmTest::m2;
vector<vector<int>> GemmTest::result;

vector<int> GemmTest::m1_;
vector<int> GemmTest::m2_;
vector<int> GemmTest::prod_;

// Performance parameter
// Base
uint GemmTest::Base_loop_j_speedup = 1;

// DGSF
uint GemmTest::DGSF_loop_k_speedup = 1;
uint GemmTest::DGSF_loop_j_speedup = 4;


void GemmTest::generateData()
{
    uint size = matrix_width * matrix_height;
    m1.resize(matrix_height);
    m2.resize(matrix_height);
    result.resize(matrix_height);

    for (size_t i = 0; i < matrix_height; ++i)
    {
        m1[i].resize(matrix_width);
        m2[i].resize(matrix_width);
        result[i].resize(matrix_width);

        for (size_t j = 0; j < matrix_width; ++j)
        {
            m1[i][j] = i * matrix_width + j;
            m2[i][j] = i * matrix_width + j;
        }
    }

    // Auto-sim
    uint size_ = matrix_width * matrix_height;
    m1_.resize(size);
    m2_.resize(size);
    prod_.resize(size);

    for (size_t i = 0; i < size; ++i)
    {
        m1_[i] = i;
        m2_[i] = i;
    }
}

void GemmTest::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_jj", "Loop", "Null"),
         make_tuple<string, string, string>("loop_kk", "Loop", "Null"),
         make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j", "Loop", "Null"),
        });

    dfg.controlTree.addLowerControlRegion("loop_jj", { "loop_kk" });
    dfg.controlTree.addLowerControlRegion("loop_kk", { "loop_i" });
    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_k" });
    dfg.controlTree.addLowerControlRegion("loop_k", { "loop_j" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("row_size", "Const", GemmTest::matrix_width);
    // loop_jj
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, {"jj"});
    dfg.addNode("jj", "Loop_head", {}, {"begin"});
    dfg.addNode("jj_lc", "Nop", {"jj"}, {});
    //dynamic_cast<Dfg_Node*>(dfg.getNode("jj"))->loop_info=make_tuple("0","","","")

    // loop_kk
    dfg.addNode("kk", "Loop_head", {}, {"jj_lc"});
    dfg.addNode("kk_lc", "Nop", {"kk"}, {});

    // loop_i
    dfg.addNode("i", "Loop_head", {}, { "kk_lc" });
    dfg.addNode("i_lc", "Nop", {"i"}, {});
    dfg.addNode("i_row", "Mul", {"i", "row_size"});

    // loop_k
    dfg.addNode("k", "Loop_head", {}, { "i_lc" });
    dfg.addNode("k_lc", "Nop", {"k"}, {});
    dfg.addNode("k_kk", "Add", {"k", "kk_lc"});
    dfg.addNode("k_row", "Mul", {"k_kk", "row_size"});
    dfg.addNode("m1_addr", "Add", {"i_row", "k_kk"});
    dfg.addNode("temp_x", "Load", {"m1_addr"}, &m1_, m1_BaseAddr);
    dfg.addNode("m1_data", "Nop", { "temp_x" });

    // loop_j
    dfg.addNode("j", "Loop_head", {}, { "k_lc" });
    dfg.addNode("j_jj", "Add", {"j", "jj_lc"});
    dfg.addNode("m2_addr", "Add", {"k_row", "j_jj"});
    dfg.addNode("m2_data", "Load", {"m2_addr"}, &m2_, m2_BaseAddr);
    dfg.addNode("mul", "Mul", {"m1_data", "m2_data"});
    dfg.addNode("prod_addr", "Add", {"i_row", "j_jj"});
    dfg.addNode("prod_data", "Load", { "prod_addr" }, &prod_, prod_BaseAddr);
    dfg.addNode("prod_data_update", "Add", {"prod_data", "mul"});
    dfg.addNode("prod_data_update_st", "Store", {"prod_addr", "prod_data_update"}, &prod_, prod_BaseAddr);

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_jj", { "begin", "end", "jj", "jj_lc" });
    dfg.addNodes2CtrlTree("loop_kk", { "kk", "kk_lc" });
    dfg.addNodes2CtrlTree("loop_i", { "i", "i_lc", "i_row" });
    dfg.addNodes2CtrlTree("loop_k", { "k", "k_lc", "k_kk", "k_row", "m1_addr", "temp_x", "m1_data" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "j_jj", "m2_addr", "m2_data", "mul", "prod_addr", "prod_data", "prod_data_update", "prod_data_update_st" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_jj", "kk");
    dfg.setTheTailNode("loop_kk", "i");
    dfg.setTheTailNode("loop_i", "k");
    dfg.setTheTailNode("loop_k", "j");
    dfg.setTheTailNode("loop_j", "prod_data_update");

    dfg.plotDot();
}