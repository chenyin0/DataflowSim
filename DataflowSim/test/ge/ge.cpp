#include "./ge.h"

using namespace DFSimTest;

Dfg Ge_Test::dfg;

vector<int> Ge_Test::A;
vector<int> Ge_Test::x;
uint Ge_Test::matrix_size = 60;
uint Ge_Test::baseAddr_A = 0;
uint Ge_Test::baseAddr_x = 56847;

void Ge_Test::generateData()
{
    A.resize(matrix_size * (matrix_size + 1));
    x.resize(matrix_size + 1);

    for (size_t i = 0; i < A.size(); ++i)
    {
        A[i] = i;
    }

    for (size_t i = 0; i < x.size(); ++i)
    {
        x[i] = i;
    }
}

void Ge_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_t", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j", "Loop", "Null"),
         make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k", "Loop", "Null"),
         make_tuple<string, string, string>("loop_i1", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j1", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_t", { "loop_j", "loop_i1" });
    dfg.controlTree.addLowerControlRegion("loop_j", { "loop_i" });
    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_k" });
    dfg.controlTree.addLowerControlRegion("loop_i1", { "loop_j1" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("n", "Const", Ge_Test::matrix_size);
    // loop_t
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "t" });
    dfg.addNode("t", "Loop_head", {}, { "begin" });
    dfg.addNode("t_lc1", "Nop", { "t" });
    dfg.addNode("t_lc2", "Nop", { "t" });

    // loop_i
    //dfg.addNode("begin", "Nop");
    //dfg.addNode("end", "Nop", {}, { "j" });
    dfg.addNode("j", "Loop_head", {}, { "t_lc1" });
    dfg.addNode("j_lc", "Nop", { "j" });
    dfg.addNode("j_n", "Mul", { "j", "n" });
    dfg.addNode("j_n_shadow", "Nop", { "j_n" });
    dfg.addNode("addr_jj", "Add", { "j_n", "j" });

    // loop_i
    dfg.addNode("i", "Loop_head", {}, { "j_lc" });
    dfg.addNode("i_lc", "Nop", { "i" });
    dfg.addNode("i_n", "Mul", { "i", "n" });
    dfg.addNode("i_n_shadow", "Nop", { "i_n" });
    dfg.addNode("addr_ij", "Add", { "i_n", "j_lc" });
    dfg.addNode("A_ij", "Load", { "addr_ij" }, &A, baseAddr_A);
    dfg.addNode("A_jj", "Load", { "addr_jj" }, &A, baseAddr_A);
    dfg.addNode("c", "Div", { "A_ij", "A_jj" });

    // loop_k
    dfg.addNode("k", "Loop_head", {}, { "i_lc" });
    dfg.addNode("addr_ik", "Add", { "i_n_shadow", "k" });
    dfg.addNode("addr_jk", "Add", { "j_n_shadow", "k" });
    dfg.addNode("A_ik", "Load", { "addr_ik" }, &A, baseAddr_A);
    dfg.addNode("A_jk", "Load", { "addr_jk" }, &A, baseAddr_A);
    dfg.addNode("c_A_jk", "Mul", { "c", "A_jk" });
    dfg.addNode("A_ik_update", "Sub", { "A_ik", "c_A_jk" });
    //dfg.addNode("A_ik_update_store", "Store", { "addr_ik", "A_ik_update" }, &A, baseAddr_A);

    // loop_i1
    dfg.addNode("i1", "Loop_head", {}, { "t_lc2" });
    dfg.addNode("i1_lc", "Nop", { "i1" });
    dfg.addNode("i1_n", "Mul", { "i1", "n" });
    dfg.addNode("i1_n_shadow", "Nop", { "i1_n" });
    dfg.addNode("addr_i1n1", "Add", { "i1_n", "n" });
    dfg.addNode("addr_i1i1", "Add", { "i1_n", "i1" });
    dfg.addNode("A_i1n1", "Load", { "addr_i1n1" }, &A, baseAddr_A);
    dfg.addNode("a_sum_shadow", "Nop", { "sum_update" });
    dfg.addNode("a_sum", "Sub", { "A_i1n1", "a_sum_shadow" });
    dfg.addNode("A_i1i1", "Load", { "addr_i1i1" }, &A, baseAddr_A);
    dfg.addNode("x_i1", "Div", { "a_sum", "A_i1i1" });
    //dfg.addNode("x_i1_store", "Store", { "i1", "x_i1" }, &x, baseAddr_x);

    // loop_j1
    dfg.addNode("j1", "Loop_head", {}, { "i1_lc" });
    dfg.addNode("addr_i1j1", "Add", { "i1_n_shadow", "j1" });
    dfg.addNode("A_i1j1", "Load", { "addr_i1j1" }, &A, baseAddr_A);
    dfg.addNode("x_j1", "Load", { "j1" }, &x, baseAddr_x);
    dfg.addNode("A_i1j1_x_j1", "Mul", { "A_i1j1", "x_j1" });
    dfg.addNode("sum_update", "Add", { "A_i1j1_x_j1" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_t", { "begin", "end", "t", "t_lc1", "t_lc2" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "j_lc", "j_n", "j_n_shadow", "addr_jj" });
    dfg.addNodes2CtrlTree("loop_i", { "i", "i_lc", "i_n", "i_n_shadow", "addr_ij", "A_ij", "A_jj", "c" });
    dfg.addNodes2CtrlTree("loop_k", { "k", "addr_ik", "addr_jk", "A_ik", "A_jk", "c_A_jk","A_ik_update"/*, "A_ik_update_store"*/ });
    dfg.addNodes2CtrlTree("loop_i1", { "i1", "i1_lc", "i1_n", "i1_n_shadow", "addr_i1n1", "addr_i1i1", "A_i1n1", "a_sum_shadow", "a_sum", "A_i1i1", "x_i1"/*, "x_i1_store"*/ });
    dfg.addNodes2CtrlTree("loop_j1", { "j1", "addr_i1j1", "A_i1j1", "x_j1", "A_i1j1_x_j1", "sum_update" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_t", "i1");
    dfg.setTheTailNode("loop_j", "i");
    dfg.setTheTailNode("loop_i", "k");
    dfg.setTheTailNode("loop_k", "A_ik_update");
    dfg.setTheTailNode("loop_i1", "x_i1");
    dfg.setTheTailNode("loop_j1", "sum_update");

    dfg.plotDot();
}

// Graph partition
void Ge_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_n")->subgraphId = 0;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 0;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 0;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_c")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 0;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 0;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 0;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 0;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 0;

        chanGraph.getNode("Lc_i1")->subgraphId = 0;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 0;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 0;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 0;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_j1")->subgraphId = 0;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 0;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 0;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 0;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 0;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 0;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 0;

        break;
    }
    case 2:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_t")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_c")->subgraphId = 0;

        //chanGraph.getNode("Lc_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_ik")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jk")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ik")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_jk")->subgraphId = 0;
        //chanGraph.getNode("Chan_c_A_jk")->subgraphId = 0;
        //chanGraph.getNode("Chan_A_ik_update")->subgraphId = 0;
        ////chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 0;

        //chanGraph.getNode("Lc_i1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_n")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_i1n1")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_i1i1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_lc")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 1;

        //chanGraph.getNode("Lc_j1")->subgraphId = 1;
        //chanGraph.getNode("Lse_x_j1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_i1j1")->subgraphId = 1;
        //chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 1;
        //chanGraph.getNode("Chan_sum_update")->subgraphId = 1;
        //chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 1;
        //chanGraph.getNode("Chan_a_sum")->subgraphId = 1;
        //chanGraph.getNode("Chan_x_i1")->subgraphId = 1;
        ////chanGraph.getNode("Lse_x_i1_store")->subgraphId = 1;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_c")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 1;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 1;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 1;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 1;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 1;

        chanGraph.getNode("Lc_i1")->subgraphId = 1;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 1;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 1;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 1;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_j1")->subgraphId = 1;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 1;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 1;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 1;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 1;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 1;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 1;

        break;
    }
    case 3:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_t")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_c")->subgraphId = 0;

        //chanGraph.getNode("Lc_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_ik")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jk")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ik")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_jk")->subgraphId = 0;
        //chanGraph.getNode("Chan_c_A_jk")->subgraphId = 0;
        //chanGraph.getNode("Chan_A_ik_update")->subgraphId = 0;
        ////chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 0;

        //chanGraph.getNode("Lc_i1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_n")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_i1n1")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_i1i1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_lc")->subgraphId = 1;
        //chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 1;

        //chanGraph.getNode("Lc_j1")->subgraphId = 2;
        //chanGraph.getNode("Lse_x_j1")->subgraphId = 2;
        //chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_i1j1")->subgraphId = 2;
        //chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 2;
        //chanGraph.getNode("Chan_sum_update")->subgraphId = 2;
        //chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 2;
        //chanGraph.getNode("Chan_a_sum")->subgraphId = 2;
        //chanGraph.getNode("Chan_x_i1")->subgraphId = 2;
        ////chanGraph.getNode("Lse_x_i1_store")->subgraphId = 2;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_c")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 1;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 1;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 1;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 1;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 1;

        chanGraph.getNode("Lc_i1")->subgraphId = 2;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 2;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 2;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 2;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 2;

        chanGraph.getNode("Lc_j1")->subgraphId = 2;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 2;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 2;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 2;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 2;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 2;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 2;

        break;
    }
    case 4:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_t")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_ij")->subgraphId = 0;
        //chanGraph.getNode("Lse_A_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_c")->subgraphId = 0;

        //chanGraph.getNode("Lc_k")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        //chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_ik")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_jk")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_ik")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_jk")->subgraphId = 1;
        //chanGraph.getNode("Chan_c_A_jk")->subgraphId = 1;
        //chanGraph.getNode("Chan_A_ik_update")->subgraphId = 1;
        ////chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 1;

        //chanGraph.getNode("Lc_i1")->subgraphId = 2;
        //chanGraph.getNode("Chan_i1_n")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_i1n1")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_i1i1")->subgraphId = 2;
        //chanGraph.getNode("Chan_i1_lc")->subgraphId = 2;
        //chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 2;

        //chanGraph.getNode("Lc_j1")->subgraphId = 3;
        //chanGraph.getNode("Lse_x_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_sum_update")->subgraphId = 3;
        //chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 3;
        //chanGraph.getNode("Chan_a_sum")->subgraphId = 3;
        //chanGraph.getNode("Chan_x_i1")->subgraphId = 3;
        ////chanGraph.getNode("Lse_x_i1_store")->subgraphId = 3;

        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_t")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_n")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_ij")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_ij")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_jj")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 1;
        //chanGraph.getNode("Chan_c")->subgraphId = 1;

        //chanGraph.getNode("Lc_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_ik")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_jk")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_ik")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_jk")->subgraphId = 2;
        //chanGraph.getNode("Chan_c_A_jk")->subgraphId = 2;
        //chanGraph.getNode("Chan_A_ik_update")->subgraphId = 2;
        ////chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 2;

        //chanGraph.getNode("Lc_i1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1n1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1i1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_lc")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 3;

        //chanGraph.getNode("Lc_j1")->subgraphId = 3;
        //chanGraph.getNode("Lse_x_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 3;
        //chanGraph.getNode("Chan_sum_update")->subgraphId = 3;
        //chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 3;
        //chanGraph.getNode("Chan_a_sum")->subgraphId = 3;
        //chanGraph.getNode("Chan_x_i1")->subgraphId = 3;
        ////chanGraph.getNode("Lse_x_i1_store")->subgraphId = 3;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 1;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_c")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 2;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 2;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 2;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 2;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 2;

        chanGraph.getNode("Lc_i1")->subgraphId = 3;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 3;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 3;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 3;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 3;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 3;

        chanGraph.getNode("Lc_j1")->subgraphId = 3;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 3;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 3;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 3;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 3;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 3;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 3;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 3;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 3;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 3;

        break;
    }
    case 5:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_t")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        //chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr_jj")->subgraphId = 0;
        //chanGraph.getNode("Chan_j_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_n")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr_ij")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_ij")->subgraphId = 1;
        //chanGraph.getNode("Lse_A_jj")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        //chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 1;
        //chanGraph.getNode("Chan_c")->subgraphId = 1;

        //chanGraph.getNode("Lc_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_ik")->subgraphId = 2;
        //chanGraph.getNode("Chan_addr_jk")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_ik")->subgraphId = 2;
        //chanGraph.getNode("Lse_A_jk")->subgraphId = 2;
        //chanGraph.getNode("Chan_c_A_jk")->subgraphId = 2;
        //chanGraph.getNode("Chan_A_ik_update")->subgraphId = 2;
        ////chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 2;

        //chanGraph.getNode("Lc_i1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 3;
        //chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1n1")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_i1i1")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_lc")->subgraphId = 3;
        //chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 3;

        //chanGraph.getNode("Lc_j1")->subgraphId = 4;
        //chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 4;
        //chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 4;
        //chanGraph.getNode("Lse_x_j1")->subgraphId = 4;
        //chanGraph.getNode("Lse_A_i1j1")->subgraphId = 4;
        //chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 4;
        //chanGraph.getNode("Chan_sum_update")->subgraphId = 4;
        //chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 4;
        //chanGraph.getNode("Chan_a_sum")->subgraphId = 4;
        //chanGraph.getNode("Chan_x_i1")->subgraphId = 4;
        ////chanGraph.getNode("Lse_x_i1_store")->subgraphId = 4;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_c")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 3;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 3;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 3;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 3;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 3;

        chanGraph.getNode("Lc_i1")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 4;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 4;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 4;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 4;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 4;

        chanGraph.getNode("Lc_j1")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 4;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 4;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 4;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 4;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 4;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 4;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 4;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 4;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 4;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 4;

        break;
    }
    case 6:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_c")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 3;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 3;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 3;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 3;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 3;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 3;

        chanGraph.getNode("Lc_i1")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 4;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 4;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 4;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 4;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 4;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 4;

        chanGraph.getNode("Lc_j1")->subgraphId = 5;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 5;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 5;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 5;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 5;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 5;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 5;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 5;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 5;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 5;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 6;

        break;
    }
    case 7:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n")->subgraphId = 1;
        chanGraph.getNode("Chan_j_n_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_addr_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_j_lc_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_jj_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_addr_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_ij")->subgraphId = 2;
        chanGraph.getNode("Lse_A_jj")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_j_n_shadow_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_n_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_c")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_n_shadow_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_j_n_shadow_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_c_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_ik")->subgraphId = 3;
        chanGraph.getNode("Chan_addr_jk")->subgraphId = 3;
        chanGraph.getNode("Lse_A_ik")->subgraphId = 4;
        chanGraph.getNode("Lse_A_jk")->subgraphId = 4;
        chanGraph.getNode("Chan_c_A_jk")->subgraphId = 4;
        chanGraph.getNode("Chan_A_ik_update")->subgraphId = 4;
        //chanGraph.getNode("Lse_A_ik_update_store")->subgraphId = 4;

        chanGraph.getNode("Lc_i1")->subgraphId = 5;
        chanGraph.getNode("Chan_i1_n")->subgraphId = 5;
        chanGraph.getNode("Chan_addr_i1n1")->subgraphId = 5;
        chanGraph.getNode("Chan_addr_i1i1")->subgraphId = 5;
        chanGraph.getNode("Lse_A_i1n1")->subgraphId = 5;
        chanGraph.getNode("Lse_A_i1i1")->subgraphId = 5;
        chanGraph.getNode("Chan_i1_lc")->subgraphId = 5;
        chanGraph.getNode("Chan_i1_n_shadow")->subgraphId = 5;

        chanGraph.getNode("Lc_j1")->subgraphId = 6;
        chanGraph.getNode("Chan_i1_n_shadow_scatter_loop_j1")->subgraphId = 6;
        chanGraph.getNode("Chan_addr_i1j1")->subgraphId = 6;
        chanGraph.getNode("Lse_x_j1")->subgraphId = 6;
        chanGraph.getNode("Lse_A_i1j1")->subgraphId = 6;
        chanGraph.getNode("Chan_A_i1j1_x_j1")->subgraphId = 6;
        chanGraph.getNode("Chan_sum_update")->subgraphId = 6;
        chanGraph.getNode("Chan_a_sum_shadow")->subgraphId = 6;
        chanGraph.getNode("Chan_a_sum")->subgraphId = 6;
        chanGraph.getNode("Chan_x_i1")->subgraphId = 6;
        //chanGraph.getNode("Lse_x_i1_store")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}