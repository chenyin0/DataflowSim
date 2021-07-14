#include "./cholesky.h"

using namespace DFSimTest;

Dfg Cholesky_Test::dfg;

vector<int> Cholesky_Test::A;
//vector<int> Ge_Test::x;
uint Cholesky_Test::matrix_size = 60;
uint Cholesky_Test::baseAddr_A = 0;
//uint Ge_Test::baseAddr_x = 56847;

void Cholesky_Test::generateData()
{
    A.resize(matrix_size * matrix_size);

    for (size_t i = 0; i < A.size(); ++i)
    {
        A[i] = rand() % 100 + 1;
    }
}

void Cholesky_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k", "Loop", "Null"),
         make_tuple<string, string, string>("loop_k1", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_j", "loop_k1" });
    dfg.controlTree.addLowerControlRegion("loop_j", { "loop_k" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    // loop_i
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "i" });
    dfg.addNode("i", "Loop_head", {}, { "begin" });
    dfg.addNode("i_lc1", "Nop", { "i" });
    dfg.addNode("i_lc2", "Nop", { "i" });
    dfg.addNode("i_base", "Mul", { "i" });
    dfg.addNode("i_base_k", "Nop", { "i_base" });
    dfg.addNode("i_base_k1", "Nop", { "i_base" });
    dfg.addNode("Aii_addr", "Add", { "i_base", "i" });
    dfg.addNode("Aii_sqrt", "Sqrt", { "Aii_update" });

    // loop_j
    dfg.addNode("j", "Loop_head", {}, { "i_lc1" });
    dfg.addNode("j_lc", "Nop", { "j" });
    dfg.addNode("j_base", "Mul", { "j" });
    dfg.addNode("j_base_shadow", "Nop", { "j_base" });
    dfg.addNode("Aij_addr", "Add", { "i_base_k", "j" });
    dfg.addNode("Aij", "Load", { "Aij_addr" }, &A, baseAddr_A);
    dfg.addNode("Aij_shadow", "Nop", { "Aij" });
    dfg.addNode("Ajj_addr", "Add", { "j_base", "j" });
    dfg.addNode("Ajj", "Load", { "Ajj_addr" }, &A, baseAddr_A);
    dfg.addNode("Aij_update_shadow", "Nop", { "Aij_update" });
    dfg.addNode("Aij_div", "Div", { "Aij_update_shadow", "Ajj" });

    // loop_k
    dfg.addNode("k", "Loop_head", {}, { "j_lc" });
    dfg.addNode("Aik_addr", "Add", { "i_base_k", "k" });
    dfg.addNode("Ajk_addr", "Add", { "j_base_shadow", "k" });
    dfg.addNode("Aik", "Load", { "Aik_addr" }, &A, baseAddr_A);
    dfg.addNode("Ajk", "Load", { "Ajk_addr" }, &A, baseAddr_A);
    dfg.addNode("Aik_Ajk", "Mul", { "Aik", "Ajk" });
    dfg.addNode("Aij_update", "Sub", { "Aij_shadow", "Aik_Ajk" });

    // loop_k1
    dfg.addNode("k1", "Loop_head", {}, { "i_lc2" });
    dfg.addNode("Aik1_addr", "Add", { "i_base_k1", "k1" });
    dfg.addNode("Aik1", "Load", { "Aik1_addr" }, &A, baseAddr_A);
    dfg.addNode("Aii", "Load", { "Aii_addr" }, &A, baseAddr_A);
    dfg.addNode("Aik1_Aik1", "Mul", { "Aik1"});
    dfg.addNode("Aii_update", "Sub", { "Aii", "Aik1_Aik1" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc1", "i_lc2", "i_base", "Aii_addr", "i_base_k", "i_base_k1", "Aii_sqrt" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "j_lc", "j_base", "j_base_shadow", "Aij_addr", "Aij", "Aij_shadow", "Ajj_addr", "Ajj", "Aij_update_shadow", "Aij_div" });
    dfg.addNodes2CtrlTree("loop_k", { "k", "Aik_addr", "Ajk_addr", "Aik", "Ajk", "Aik_Ajk", "Aij_update" });
    dfg.addNodes2CtrlTree("loop_k1", { "k1", "Aik1_addr", "Aik1", "Aii", "Aik1_Aik1","Aii_update" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_i", /*"Aii_sqrt"*/ "j");
    dfg.setTheTailNode("loop_j", "Aij_div");
    dfg.setTheTailNode("loop_k", "Aij_update");
    dfg.setTheTailNode("loop_k1", "Aii_update");

    dfg.plotDot();
}

// Graph partition
void Cholesky_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 0;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 0;
        chanGraph.getNode("Lse_Aij")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Lse_Aik")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_Aii")->subgraphId = 0;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 0;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 0;
        chanGraph.getNode("Lse_Aij")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Lse_Aik")->subgraphId = 0;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 0;

        chanGraph.getNode("Lc_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_Aii")->subgraphId = 1;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 1;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 1;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 1;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 1;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 1;
        chanGraph.getNode("Lse_Aij")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Lse_Aik")->subgraphId = 1;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 1;

        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_Aii")->subgraphId = 2;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 2;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 2;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 1;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 1;
        chanGraph.getNode("Lse_Aij")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 2;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Lse_Aik")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 2;

        chanGraph.getNode("Lc_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_Aii")->subgraphId = 3;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 3;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 3;

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 1;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 1;
        chanGraph.getNode("Lse_Aij")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 2;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Lse_Aik")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 2;

        chanGraph.getNode("Lc_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 3;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_Aii")->subgraphId = 4;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 4;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 4;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 4;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 4;

        break;
    }
    case 6:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 1;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 1;
        chanGraph.getNode("Lse_Aij")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 2;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 3;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Lse_Aik")->subgraphId = 3;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 3;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 3;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 3;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 3;

        chanGraph.getNode("Lc_k1")->subgraphId = 4;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 4;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 4;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_Aii")->subgraphId = 5;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 5;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 5;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 5;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 5;

        break;
    }
    case 7:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_base_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_Aii_addr")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base")->subgraphId = 1;
        chanGraph.getNode("Chan_i_base_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_base_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_Ajj_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_Aij_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j")->subgraphId = 2;
        chanGraph.getNode("Lse_Ajj")->subgraphId = 2;
        chanGraph.getNode("Lse_Aij")->subgraphId = 2;
        chanGraph.getNode("Chan_Aij_shadow")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_j_base_shadow_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_base_k_relay_loop_j_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_Ajk_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_Aik_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_Ajk")->subgraphId = 4;
        chanGraph.getNode("Chan_Aij_shadow_scatter_loop_k")->subgraphId = 4;
        chanGraph.getNode("Lse_Aik")->subgraphId = 4;
        chanGraph.getNode("Chan_Aik_Ajk")->subgraphId = 4;
        chanGraph.getNode("Chan_Aij_update")->subgraphId = 4;
        chanGraph.getNode("Chan_Aij_update_shadow")->subgraphId = 4;
        chanGraph.getNode("Chan_Aij_div")->subgraphId = 4;

        chanGraph.getNode("Lc_k1")->subgraphId = 5;
        chanGraph.getNode("Chan_i_base_k1_scatter_loop_k1")->subgraphId = 5;
        chanGraph.getNode("Chan_Aii_addr_scatter_loop_k1")->subgraphId = 5;
        chanGraph.getNode("Chan_Aik1_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_Aii")->subgraphId = 6;
        chanGraph.getNode("Lse_Aik1")->subgraphId = 6;
        chanGraph.getNode("Chan_Aik1_Aik1")->subgraphId = 6;
        chanGraph.getNode("Chan_Aii_update")->subgraphId = 6;
        chanGraph.getNode("Chan_Aii_sqrt")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}