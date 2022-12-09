#include "./hotspot.h"

using namespace DFSimTest;

Dfg HotSpot_Test::dfg;

vector<int> HotSpot_Test::temp;
vector<int> HotSpot_Test::power;

uint64_t grid_width = 1024;
uint64_t grid_height = 1024;
float FACTOR_CHIP = 0.5;
uint64_t SPEC_HEAT_SI = 1.75 * pow(10, 6);
float t_chip = 0.0005;
uint64_t K_SI = 100;

const uint64_t HotSpot_Test::block_size = 256;
const uint64_t HotSpot_Test::CAP = FACTOR_CHIP * SPEC_HEAT_SI * t_chip * grid_width * grid_height;
const uint64_t HotSpot_Test::RX = grid_width / (2.0 * K_SI * t_chip * grid_height);
const uint64_t HotSpot_Test::RY = grid_height / (2.0 * K_SI * t_chip * grid_width);
const uint64_t HotSpot_Test::RZ = t_chip / (K_SI * grid_height * grid_width);

const uint64_t HotSpot_Test::temp_BaseAddr = 0;
const uint64_t HotSpot_Test::power_BaseAddr = 52865;

void HotSpot_Test::generateData()
{
    std::ifstream input_power;
    input_power.open("./test/hotspot/power_64.dat");
    std::ifstream input_temp;
    input_temp.open("./test/hotspot/temp_64.dat");

    string tmp_str;
    while (getline(input_power, tmp_str))
    {
        power.push_back(std::stof(tmp_str) * 100000.0);
    }

    while (getline(input_temp, tmp_str))
    {
        temp.push_back(std::stof(tmp_str) * 100.0);
    }
}

void HotSpot_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { std::make_tuple<string, string, string>("loop_r", "Loop", "Null"),
         std::make_tuple<string, string, string>("loop_c", "Loop", "Null"),
         std::make_tuple<string, string, string>("cond1_true", "Branch", "truePath"),
         std::make_tuple<string, string, string>("cond1_false", "Branch", "falsePath"),
         std::make_tuple<string, string, string>("cond2_true", "Branch", "truePath"),
         std::make_tuple<string, string, string>("cond2_false", "Branch", "falsePath"),
         std::make_tuple<string, string, string>("cond3_true", "Branch", "truePath"),
         std::make_tuple<string, string, string>("cond3_false", "Branch", "falsePath")
        });

    dfg.controlTree.addLowerControlRegion("loop_r", { "loop_c" });
    dfg.controlTree.addLowerControlRegion("loop_c", { "cond1_true", "cond1_false" });
    dfg.controlTree.addLowerControlRegion("cond1_false", { "cond2_true", "cond2_false" });
    dfg.controlTree.addLowerControlRegion("cond2_false", { "cond3_true", "cond3_false" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("CAP", "Const", HotSpot_Test::CAP);
    dfg.addNode("RX", "Const", HotSpot_Test::RX);
    dfg.addNode("RY", "Const", HotSpot_Test::RY);
    dfg.addNode("RZ", "Const", HotSpot_Test::RZ);
    // loop_r
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "r" });
    dfg.addNode("r", "Loop_head", {}, { "begin" });
    dfg.addNode("r_lc", "Nop", { "r" });

    // loop_c
    dfg.addNode("c", "Loop_head", {}, { "r_lc" });
    dfg.addNode("cond1", "Cmp", { "r_lc", "c" });
    dfg.addNode("delta_merge_cond1", "selPartial", { "cond1", "delta_cond1", "delta_merge_cond2" });
    dfg.addNode("temp_rc_addr", "Add", { "r_lc", "c" });
    dfg.addNode("temp_rc", "Load", { "temp_rc_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("result", "Add", { "temp_rc", "delta_merge_cond1" });

    // Cond1_true
    dfg.addNode("delta_cond1", "Mul", { "cond1", "RX", "RY", "RZ", "CAP" });  // Pre-determined

    // Cond1_false
    dfg.addNode("cond2", "Cmp", { "cond1", "r_lc", "c" });
    dfg.addNode("delta_merge_cond2", "selPartial", { "cond2", "delta_cond2", "delta_merge_cond3" });

    // Cond2_true
    dfg.addNode("power_c_cond2_addr", "And", { "cond2", "c" });
    dfg.addNode("temp_c_cond2_addr", "And", { "cond2", "c" });
    dfg.addNode("power_c_cond2", "Load", { "power_c_cond2_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_1_cond2", "Load", { "temp_c_cond2_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("temp_c_cond2", "Load", { "temp_c_cond2_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_col_cond2", "Load", { "temp_c_cond2_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("delta_cond2", "Mul", { "cond2", "power_c_cond2", "temp_c_1_cond2", "temp_c_cond2", "temp_c_col_cond2" });

    // Cond2_false
    dfg.addNode("cond3", "Cmp", { "cond2", "r_lc" });
    dfg.addNode("delta_merge_cond3", "selPartial", { "cond3", "delta_cond3_t", "delta_cond3_f" });

    // Cond3_true
    dfg.addNode("power_c_cond3_t_addr", "And", { "cond3", "c" });
    dfg.addNode("temp_c_cond3_t_addr", "And", { "cond3", "c" });
    dfg.addNode("power_c_cond3_t", "Load", { "power_c_cond3_t_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_1_cond3_t", "Load", { "temp_c_cond3_t_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("temp_c_cond3_t", "Load", { "temp_c_cond3_t_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_col_cond3_t", "Load", { "temp_c_cond3_t_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("delta_cond3_t", "Mul", { "cond3", "power_c_cond3_t", "temp_c_1_cond3_t", "temp_c_cond3_t", "temp_c_col_cond3_t" });

    // Cond3_false
    dfg.addNode("power_c_cond3_f_addr", "And", { "cond3", "c" });
    dfg.addNode("temp_c_cond3_f_addr", "And", { "cond3", "c" });
    dfg.addNode("power_c_cond3_f", "Load", { "power_c_cond3_f_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_1_cond3_f", "Load", { "temp_c_cond3_f_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("temp_c_cond3_f", "Load", { "temp_c_cond3_f_addr" }, &power, power_BaseAddr);
    dfg.addNode("temp_c_col_cond3_f", "Load", { "temp_c_cond3_f_addr" }, &temp, temp_BaseAddr);
    dfg.addNode("delta_cond3_f", "Mul", { "cond3", "power_c_cond3_f", "temp_c_1_cond3_f", "temp_c_cond3_f", "temp_c_col_cond3_f" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_r", { "begin", "end", "r", "r_lc" });
    dfg.addNodes2CtrlTree("loop_c", { "c", "cond1", "delta_merge_cond1", "temp_rc_addr", "temp_rc", "result" });
    dfg.addNodes2CtrlTree("cond1_true", { "delta_cond1" });
    dfg.addNodes2CtrlTree("cond1_false", { "cond2", "delta_merge_cond2" });
    dfg.addNodes2CtrlTree("cond2_true", { "power_c_cond2_addr", "temp_c_cond2_addr", "power_c_cond2", "temp_c_1_cond2", "temp_c_cond2", "temp_c_col_cond2", "delta_cond2" });
    dfg.addNodes2CtrlTree("cond2_false", { "cond3", "delta_merge_cond3" });
    dfg.addNodes2CtrlTree("cond3_true", { "power_c_cond3_t_addr", "temp_c_cond3_t_addr", "power_c_cond3_t", "temp_c_1_cond3_t", "temp_c_cond3_t", "temp_c_col_cond3_t", "delta_cond3_t" });
    dfg.addNodes2CtrlTree("cond3_false", { "power_c_cond3_f_addr", "temp_c_cond3_f_addr", "power_c_cond3_f", "temp_c_1_cond3_f", "temp_c_cond3_f", "temp_c_col_cond3_f", "delta_cond3_f" });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_r", "c");
    dfg.setTheTailNode("loop_c", "result" /*"real_odd_update"*/);

    dfg.plotDot();
}


// Graph partition
void HotSpot_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 0;

        chanGraph.getNode("Chan_cond2")->subgraphId = 0;
        chanGraph.getNode("Chan_cond3")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 0;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 0;

        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 0;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 0;
        chanGraph.getNode("Chan_result")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 1;

        chanGraph.getNode("Chan_cond3")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 1;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_result")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 1;

        chanGraph.getNode("Chan_cond3")->subgraphId = 2;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 2;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 2;
        chanGraph.getNode("Chan_result")->subgraphId = 2;


        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 0;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 0;

        chanGraph.getNode("Chan_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 1;

        chanGraph.getNode("Chan_cond3")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 1;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 2;
        chanGraph.getNode("Chan_result")->subgraphId = 2;*/

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 2;

        chanGraph.getNode("Chan_cond3")->subgraphId = 3;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 3;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 3;
        chanGraph.getNode("Chan_result")->subgraphId = 3;

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 0;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 0;

        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 0;

        chanGraph.getNode("Chan_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 1;

        chanGraph.getNode("Chan_cond3")->subgraphId = 2;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 3;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 3;
        chanGraph.getNode("Chan_result")->subgraphId = 3;*/

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 3;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 3;

        chanGraph.getNode("Chan_cond3")->subgraphId = 4;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 4;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 4;
        chanGraph.getNode("Chan_result")->subgraphId = 4;

        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 2;

        chanGraph.getNode("Chan_cond3")->subgraphId = 3;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 3;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 3;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 4;
        chanGraph.getNode("Chan_result")->subgraphId = 4;*/

        break;
    }
    case 6:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 2;

        chanGraph.getNode("Chan_cond3")->subgraphId = 3;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 4;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 5;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 5;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 5;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 5;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 5;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 5;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 5;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 5;
        chanGraph.getNode("Chan_result")->subgraphId = 5;

        break;
    }
    case 7:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_r")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;
        chanGraph.getNode("Lc_c")->subgraphId = 0;
        chanGraph.getNode("Chan_r_lc_scatter_loop_c")->subgraphId = 0;
        chanGraph.getNode("Chan_cond1")->subgraphId = 1;
        chanGraph.getNode("Chan_temp_rc_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_temp_rc")->subgraphId = 1;
        chanGraph.getNode("Chan_delta_cond1")->subgraphId = 1;

        chanGraph.getNode("Chan_cond2")->subgraphId = 2;
        chanGraph.getNode("Chan_temp_c_cond2_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_temp_c_col_cond2")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_cond2")->subgraphId = 3;
        chanGraph.getNode("Lse_temp_c_1_cond2")->subgraphId = 3;
        chanGraph.getNode("Chan_power_c_cond2_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_power_c_cond2")->subgraphId = 4;
        chanGraph.getNode("Chan_delta_cond2")->subgraphId = 4;

        chanGraph.getNode("Chan_cond3")->subgraphId = 5;
        chanGraph.getNode("Chan_power_c_cond3_t_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_temp_c_cond3_t_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_power_c_cond3_t")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_1_cond3_t")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_cond3_t")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_col_cond3_t")->subgraphId = 6;
        chanGraph.getNode("Chan_delta_cond3_t")->subgraphId = 6;

        chanGraph.getNode("Chan_power_c_cond3_f_addr")->subgraphId = 6;
        chanGraph.getNode("Chan_temp_c_cond3_f_addr")->subgraphId = 6;
        chanGraph.getNode("Lse_power_c_cond3_f")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_1_cond3_f")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_cond3_f")->subgraphId = 6;
        chanGraph.getNode("Lse_temp_c_col_cond3_f")->subgraphId = 6;
        chanGraph.getNode("Chan_delta_cond3_f")->subgraphId = 6;
        chanGraph.getNode("Chan_delta_merge_cond3")->subgraphId = 6;
        chanGraph.getNode("Chan_delta_merge_cond2")->subgraphId = 6;
        chanGraph.getNode("Chan_delta_merge_cond1")->subgraphId = 6;
        chanGraph.getNode("Chan_result")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}