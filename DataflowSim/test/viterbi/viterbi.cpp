#include "./viterbi.h"

using namespace DFSimTest;

Dfg Viterbi_Test::dfg;

const uint Viterbi_Test::N_OBS = 50;
const uint Viterbi_Test::N_STATES = 50;
const uint Viterbi_Test::N_TOKENS = 50;

const uint Viterbi_Test::llike_BaseAddr = 0;
const uint Viterbi_Test::transition_BaseAddr = 21648;
const uint Viterbi_Test::emission_BaseAddr = 497516;

vector<int> Viterbi_Test::obs;
vector<int> Viterbi_Test::llike;
vector<int> Viterbi_Test::transition;
vector<int> Viterbi_Test::emission;

void Viterbi_Test::generateData()
{
    obs.resize(100000);
    llike.resize(100000);
    transition.resize(100000);
    emission.resize(100000);

    //for (size_t i = 0; i < fft_size; ++i)
    //{
    //    real[i] = i;
    //    img[i] = i;
    //    real_twid[i] = i;
    //    img_twid[i] = i;
    //}
}

void Viterbi_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_t", "Loop", "Null"),
         make_tuple<string, string, string>("loop_curr", "Loop", "Null"),
         make_tuple<string, string, string>("loop_prev_", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_t", { "loop_curr" });
    dfg.controlTree.addLowerControlRegion("loop_curr", { "loop_prev_" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("N_OBS", "Const", Viterbi_Test::N_OBS);
    dfg.addNode("N_TOKENS", "Const", Viterbi_Test::N_STATES);
    dfg.addNode("N_STATES", "Const", Viterbi_Test::N_TOKENS);
    // loop_t
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "t" });
    dfg.addNode("t", "Loop_head", {}, { "begin" });
    dfg.addNode("t_lc", "Nop", { "t" });

    // loop_curr
    dfg.addNode("curr", "Loop_head", {}, { "t_lc" });
    dfg.addNode("curr_lc", "Nop", { "curr" });
    dfg.addNode("prev", "Nop", {}, { "curr" });
    dfg.addNode("llike", "Load", { "t_lc"/*, "prev"*/ }, &llike, llike_BaseAddr);
    dfg.addNode("prev_N", "Mul", { "prev", "N_STATES" });
    dfg.addNode("prev_N_curr", "Add", { "prev_N", "curr" });
    dfg.addNode("prev_N_curr_shadow", "Nop", { "prev_N_curr" });
    dfg.addNode("curr_N", "Mul", { "curr", "N_TOKENS" });
    dfg.addNode("curr_N_obs", "Add", { "curr_N" });
    //dfg.addNode("curr_N_obs_shadow", "Nop", { "curr_N_obs" });
    dfg.addNode("transition", "Load", { "prev_N_curr" }, &transition, transition_BaseAddr);
    dfg.addNode("emission", "Load", { "curr_N_obs" }, &emission, emission_BaseAddr);
    dfg.addNode("llike_transition", "Add", { "llike", "transition" });
    dfg.addNode("min_p", "Add", { "llike_transition", "emission" });
    dfg.addNode("llike_update", "Store", { "curr", "min_p" }, &emission, emission_BaseAddr);

    // loop_prev_
    dfg.addNode("prev_", "Loop_head", {}, { "curr_lc" });
    dfg.addNode("llike_", "Load", { "t_lc"/*, "prev_"*/ }, &llike, llike_BaseAddr);
    dfg.addNode("prev_N_", "Mul", { "prev_", "N_STATES" });
    dfg.addNode("prev_N_curr_", "Add", { "prev_N_", "curr_lc" });
    dfg.addNode("curr_N_", "Mul", { "curr_lc", "N_TOKENS" });
    dfg.addNode("curr_N_obs_", "Add", { "curr_N_" });
    dfg.addNode("transition_", "Load", { "prev_N_curr_shadow" }, &transition, transition_BaseAddr);
    dfg.addNode("emission_", "Load", { "curr_N_obs_" }, &emission, emission_BaseAddr);
    dfg.addNode("llike_transition_", "Add", { "llike_", "transition_" });
    dfg.addNode("p", "Add", { "llike_transition_", "emission_" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_t", { "begin", "end", "t", "t_lc" });
    dfg.addNodes2CtrlTree("loop_curr", { "curr", "curr_lc", "prev", "llike", "prev_N", "prev_N_curr", "prev_N_curr_shadow",
        "curr_N", "curr_N_obs", /*"curr_N_obs_shadow",*/ "transition", "emission", "llike_transition", "min_p", "llike_update" });
    dfg.addNodes2CtrlTree("loop_prev_", { "prev_", "llike_", "prev_N_", "prev_N_curr_", "curr_N_", "curr_N_obs_", "transition_",
        "emission_", "llike_transition_", "p" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_t", "curr");
    dfg.setTheTailNode("loop_curr", "llike_update");
    dfg.setTheTailNode("loop_prev_", "p");

    dfg.plotDot();
}


// Graph partition
void Viterbi_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 0;
        chanGraph.getNode("Lse_llike")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 0;
        chanGraph.getNode("Lse_emission")->subgraphId = 0;
        chanGraph.getNode("Lse_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_min_p")->subgraphId = 0;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 0;
      
        chanGraph.getNode("Lc_prev_")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 0;
        chanGraph.getNode("Lse_llike_")->subgraphId = 0;
        chanGraph.getNode("Lse_transition_")->subgraphId = 0;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 0;
        chanGraph.getNode("Lse_emission_")->subgraphId = 0;
        chanGraph.getNode("Chan_p")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 0;
        chanGraph.getNode("Lse_llike")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 0;
        chanGraph.getNode("Lse_emission")->subgraphId = 0;
        chanGraph.getNode("Lse_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_min_p")->subgraphId = 0;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 1;
        chanGraph.getNode("Lse_llike_")->subgraphId = 1;
        chanGraph.getNode("Lse_transition_")->subgraphId = 1;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 1;
        chanGraph.getNode("Lse_emission_")->subgraphId = 1;
        chanGraph.getNode("Chan_p")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 0;
        chanGraph.getNode("Lse_llike")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 0;
        chanGraph.getNode("Lse_emission")->subgraphId = 0;
        chanGraph.getNode("Lse_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 0;
        chanGraph.getNode("Chan_min_p")->subgraphId = 0;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 0;

        chanGraph.getNode("Lc_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 1;
        chanGraph.getNode("Lse_llike_")->subgraphId = 2;
        chanGraph.getNode("Lse_transition_")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 2;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 2;
        chanGraph.getNode("Lse_emission_")->subgraphId = 2;
        chanGraph.getNode("Chan_p")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 0;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 0;
        chanGraph.getNode("Lse_llike")->subgraphId = 1;
        chanGraph.getNode("Lse_emission")->subgraphId = 1;
        chanGraph.getNode("Lse_transition")->subgraphId = 1;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 1;
        chanGraph.getNode("Chan_min_p")->subgraphId = 1;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 1;

        chanGraph.getNode("Lc_prev_")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 2;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 2;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 2;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 2;
        chanGraph.getNode("Lse_llike_")->subgraphId = 3;
        chanGraph.getNode("Lse_transition_")->subgraphId = 3;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Lse_emission_")->subgraphId = 3;
        chanGraph.getNode("Chan_p")->subgraphId = 3;

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 1;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 2;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_min_p")->subgraphId = 2;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 2;

        chanGraph.getNode("Lc_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_")->subgraphId = 4;
        chanGraph.getNode("Lse_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 4;
        chanGraph.getNode("Lse_emission_")->subgraphId = 4;
        chanGraph.getNode("Chan_p")->subgraphId = 4;

        break;
    }
    case 6:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 1;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 2;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_min_p")->subgraphId = 2;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 2;

        chanGraph.getNode("Lc_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_")->subgraphId = 4;
        chanGraph.getNode("Lse_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 5;
        chanGraph.getNode("Lse_emission_")->subgraphId = 5;
        chanGraph.getNode("Chan_p")->subgraphId = 5;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 1;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 2;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_min_p")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 3;

        chanGraph.getNode("Lc_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 4;
        chanGraph.getNode("Lse_llike_")->subgraphId = 5;
        chanGraph.getNode("Lse_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 5;
        chanGraph.getNode("Lse_emission_")->subgraphId = 5;
        chanGraph.getNode("Chan_p")->subgraphId = 5;

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 1;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 2;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_min_p")->subgraphId = 2;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 2;

        chanGraph.getNode("Lc_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 3;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 3;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_")->subgraphId = 4;
        chanGraph.getNode("Lse_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 4;
        chanGraph.getNode("Lse_emission_")->subgraphId = 5;
        chanGraph.getNode("Chan_p")->subgraphId = 5;*/

        break;
    }
    case 7:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 2;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 3;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 3;
        chanGraph.getNode("Chan_min_p")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 3;

        chanGraph.getNode("Lc_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 4;
        chanGraph.getNode("Lse_llike_")->subgraphId = 5;
        chanGraph.getNode("Lse_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 6;
        chanGraph.getNode("Lse_emission_")->subgraphId = 6;
        chanGraph.getNode("Chan_p")->subgraphId = 6;*/

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 2;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 3;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 3;
        chanGraph.getNode("Chan_min_p")->subgraphId = 4;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 4;

        chanGraph.getNode("Lc_prev_")->subgraphId = 5;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 5;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 5;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 5;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 5;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 5;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 5;
        chanGraph.getNode("Lse_llike_")->subgraphId = 6;
        chanGraph.getNode("Lse_transition_")->subgraphId = 6;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 6;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 6;
        chanGraph.getNode("Lse_emission_")->subgraphId = 6;
        chanGraph.getNode("Chan_p")->subgraphId = 6;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_t")->subgraphId = 0;
        chanGraph.getNode("Chan_t_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_prev")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_scatter_loop_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_N_obs")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N")->subgraphId = 1;
        chanGraph.getNode("Chan_prev_N_curr")->subgraphId = 1;
        chanGraph.getNode("Chan_curr_lc")->subgraphId = 1;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr")->subgraphId = 2;
        chanGraph.getNode("Chan_prev_N_curr_shadow")->subgraphId = 2;
        chanGraph.getNode("Lse_llike")->subgraphId = 2;
        chanGraph.getNode("Lse_emission")->subgraphId = 3;
        chanGraph.getNode("Lse_transition")->subgraphId = 2;
        chanGraph.getNode("Chan_llike_transition")->subgraphId = 3;
        chanGraph.getNode("Chan_min_p")->subgraphId = 3;
        chanGraph.getNode("Lse_llike_update")->subgraphId = 3;

        chanGraph.getNode("Lc_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_shadow_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_lc_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_t_lc_relay_loop_curr_scatter_loop_prev_")->subgraphId = 4;
        chanGraph.getNode("Chan_prev_N_curr_")->subgraphId = 4;
        chanGraph.getNode("Chan_curr_N_")->subgraphId = 4;
        chanGraph.getNode("Lse_llike_")->subgraphId = 5;
        chanGraph.getNode("Lse_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_llike_transition_")->subgraphId = 5;
        chanGraph.getNode("Chan_curr_N_obs_")->subgraphId = 5;
        chanGraph.getNode("Lse_emission_")->subgraphId = 6;
        chanGraph.getNode("Chan_p")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}