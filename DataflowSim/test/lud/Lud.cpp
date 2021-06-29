#include "./Lud.h"

using namespace DFSimTest;

Dfg LudTest::dfg;

vector<int> LudTest::matrix;
uint LudTest::matrix_size = 50;
uint LudTest::baseAddr = 0;

uint LudTest::Base_outer_loop_speedup = 1;
uint LudTest::Base_inner_loop_speedup = 4;

uint LudTest::DGSF_outer_loop_speedup = 2;
uint LudTest::DGSF_inner_loop_speedup = 4;

uint LudTest::DGSF_outer_loop_buffer_size = 2;

void LudTest::generateData()
{
    matrix.resize(matrix_size * matrix_size);

    for (auto& i : matrix)
    {
        i = 1;
    }
}

void LudTest::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j1", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k1", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j2", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k2", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_j1", "loop_j2" });
    dfg.controlTree.addLowerControlRegion("loop_j1", { "loop_k1" });
    dfg.controlTree.addLowerControlRegion("loop_j2", { "loop_k2" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("SIZE", "Const", LudTest::matrix_size);
    // loop_i
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "i" });
    dfg.addNode("i", "Loop_head", {}, { "begin" });
    dfg.addNode("i_lc1", "Nop", { "i" });
    dfg.addNode("i_lc2", "Nop", { "i" });

    // loop_j1
    dfg.addNode("j1", "Loop_head", {}, { "i_lc1" });
    dfg.addNode("j1_lc", "Nop", { "j1" }, { });
    dfg.addNode("i_size", "Mul", { "i_lc1", "SIZE" }, { "j1" });
    dfg.addNode("i_size_shadow", "Nop", { "i_size" });
    dfg.addNode("i_size_j", "Add", { "i_size", "j1" });
    dfg.addNode("sum_j1", "Load", { "i_size_j" }, &matrix, baseAddr);
    dfg.addNode("sum_j1_shadow", "Nop", { "sum_j1"});
    dfg.addNode("sum_update_k1_drain", "Nop", { "sum_update_k1" });
    dfg.addNode("a_update_j1", "Store", { "i_size_j", "sum_update_k1_drain" }, &matrix, baseAddr);

    // loop_k1
    dfg.addNode("k1", "Loop_head", {}, { "j1_lc" });
    dfg.addNode("i_size_k", "Add", { "i_size_shadow", "k1" });
    dfg.addNode("k1_size", "Mul", { "k1", "SIZE" });
    dfg.addNode("k_size_j", "Add", { "k1_size", "j1_lc" });
    dfg.addNode("a1", "Load", { "i_size_k"}, &matrix, baseAddr);
    dfg.addNode("a2", "Load", { "k_size_j" }, &matrix, baseAddr);
    dfg.addNode("a1_a2", "Mul", { "a1", "a2" });
    dfg.addNode("sum_update_k1", "Sub", { "sum_j1_shadow", "a1_a2" });

    // loop_j2
    dfg.addNode("j2", "Loop_head", {}, { "i_lc2" });
    dfg.addNode("j2_lc", "Nop", { "j2" }, { });
    dfg.addNode("j_size", "Mul", { "j2", "SIZE" });
    dfg.addNode("j_size_shadow", "Nop", { "j_size" });
    dfg.addNode("j_size_i", "Add", { "j_size", "i_lc2" });
    dfg.addNode("sum_j2", "Load", { "j_size_i" }, &matrix, baseAddr);
    dfg.addNode("sum_j2_shadow", "Nop", { "sum_j2" });
    dfg.addNode("i_size_", "Mul", { "i_lc2", "SIZE" }, {"j2"});
    dfg.addNode("i_size_i", "Add", { "i_size_", "i_lc2" });
    dfg.addNode("a_i_size_i", "Load", { "i_size_i" }, &matrix, baseAddr);
    dfg.addNode("sum_update_k2_drain", "Nop", { "sum_update_k2" });
    dfg.addNode("sum_div", "Div", { "sum_update_k2_drain", "a_i_size_i" });
    dfg.addNode("a_update_j2", "Store", { "j_size_i", "sum_div" }, &matrix, baseAddr);

    // loop_k2
    dfg.addNode("k2", "Loop_head", {}, { "j2_lc" });
    dfg.addNode("j_size_k", "Add", { "j_size_shadow", "k2" });
    dfg.addNode("k2_size", "Mul", { "k2", "SIZE" });
    dfg.addNode("k_size_i", "Add", { "k2_size", "i_lc2" });
    dfg.addNode("a3", "Load", { "j_size_k" }, &matrix, baseAddr);
    dfg.addNode("a4", "Load", { "k_size_i" }, &matrix, baseAddr);
    dfg.addNode("a3_a4", "Mul", { "a3", "a4" });
    dfg.addNode("sum_update_k2", "Sub", { "sum_j2_shadow", "a3_a4" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc1", "i_lc2" });
    dfg.addNodes2CtrlTree("loop_j1", { "j1", "j1_lc", "i_size", "i_size_shadow", "i_size_j", "sum_j1", "sum_j1_shadow", "a_update_j1", "sum_update_k1_drain" });
    dfg.addNodes2CtrlTree("loop_k1", { "k1", "i_size_k", "k1_size", "k_size_j", "a1", "a2", "a1_a2", "sum_update_k1" });
    dfg.addNodes2CtrlTree("loop_j2", { "j2", "j2_lc", "j_size", "j_size_shadow", "j_size_i", "sum_j2","sum_j2_shadow", "i_size_", "i_size_i",
        "a_i_size_i", "sum_div", "a_update_j2", "sum_update_k2_drain" });
    dfg.addNodes2CtrlTree("loop_k2", { "k2", "j_size_k", "k2_size", "k_size_i", "a3", "a4", "a3_a4", "sum_update_k2" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_i", "j2");
    dfg.setTheTailNode("loop_j1", "a_update_j1");
    dfg.setTheTailNode("loop_k1", "sum_update_k1");
    dfg.setTheTailNode("loop_j2", "a_update_j2");
    dfg.setTheTailNode("loop_k2", "sum_update_k2");

    dfg.plotDot();
}

// Graph partition
void LudTest::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 0;
        chanGraph.getNode("Lse_a2")->subgraphId = 0;
        chanGraph.getNode("Lse_a1")->subgraphId = 0;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 0;

        chanGraph.getNode("Lc_j2")->subgraphId = 0;
        chanGraph.getNode("Chan_j_size")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 0;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 0;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 0;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 0;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 0;
        chanGraph.getNode("Lse_a3")->subgraphId = 0;
        chanGraph.getNode("Lse_a4")->subgraphId = 0;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 0;
        chanGraph.getNode("Lse_a2")->subgraphId = 0;
        chanGraph.getNode("Lse_a1")->subgraphId = 0;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 0;

        chanGraph.getNode("Lc_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 1;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 1;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 1;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 1;
        chanGraph.getNode("Lse_a3")->subgraphId = 1;
        chanGraph.getNode("Lse_a4")->subgraphId = 1;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 0;
        chanGraph.getNode("Lse_a2")->subgraphId = 0;
        chanGraph.getNode("Lse_a1")->subgraphId = 0;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 0;

        chanGraph.getNode("Lc_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 1;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 1;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 1;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 2;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 2;
        chanGraph.getNode("Lse_a3")->subgraphId = 2;
        chanGraph.getNode("Lse_a4")->subgraphId = 2;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 1;
        chanGraph.getNode("Lse_a2")->subgraphId = 1;
        chanGraph.getNode("Lse_a1")->subgraphId = 1;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 1;

        chanGraph.getNode("Lc_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 2;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 2;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 2;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 2;

        chanGraph.getNode("Lc_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 3;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 3;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 3;
        chanGraph.getNode("Lse_a3")->subgraphId = 3;
        chanGraph.getNode("Lse_a4")->subgraphId = 3;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 3;

        break;
    }
    case 5:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 2;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 2;
        chanGraph.getNode("Lse_a2")->subgraphId = 2;
        chanGraph.getNode("Lse_a1")->subgraphId = 2;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 2;

        chanGraph.getNode("Lc_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 3;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 3;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 3;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 3;

        chanGraph.getNode("Lc_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 4;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 4;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 4;
        chanGraph.getNode("Lse_a3")->subgraphId = 4;
        chanGraph.getNode("Lse_a4")->subgraphId = 4;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 4;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 4;*/


        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 1;
        chanGraph.getNode("Lse_a2")->subgraphId = 1;
        chanGraph.getNode("Lse_a1")->subgraphId = 1;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 1;

        chanGraph.getNode("Lc_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 2;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 2;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 3;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 3;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 3;

        chanGraph.getNode("Lc_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 4;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 4;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 4;
        chanGraph.getNode("Lse_a3")->subgraphId = 4;
        chanGraph.getNode("Lse_a4")->subgraphId = 4;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 4;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 4;

        break;
    }
    case 6:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 2;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 2;
        chanGraph.getNode("Lse_a2")->subgraphId = 2;
        chanGraph.getNode("Lse_a1")->subgraphId = 2;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 2;

        chanGraph.getNode("Lc_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 3;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 3;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 4;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 4;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 4;

        chanGraph.getNode("Lc_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 5;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 5;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 5;
        chanGraph.getNode("Lse_a3")->subgraphId = 5;
        chanGraph.getNode("Lse_a4")->subgraphId = 5;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 5;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 5;


        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j1")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_size")->subgraphId = 0;
        //chanGraph.getNode("Chan_j1_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_size_j")->subgraphId = 0;
        //chanGraph.getNode("Lse_sum_j1")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 0;

        //chanGraph.getNode("Lc_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_k1_size")->subgraphId = 1;
        //chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_k_size_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_size_k")->subgraphId = 1;
        //chanGraph.getNode("Lse_a2")->subgraphId = 2;
        //chanGraph.getNode("Lse_a1")->subgraphId = 2;
        //chanGraph.getNode("Chan_a1_a2")->subgraphId = 2;
        //chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 2;
        //chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 2;
        //chanGraph.getNode("Lse_a_update_j1")->subgraphId = 2;

        //chanGraph.getNode("Lc_j2")->subgraphId = 3;
        //chanGraph.getNode("Chan_j_size")->subgraphId = 3;
        //chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 3;
        //chanGraph.getNode("Chan_i_size_")->subgraphId = 3;
        //chanGraph.getNode("Chan_j_size_i")->subgraphId = 3;
        //chanGraph.getNode("Chan_i_size_i")->subgraphId = 3;
        //chanGraph.getNode("Lse_sum_j2")->subgraphId = 3;
        //chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 3;
        //chanGraph.getNode("Chan_j2_lc")->subgraphId = 3;
        //chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 3;
        //chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 3;

        //chanGraph.getNode("Lc_k2")->subgraphId = 4;
        //chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 4;
        //chanGraph.getNode("Chan_k2_size")->subgraphId = 4;
        //chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 4;
        //chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 4;
        //chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 4;
        //chanGraph.getNode("Chan_j_size_k")->subgraphId = 4;
        //chanGraph.getNode("Chan_k_size_i")->subgraphId = 4;
        //chanGraph.getNode("Lse_a3")->subgraphId = 5;
        //chanGraph.getNode("Lse_a4")->subgraphId = 5;
        //chanGraph.getNode("Chan_a3_a4")->subgraphId = 5;
        //chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 5;
        //chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 5;
        //chanGraph.getNode("Chan_sum_div")->subgraphId = 5;
        //chanGraph.getNode("Lse_a_update_j2")->subgraphId = 5;

        break;
    }
    case 7:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 2;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 2;
        chanGraph.getNode("Lse_a2")->subgraphId = 3;
        chanGraph.getNode("Lse_a1")->subgraphId = 3;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 3;

        chanGraph.getNode("Lc_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 4;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 4;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 4;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 4;

        chanGraph.getNode("Lc_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 5;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 5;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 5;
        chanGraph.getNode("Lse_a3")->subgraphId = 6;
        chanGraph.getNode("Lse_a4")->subgraphId = 6;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 6;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 6;*/


        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 2;

        chanGraph.getNode("Lc_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 3;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 3;
        chanGraph.getNode("Lse_a2")->subgraphId = 3;
        chanGraph.getNode("Lse_a1")->subgraphId = 3;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 3;

        chanGraph.getNode("Lc_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 4;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 5;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 5;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 5;

        chanGraph.getNode("Lc_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 6;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 6;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 6;
        chanGraph.getNode("Lse_a3")->subgraphId = 6;
        chanGraph.getNode("Lse_a4")->subgraphId = 6;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 6;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 6;

        break;
    }
    case 8:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 2;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 2;
        chanGraph.getNode("Lse_a2")->subgraphId = 3;
        chanGraph.getNode("Lse_a1")->subgraphId = 3;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 3;

        chanGraph.getNode("Lc_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 4;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 4;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 4;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 5;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 5;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 5;

        chanGraph.getNode("Lc_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 6;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 6;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 6;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 6;
        chanGraph.getNode("Lse_a3")->subgraphId = 7;
        chanGraph.getNode("Lse_a4")->subgraphId = 7;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 7;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 7;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 7;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 7;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 7;

        break;
    }
    case 9:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc1_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size")->subgraphId = 1;
        chanGraph.getNode("Chan_j1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i_size_j")->subgraphId = 1;
        chanGraph.getNode("Lse_sum_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_size_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_j1_shadow")->subgraphId = 2;

        chanGraph.getNode("Lc_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_k1_size")->subgraphId = 3;
        chanGraph.getNode("Chan_j1_lc_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_shadow_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_j1_shadow_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_k_size_j")->subgraphId = 3;
        chanGraph.getNode("Chan_i_size_k")->subgraphId = 3;
        chanGraph.getNode("Lse_a2")->subgraphId = 4;
        chanGraph.getNode("Lse_a1")->subgraphId = 4;
        chanGraph.getNode("Chan_a1_a2")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k1")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update_k1_drain")->subgraphId = 4;
        chanGraph.getNode("Lse_a_update_j1")->subgraphId = 4;

        chanGraph.getNode("Lc_j2")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc2_scatter_loop_j2")->subgraphId = 5;
        chanGraph.getNode("Chan_i_size_")->subgraphId = 5;
        chanGraph.getNode("Chan_j_size_i")->subgraphId = 5;
        chanGraph.getNode("Chan_i_size_i")->subgraphId = 5;
        chanGraph.getNode("Lse_sum_j2")->subgraphId = 6;
        chanGraph.getNode("Lse_a_i_size_i")->subgraphId = 6;
        chanGraph.getNode("Chan_j2_lc")->subgraphId = 6;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_j2_shadow")->subgraphId = 6;

        chanGraph.getNode("Lc_k2")->subgraphId = 7;
        chanGraph.getNode("Chan_j_size_shadow")->subgraphId = 7;
        chanGraph.getNode("Chan_k2_size")->subgraphId = 7;
        chanGraph.getNode("Chan_j_size_shadow_scatter_loop_k2")->subgraphId = 7;
        chanGraph.getNode("Chan_i_lc2_relay_loop_j2_scatter_loop_k2")->subgraphId = 7;
        chanGraph.getNode("Chan_sum_j2_shadow_scatter_loop_k2")->subgraphId = 7;
        chanGraph.getNode("Chan_j_size_k")->subgraphId = 7;
        chanGraph.getNode("Chan_k_size_i")->subgraphId = 7;
        chanGraph.getNode("Lse_a3")->subgraphId = 8;
        chanGraph.getNode("Lse_a4")->subgraphId = 8;
        chanGraph.getNode("Chan_a3_a4")->subgraphId = 8;
        chanGraph.getNode("Chan_sum_update_k2")->subgraphId = 8;
        chanGraph.getNode("Chan_sum_update_k2_drain")->subgraphId = 8;
        chanGraph.getNode("Chan_sum_div")->subgraphId = 8;
        chanGraph.getNode("Lse_a_update_j2")->subgraphId = 8;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}