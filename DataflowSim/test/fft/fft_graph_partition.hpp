#include "../../src/sim/graph.h"

using namespace DFSim;

class GraphPartition
{
public:
    static void graphPartition_1(ChanGraph& chanGraph)
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
    }

    static void graphPartition_2(ChanGraph& chanGraph)
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
    }

    static void graphPartition_3(ChanGraph& chanGraph)
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
    }

    static void graphPartition_4(ChanGraph& chanGraph)
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
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 1;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 1;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 1;
        chanGraph.getNode("Chan_tt")->subgraphId = 1;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 1;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 1;
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
        chanGraph.getNode("Chan_branch_merge")->subgraphId = 3;
    }

    static void graphPartition_5(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
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
        chanGraph.getNode("Chan_tt")->subgraphId = 2;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 2;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 3;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 2;
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
    }

    static void graphPartition_6(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
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
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 4;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 4;
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
    }

    static void graphPartition_7(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_span")->subgraphId = 0;
        chanGraph.getNode("Chan_span_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_odd_lp")->subgraphId = 1;
        chanGraph.getNode("Chan_log")->subgraphId = 2;
        chanGraph.getNode("Chan_span_lc_scatter_loop_odd")->subgraphId = 1;
        chanGraph.getNode("Chan_odd")->subgraphId = 2;
        chanGraph.getNode("Chan_even")->subgraphId = 2;
        chanGraph.getNode("Lse_real_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_img_odd_val")->subgraphId = 3;
        chanGraph.getNode("Lse_real_even_val")->subgraphId = 4;
        chanGraph.getNode("Lse_img_even_val")->subgraphId = 4;
        chanGraph.getNode("Chan_tt")->subgraphId = 4;
        chanGraph.getNode("Chan_real_even_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_real_odd_val_update")->subgraphId = 5;
        chanGraph.getNode("Chan_img_even_val_update")->subgraphId = 5;
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
    }

};
