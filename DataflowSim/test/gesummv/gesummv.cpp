#include "./gesummv.h"

using namespace DFSimTest;

Dfg Gesummv_Test::dfg;

vector<int> Gesummv_Test::A;
vector<int> Gesummv_Test::B;
vector<int> Gesummv_Test::x;
vector<int> Gesummv_Test::y;

const uint Gesummv_Test::A_BaseAddr = 0;
const uint Gesummv_Test::B_BaseAddr = 16055;
const uint Gesummv_Test::x_BaseAddr = 325589;
const uint Gesummv_Test::y_BaseAddr = 4800962;

const uint Gesummv_Test::_PB_N = 200;
const uint Gesummv_Test::alpha = 2;
const uint Gesummv_Test::beta = 4;

void Gesummv_Test::generateData()
{
    uint size = _PB_N * _PB_N;
    A.resize(size);
    B.resize(size);
    x.resize(size);
    y.resize(size);

    for (size_t i = 0; i < size; ++i)
    {
        A[i] = i;
        B[i] = i;
        x[i] = i;
        y[i] = i;
    }
}

void Gesummv_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("loop_j", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_j" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("alpha", "Const", Gesummv_Test::alpha);
    dfg.addNode("beta", "Const", Gesummv_Test::beta);
    // loop_i
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "i" });
    dfg.addNode("i", "Loop_head", {}, { "begin" });
    dfg.addNode("i_lc", "Nop", { "i" });
    dfg.addNode("i_PB_N", "Mul", { "i" });
    dfg.addNode("alpha_tmp", "Mul", { "tmp", "alpha" });
    dfg.addNode("beta_y", "Mul", { "y_update", "beta" });
    dfg.addNode("y_update_i", "Add", { "alpha_tmp", "beta_y" });

    // loop_j
    dfg.addNode("j", "Loop_head", {}, { "i_lc" });
    dfg.addNode("addr", "ADD", {"i_PB_N", "j"});
    dfg.addNode("A", "Load", { "addr" }, &A, A_BaseAddr);
    dfg.addNode("x", "Load", { "j" }, &x, x_BaseAddr);
    dfg.addNode("B", "Load", { "addr" }, &B, B_BaseAddr);
    //dfg.addNode("y", "Load", { "i_lc" }, &y, y_BaseAddr);
    dfg.addNode("A_x", "Mul", { "A", "x" });
    dfg.addNode("tmp", "Add", { "A_x"});
    dfg.addNode("B_x", "Mul", { "B", "x" });
    dfg.addNode("y_update", "Add", { "B_x"});

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_i", { "begin", "end", "i", "i_lc", "i_PB_N", "alpha_tmp", "beta_y", "y_update_i" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "addr", "A", "x", "B", /*"y",*/ "A_x", "tmp", "B_x", "y_update" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_i", "y_update_i");
    dfg.setTheTailNode("loop_j", "y_update");

    dfg.plotDot();
}


// Graph partition
void Gesummv_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 0;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 0;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Lse_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 0;
        //chanGraph.getNode("Lse_y")->subgraphId = 0;
        chanGraph.getNode("Chan_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_A")->subgraphId = 0;
        chanGraph.getNode("Lse_B")->subgraphId = 0;
        chanGraph.getNode("Chan_A_x")->subgraphId = 0;
        chanGraph.getNode("Chan_B_x")->subgraphId = 0;
        chanGraph.getNode("Chan_tmp")->subgraphId = 0;
        chanGraph.getNode("Chan_y_update")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_A")->subgraphId = 1;
        chanGraph.getNode("Lse_B")->subgraphId = 1;
        chanGraph.getNode("Chan_A_x")->subgraphId = 1;
        chanGraph.getNode("Chan_B_x")->subgraphId = 1;
        chanGraph.getNode("Chan_tmp")->subgraphId = 1;
        chanGraph.getNode("Chan_y_update")->subgraphId = 1;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 1;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 1;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 1;

        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 0;
        //chanGraph.getNode("Lse_x")->subgraphId = 0;
        ////chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 0;
        ////chanGraph.getNode("Lse_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_addr")->subgraphId = 0;
        //chanGraph.getNode("Lse_A")->subgraphId = 0;
        //chanGraph.getNode("Lse_B")->subgraphId = 0;
        //chanGraph.getNode("Chan_A_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_B_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_tmp")->subgraphId = 0;
        //chanGraph.getNode("Chan_y_update")->subgraphId = 0;

        //chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 1;
        //chanGraph.getNode("Chan_beta_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_y_update_i")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_A")->subgraphId = 1;
        chanGraph.getNode("Lse_B")->subgraphId = 1;
        chanGraph.getNode("Chan_A_x")->subgraphId = 1;
        chanGraph.getNode("Chan_B_x")->subgraphId = 1;
        chanGraph.getNode("Chan_tmp")->subgraphId = 1;
        chanGraph.getNode("Chan_y_update")->subgraphId = 1;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 2;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 2;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 1;
        chanGraph.getNode("Lse_A")->subgraphId = 1;
        chanGraph.getNode("Lse_B")->subgraphId = 1;
        chanGraph.getNode("Chan_A_x")->subgraphId = 2;
        chanGraph.getNode("Chan_B_x")->subgraphId = 2;
        chanGraph.getNode("Chan_tmp")->subgraphId = 2;
        chanGraph.getNode("Chan_y_update")->subgraphId = 2;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 3;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 3;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 3;

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 2;
        chanGraph.getNode("Lse_A")->subgraphId = 2;
        chanGraph.getNode("Lse_B")->subgraphId = 2;
        chanGraph.getNode("Chan_A_x")->subgraphId = 3;
        chanGraph.getNode("Chan_B_x")->subgraphId = 3;
        chanGraph.getNode("Chan_tmp")->subgraphId = 3;
        chanGraph.getNode("Chan_y_update")->subgraphId = 3;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 4;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 4;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 4;

        break;
    }
    case 6:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 1;
        ////chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        ////chanGraph.getNode("Lse_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_addr")->subgraphId = 1;
        //chanGraph.getNode("Lse_x")->subgraphId = 2;
        //chanGraph.getNode("Lse_A")->subgraphId = 3;
        //chanGraph.getNode("Lse_B")->subgraphId = 3;
        //chanGraph.getNode("Chan_A_x")->subgraphId = 4;
        //chanGraph.getNode("Chan_B_x")->subgraphId = 4;
        //chanGraph.getNode("Chan_tmp")->subgraphId = 4;
        //chanGraph.getNode("Chan_y_update")->subgraphId = 4;

        //chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 5;
        //chanGraph.getNode("Chan_beta_y")->subgraphId = 5;
        //chanGraph.getNode("Chan_y_update_i")->subgraphId = 5;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 2;
        chanGraph.getNode("Lse_A")->subgraphId = 2;
        chanGraph.getNode("Lse_B")->subgraphId = 3;
        chanGraph.getNode("Chan_A_x")->subgraphId = 4;
        chanGraph.getNode("Chan_B_x")->subgraphId = 4;
        chanGraph.getNode("Chan_tmp")->subgraphId = 4;
        chanGraph.getNode("Chan_y_update")->subgraphId = 4;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 5;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 5;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 5;

        break;
    }
    case 7:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_i_PB_N")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_PB_N_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Lse_y")->subgraphId = 1;
        chanGraph.getNode("Chan_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_x")->subgraphId = 2;
        chanGraph.getNode("Lse_A")->subgraphId = 2;
        chanGraph.getNode("Lse_B")->subgraphId = 3;
        chanGraph.getNode("Chan_A_x")->subgraphId = 4;
        chanGraph.getNode("Chan_B_x")->subgraphId = 4;
        chanGraph.getNode("Chan_tmp")->subgraphId = 4;
        chanGraph.getNode("Chan_y_update")->subgraphId = 4;

        chanGraph.getNode("Chan_alpha_tmp")->subgraphId = 5;
        chanGraph.getNode("Chan_beta_y")->subgraphId = 6;
        chanGraph.getNode("Chan_y_update_i")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}