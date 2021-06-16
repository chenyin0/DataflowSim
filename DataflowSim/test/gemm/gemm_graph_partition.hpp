#include "../../src/sim/graph.h"

using namespace DFSim;

class GraphPartition
{
public:
    static void graphPartition_1(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_k_row")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 0;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 0;
        chanGraph.getNode("Chan_mul")->subgraphId = 0;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 0;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 0;
    }

    static void graphPartition_2(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
    
        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;
    
        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 0;
    
        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_k_row")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 0;
    
        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 1;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 1;
        chanGraph.getNode("Chan_mul")->subgraphId = 1;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 1;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 1;
    }

    static void graphPartition_3(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_k_row")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 0;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 1;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 2;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 2;
        chanGraph.getNode("Chan_mul")->subgraphId = 2;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 2;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 2;
    }

    static void graphPartition_4(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_k_row")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 1;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 1;

        chanGraph.getNode("Lc_j")->subgraphId = 2;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 2;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 3;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 3;
        chanGraph.getNode("Chan_mul")->subgraphId = 3;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 3;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 3;
    }

    static void graphPartition_5(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_i_row")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 0;

        chanGraph.getNode("Lc_k")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 1;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_k_row")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 2;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 2;

        chanGraph.getNode("Lc_j")->subgraphId = 3;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 3;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 4;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 4;
        chanGraph.getNode("Chan_mul")->subgraphId = 4;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 4;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 4;
    }

    static void graphPartition_6(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_i_row")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 1;

        chanGraph.getNode("Lc_k")->subgraphId = 2;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 2;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_k_row")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 3;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 3;

        chanGraph.getNode("Lc_j")->subgraphId = 4;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 4;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 5;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 5;
        chanGraph.getNode("Chan_mul")->subgraphId = 5;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 5;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 5;
    }

    static void graphPartition_7(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 2;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_row")->subgraphId = 2;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 3;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 3;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_m1_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_k_row")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 4;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 4;

        chanGraph.getNode("Lc_j")->subgraphId = 5;
        chanGraph.getNode("Chan_m1_data_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 5;
        chanGraph.getNode("Chan_m2_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_m2_data")->subgraphId = 6;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 6;
        chanGraph.getNode("Chan_mul")->subgraphId = 6;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 6;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 6;
    }

    /*static void graphPartition_8(ChanGraph& chanGraph)
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_jj")->subgraphId = 0;
        chanGraph.getNode("Chan_jj_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_kk_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_scatter_loop_kk")->subgraphId = 1;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk")->subgraphId = 1;

        chanGraph.getNode("Lc_i")->subgraphId = 2;
        chanGraph.getNode("Chan_kk_lc_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_kk_scatter_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_i_row")->subgraphId = 2;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i")->subgraphId = 2;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i")->subgraphId = 2;

        chanGraph.getNode("Lc_k")->subgraphId = 3;
        chanGraph.getNode("Chan_kk_lc_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_row_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_jj_lc_relay_loop_i_scatter_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_k_lc")->subgraphId = 3;
        chanGraph.getNode("Chan_k_kk")->subgraphId = 3;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_i_row_relay_loop_k")->subgraphId = 3;
        chanGraph.getNode("Chan_m3_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_k_row")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_x")->subgraphId = 4;
        chanGraph.getNode("Chan_m1_data")->subgraphId = 4;

        chanGraph.getNode("Lc_j")->subgraphId = 5;
        chanGraph.getNode("Chan_jj_lc_relay_loop_k_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_i_row_relay_loop_k_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_k_row_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_j_jj")->subgraphId = 5;
        chanGraph.getNode("Chan_m6_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_prod_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_m6_data")->subgraphId = 6;
        chanGraph.getNode("Lse_prod_data")->subgraphId = 6;
        chanGraph.getNode("Chan_mul")->subgraphId = 6;
        chanGraph.getNode("Chan_prod_data_update")->subgraphId = 6;
        chanGraph.getNode("Lse_prod_data_update_st")->subgraphId = 6;
    }*/
};
