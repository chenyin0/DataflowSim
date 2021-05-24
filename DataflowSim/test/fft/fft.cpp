#include "./fft.h"

using namespace DFSimTest;

Dfg FFT_Test::dfg;

const uint FFT_Test::fft_size = 1024;
vector<int> FFT_Test::real;
vector<int> FFT_Test::img;
vector<int> FFT_Test::real_twid;
vector<int> FFT_Test::img_twid;

const uint FFT_Test::real_BaseAddr = 0;
const uint FFT_Test::img_BaseAddr = 5280;
const uint FFT_Test::real_twid_BaseAddr = 26890;
const uint FFT_Test::img_twid_BaseAddr = 58947;

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
        { make_tuple<string, string, string>("loop_span", "Loop", "Null"),
         make_tuple<string, string, string>("loop_odd", "Loop", "Null"),
         make_tuple<string, string, string>("branch_rootindex", "Branch", "truePath")
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
    dfg.addNode("even", "Xor", {"odd", "span_lc"});

    dfg.addNode("real_even_val", "Load", { "even" }, &real, real_BaseAddr);
    dfg.addNode("real_odd_val", "Load", { "odd"}, &real, real_BaseAddr);
    dfg.addNode("img_even_val", "Load", {"even"}, &img, img_BaseAddr);
    dfg.addNode("img_odd_val", "Load", { "odd" }, &img, img_BaseAddr);

    dfg.addNode("real_even_val_update", "Add", { "real_even_val", "real_odd_val" });
    dfg.addNode("real_odd_val_update", "Sub", { "real_even_val", "real_odd_val" });

    dfg.addNode("img_even_val_update", "Add", { "img_even_val", "img_odd_val" });
    dfg.addNode("img_odd_val_update", "Sub", { "img_even_val", "img_odd_val" });

    dfg.addNode("log", "Add", {}, { "odd_lp" });  // Self-add
    dfg.addNode("tt", "Shl", {"even", "log"});
    dfg.addNode("rootindex", "And", { "tt", "FFT_SIZE" });

    dfg.addNode("rootindex_cmp", "Bne", { "rootindex" });  // rootindex != 0

    dfg.addNode("rootindex_active", "Nop", { "rootindex_cmp", "rootindex" });  // rootindex != 0
    dfg.addNode("real_twid_val", "Load", { "rootindex_active", "rootindex_active" }, &real_twid, real_twid_BaseAddr);
    dfg.addNode("img_twid_val", "Load", { "rootindex_active", "rootindex_active" }, &img_twid, img_twid_BaseAddr);

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
    dfg.setTheTailNode("loop_odd", "branch_merge");

    dfg.plotDot();
}