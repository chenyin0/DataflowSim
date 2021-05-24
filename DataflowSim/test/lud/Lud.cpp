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
    dfg.addNode("j1_lc", "Nop", {}, { "j1" });
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
    dfg.addNode("j2_lc", "Nop", {}, { "j2" });
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