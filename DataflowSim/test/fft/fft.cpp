#include "./fft.h"

using namespace DFSimTest;

Dfg FFT_Test::dfg;

const uint64_t FFT_Test::fft_size = 4096;
vector<int> FFT_Test::real;
vector<int> FFT_Test::img;
vector<int> FFT_Test::real_twid;
vector<int> FFT_Test::img_twid;

const uint64_t FFT_Test::real_BaseAddr = 0;
const uint64_t FFT_Test::img_BaseAddr = 5280;
const uint64_t FFT_Test::real_twid_BaseAddr = 26890;
const uint64_t FFT_Test::img_twid_BaseAddr = 58947;

void FFT_Test::generateData()
{
    real.resize(fft_size);
    img.resize(fft_size);
    real_twid.resize(fft_size);
    img_twid.resize(fft_size);

    for (size_t i = 0; i < fft_size; ++i)
    {
        real[i] = i;
        img[i] = i;
        real_twid[i] = i;
        img_twid[i] = i;
    }
}

void FFT_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { std::make_tuple<string, string, string>("loop_span", "Loop", "Null"),
         std::make_tuple<string, string, string>("loop_odd", "Loop", "Null"),
         std::make_tuple<string, string, string>("branch_rootindex", "Branch", "truePath")
        });

    dfg.controlTree.addLowerControlRegion("loop_span", { "loop_odd" });
    dfg.controlTree.addLowerControlRegion("loop_odd", { "branch_rootindex" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("FFT_SIZE", "Const", FFT_Test::fft_size);
    // loop_span
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "span" });
    dfg.addNode("span", "Loop_head", {}, { "begin" });
    dfg.addNode("span_lc", "Nop", { "span" });

    // loop_odd
    dfg.addNode("odd_lp", "Loop_head", {}, { "span_lc" });
    dfg.addNode("odd", "Or", { "odd_lp", "span_lc" });
    dfg.addNode("even", "Xor", { "odd", "span_lc" });

    dfg.addNode("real_even_val", "Load", { "even" }, &real, real_BaseAddr);
    dfg.addNode("real_odd_val", "Load", { "odd" }, &real, real_BaseAddr);
    dfg.addNode("img_even_val", "Load", { "even" }, &img, img_BaseAddr);
    dfg.addNode("img_odd_val", "Load", { "odd" }, &img, img_BaseAddr);

    dfg.addNode("real_even_val_update", "Add", { "real_even_val", "real_odd_val" });
    dfg.addNode("real_odd_val_update", "Sub", { "real_even_val", "real_odd_val" });

    dfg.addNode("img_even_val_update", "Add", { "img_even_val", "img_odd_val" });
    dfg.addNode("img_odd_val_update", "Sub", { "img_even_val", "img_odd_val" });

    dfg.addNode("log", "Add", {}, { "odd_lp" });  // Self-add
    dfg.addNode("tt", "Shl", { "even", "log" });
    dfg.addNode("rootindex", "And", { "tt", "FFT_SIZE" });

    dfg.addNode("rootindex_cmp", "Cmp", { "rootindex" });  // rootindex != 0

    dfg.addNode("rootindex_active", "Nop", { "rootindex_cmp", "rootindex" });  // rootindex != 0
    dfg.addNode("real_twid_val", "Load", { "rootindex_active"/*, "rootindex_active" */ }, &real_twid, real_twid_BaseAddr);
    dfg.addNode("img_twid_val", "Load", { "rootindex_active"/*, "rootindex_active" */ }, &img_twid, img_twid_BaseAddr);

    dfg.addNode("real_twid_real", "Mul", { "real_twid_val", "real_odd_val_update" });
    dfg.addNode("img_twid_img", "Mul", { "img_twid_val", "img_odd_val_update" });
    dfg.addNode("real_twid_img", "Mul", { "real_twid_val", "img_odd_val_update" });
    dfg.addNode("img_twid_real", "Mul", { "img_twid_val", "real_odd_val_update" });

    dfg.addNode("img_odd_update", "Add", { "real_twid_img", "img_twid_real" });
    dfg.addNode("real_odd_update", "Sub", { "real_twid_real", "img_twid_img" });

    dfg.addNode("branch_merge", "selPartial", { "rootindex_cmp", "real_odd_update", "rootindex" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_span", { "begin", "end", "span", "span_lc" });
    dfg.addNodes2CtrlTree("loop_odd", { "odd_lp", "odd", "even", "real_even_val", "real_odd_val",
        "img_even_val", "img_odd_val", "real_even_val_update", "real_odd_val_update", "img_even_val_update", "img_odd_val_update", "log", "tt", "rootindex", "rootindex_cmp", "branch_merge" });
    dfg.addNodes2CtrlTree("branch_rootindex", { "rootindex_active", "real_twid_val", "img_twid_val", "real_twid_real", "img_twid_img", "real_twid_img", "img_twid_real", "img_odd_update", "real_odd_update" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_span", "odd_lp");
    dfg.setTheTailNode("loop_odd", "branch_merge" /*"real_odd_update"*/);

    dfg.plotDot();
}


// Graph partition
void FFT_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 0;
        chanGraph.getNode("Chan_log")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_even")->subgraphId = 0;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 0;
        chanGraph.getNode("Chan_tt")->subgraphId = 0;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 0;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 0;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 0;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 0;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 0;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 0;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 0;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 0;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 0;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 0;
        chanGraph.getNode("Chan_log")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_even")->subgraphId = 0;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 0;
        chanGraph.getNode("Chan_tt")->subgraphId = 0;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 0;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 0;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 1;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 1;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 1;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 1;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 1;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 1;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 1;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 1;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 1;

        break;
    }
    case 3:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 0;
        chanGraph.getNode("Chan_log")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_even")->subgraphId = 0;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 0;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 1;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 1;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 1;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 2;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 2;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 2;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 2;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 2;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 2;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 2;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 1;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 1;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 1;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 2;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 2;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 2;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 2;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 2;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 2;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 2;

        break;
    }
    case 4:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 0;
        chanGraph.getNode("Chan_log")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 0;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 1;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 2;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 3;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 3;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 3;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 3;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 3;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 3;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 3;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 1;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 2;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 3;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 3;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 4;

        break;
    }
    case 5:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 3;
        chanGraph.getNode("Chan_tt")->subgraphId = 3;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 3;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 4;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 4;

        break;*/

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 2;
        chanGraph.getNode("Chan_tt")->subgraphId = 3;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 3;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 4;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 4;

        break;*/

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 2;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 2;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 2;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 2;
        chanGraph.getNode("Chan_tt")->subgraphId = 2;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 3;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 3;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 4;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 4;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 2;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 2;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 2;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 3;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 3;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 4;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 4;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 4;

        break;
    }
    case 6:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_span")->subgraphId = 0;
        //chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        //chanGraph.getNode("Chan_log")->subgraphId = 1;
        //chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        //chanGraph.getNode("Chan_odd")->subgraphId = 1;
        //chanGraph.getNode("Chan_even")->subgraphId = 1;
        //chanGraph.getNode("Lse_real_odd_val")->subgraphId = 2;
        //chanGraph.getNode("Lse_img_odd_val")->subgraphId = 2;
        //chanGraph.getNode("Lse_real_even_val")->subgraphId = 3;
        //chanGraph.getNode("Lse_img_even_val")->subgraphId = 3;
        //chanGraph.getNode("Chan_tt")->subgraphId = 3;
        //chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        //chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 4;
        //chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        //chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 4;
        //chanGraph.getNode("Chan_rootindex")->subgraphId = 4;
        //chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 4;

        //chanGraph.getNode("Chan_rootindex_active")->subgraphId = 5;
        //chanGraph.getNode("Lse_real_twid_val")->subgraphId = 5;
        //chanGraph.getNode("Lse_img_twid_val")->subgraphId = 5;
        //chanGraph.getNode("Chan_real_twid_real")->subgraphId = 5;
        //chanGraph.getNode("Chan_real_twid_img")->subgraphId = 5;
        //chanGraph.getNode("Chan_img_twid_img")->subgraphId = 5;
        //chanGraph.getNode("Chan_img_twid_real")->subgraphId = 5;
        //chanGraph.getNode("Chan_real_odd_update")->subgraphId = 5;
        //chanGraph.getNode("Chan_img_odd_update")->subgraphId = 5;
        //chanGraph.getNode("Chan_branch_merge")->subgraphId = 5;

        //break;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 2;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 2;
        chanGraph.getNode("Chan_odd")->subgraphId = 2;
        chanGraph.getNode("Chan_even")->subgraphId = 1;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 3;
        chanGraph.getNode("Chan_tt")->subgraphId = 3;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 4;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 4;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 5;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 5;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 5;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 5;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 5;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 5;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 5;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 5;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 5;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 5;

        break;
    }
    case 7:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 2;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 3;
        chanGraph.getNode("Chan_odd")->subgraphId = 3;
        chanGraph.getNode("Chan_even")->subgraphId = 3;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 4;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 4;
        chanGraph.getNode("Chan_tt")->subgraphId = 4;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 5;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 5;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 6;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 6;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 6;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 6;

        break;*/

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 2;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 2;
        chanGraph.getNode("Chan_odd")->subgraphId = 3;
        chanGraph.getNode("Chan_even")->subgraphId = 3;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 4;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 4;
        chanGraph.getNode("Chan_tt")->subgraphId = 4;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 5;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 5;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 6;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 6;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 6;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 6;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 1;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 2;
        chanGraph.getNode("Chan_even")->subgraphId = 2;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 3;
        chanGraph.getNode("Chan_tt")->subgraphId = 3;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_odd_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_rootindex")->subgraphId = 4;
        chanGraph.getNode("Chan_rootindex_cmp")->subgraphId = 4;

        chanGraph.getNode("Chan_rootindex_active")->subgraphId = 5;
        chanGraph.getNode("Lse_real_twid_val")->subgraphId = 5;
        chanGraph.getNode("Lse_img_twid_val")->subgraphId = 5;
        chanGraph.getNode("Chan_real_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_img")->subgraphId = 6;
        chanGraph.getNode("Chan_img_twid_real")->subgraphId = 6;
        chanGraph.getNode("Chan_real_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_img_odd_update")->subgraphId = 6;
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}