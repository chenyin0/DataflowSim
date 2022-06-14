#include "./cfd.h"

using namespace DFSimTest;

Dfg Cfd_Test::dfg;

uint Cfd_Test::nelr = 4096;
uint Cfd_Test::VAR_DENSITY = 0;
uint Cfd_Test::VAR_MOMENTUM = 1;
uint Cfd_Test::VAR_DENSITY_ENERGY = 1 + 3;
;
uint Cfd_Test::block_length = 10;
vector<int> Cfd_Test::variables;
vector<int> Cfd_Test::normals;

uint Cfd_Test::variables_BaseAddr = 0;
uint Cfd_Test::normals_BaseAddr = 256985;

void Cfd_Test::generateData()
{
    uint size = nelr + VAR_DENSITY_ENERGY * nelr;
    variables.resize(size);
    for (size_t i = 0; i < variables.size(); ++i)
    {
        variables[i] = i;
    }

    normals.resize(size);
    for (size_t i = 0; i < normals.size(); ++i)
    {
        normals[i] = i;
    }
}

void Cfd_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { std::make_tuple<string, string, string>("loop_blk", "Loop", "Null"),
         std::make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         std::make_tuple<string, string, string>("loop_j", "Loop", "Null")
        });

    dfg.controlTree.addLowerControlRegion("loop_blk", { "loop_i" });
    dfg.controlTree.addLowerControlRegion("loop_i", { "loop_j" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    dfg.addNode("nelr", "Const", Cfd_Test::nelr);
    // loop_blk
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "blk" });
    dfg.addNode("blk", "Loop_head", {}, { "begin" });
    dfg.addNode("blk_lc", "Nop", { "blk" });
    dfg.addNode("b_start", "Mul", { "blk"});
    dfg.addNode("b_end", "Mul", { "blk" });  // (blk+1)*block_length > nelr ? nelr : (blk+1)*block_length;

    // loop_i
    dfg.addNode("i", "Loop_head", {"b_start", "b_end"}, { "blk_lc" });
    dfg.addNode("i_lc", "Nop", { "i" });
    dfg.addNode("density_i_addr", "Add", {"i"});  // i + VAR_DENSITY*nelr
    dfg.addNode("density_i", "Load", { "density_i_addr" }, &variables, variables_BaseAddr);
    dfg.addNode("momentum_x_addr", "Add", { "i" });  // i + (VAR_MOMENTUM+0)*nelr
    dfg.addNode("momentum_y_addr", "Add", { "i" });  // i + (VAR_MOMENTUM+1)*nelr
    dfg.addNode("momentum_z_addr", "Add", { "i" });  // i + (VAR_MOMENTUM+2)*nelr
    dfg.addNode("momentum_x", "Load", { "momentum_x_addr" }, &variables, variables_BaseAddr);
    dfg.addNode("momentum_y", "Load", { "momentum_y_addr" }, &variables, variables_BaseAddr);
    dfg.addNode("momentum_z", "Load", { "momentum_z_addr" }, &variables, variables_BaseAddr);
    dfg.addNode("density_energy_i_addr", "Add", { "i" });
    dfg.addNode("density_energy_i", "Load", { "density_energy_i_addr" }, &variables, variables_BaseAddr);
    dfg.addNode("velocity_x", "Div", { "momentum_x", "density_i" });
    dfg.addNode("velocity_y", "Div", { "momentum_y", "density_i" });
    dfg.addNode("velocity_z", "Div", { "momentum_z", "density_i" });
    dfg.addNode("velocity_x_2", "Mul", { "velocity_x", "velocity_x" });
    dfg.addNode("velocity_y_2", "Mul", { "velocity_y", "velocity_y" });
    dfg.addNode("velocity_z_2", "Mul", { "velocity_z", "velocity_z" });
    dfg.addNode("speed_sqd", "Add", { "velocity_x_2", "velocity_y_2", "velocity_z_2" });
    dfg.addNode("speed", "Sqrt", { "speed_sqd" });
    dfg.addNode("density_i_speed_sqd", "Mul", { "density_i", "speed_sqd" });
    dfg.addNode("pressure", "Sub", { "density_energy_i", "density_i_speed_sqd" });  // float(0.4f)*(density_energy_i - float(0.5f)*density_i*speed_sqd)
    dfg.addNode("pressure_density_i", "Div", { "pressure", "density_i" });
    //dfg.addNode("GAMMA_pressure_density_i", "Mul", { "pressure_density_i"});  // (GAMMA)*pressure/density_i
    //dfg.addNode("speed_of_sound", "Sqrt", { "GAMMA_pressure_density_i"});

    dfg.addNode("flux_i_momentum_x_x", "Mul", { "velocity_x", "momentum_x" });
    dfg.addNode("flux_i_momentum_x_y", "Mul", { "velocity_x", "momentum_y" });
    dfg.addNode("flux_i_momentum_x_z", "Mul", { "velocity_x", "momentum_z" });

    dfg.addNode("flux_i_momentum_y_x", "Nop", { "flux_i_momentum_x_y"});
    dfg.addNode("flux_i_momentum_y_y", "Mul", { "velocity_y", "momentum_y" });
    dfg.addNode("flux_i_momentum_y_z", "Mul", { "velocity_y", "momentum_z" });

    dfg.addNode("flux_i_momentum_z_x", "Nop", { "flux_i_momentum_x_z"});
    dfg.addNode("flux_i_momentum_z_y", "Nop", { "flux_i_momentum_y_z"});
    dfg.addNode("flux_i_momentum_z_z", "Mul", { "velocity_z", "momentum_z" });

    dfg.addNode("de_p", "Add", { "density_energy_i", "pressure" });
    dfg.addNode("flux_i_density_energy_x", "Mul", { "velocity_x", "de_p" });
    dfg.addNode("flux_i_density_energy_y", "Mul", { "velocity_y", "de_p" });
    dfg.addNode("flux_i_density_energy_z", "Mul", { "velocity_z", "de_p" });

    // loop_j
    dfg.addNode("j", "Loop_head", {}, { "i_lc" });
    dfg.addNode("normal_x_addr", "Add", { "i_lc", "j" });  // i + (j + 0*4)*nelr
    dfg.addNode("normal_y_addr", "Add", { "i_lc", "j" });  // i + (j + 1*4)*nelr
    dfg.addNode("normal_z_addr", "Add", { "i_lc", "j" });  // i + (j + 2*4)*nelr
    dfg.addNode("normal_x", "Load", { "normal_x_addr" }, &normals, normals_BaseAddr);
    dfg.addNode("normal_y", "Load", { "normal_y_addr" }, &normals, normals_BaseAddr);
    dfg.addNode("normal_z", "Load", { "normal_z_addr" }, &normals, normals_BaseAddr);

    dfg.addNode("normal_x_2", "Mul", { "normal_x", "flux_i_density_energy_x" });  // i + (j + 0*4)*nelr
    dfg.addNode("normal_y_2", "Mul", { "normal_y", "flux_i_density_energy_y" });  // i + (j + 1*4)*nelr
    dfg.addNode("normal_z_2", "Mul", { "normal_z", "flux_i_density_energy_z" });  // i + (j + 2*4)*nelr
    dfg.addNode("normal_xyz", "Add", { "normal_x_2", "normal_y_2", "normal_z_2" });
    dfg.addNode("normal_len", "Sqrt", { "normal_xyz" });

    dfg.addNode("density_nb_addr", "Add", { "i_lc", "j" });  // i + (j + 0*4)*nelr
    dfg.addNode("momentum_nb_x_addr", "Add", { "i_lc", "j" });  // i + (j + 1*4)*nelr
    dfg.addNode("momentum_nb_y_addr", "Add", { "i_lc", "j" });  // i + (j + 2*4)*nelr
    dfg.addNode("momentum_nb_z_addr", "Add", { "i_lc", "j" });  // i + (j + 1*4)*nelr
    dfg.addNode("density_energy_nb_addr", "Add", { "i_lc", "j" });  // i + (j + 2*4)*nelr

    //dfg.addNode("density_nb", "Load", { "density_nb_addr" }, &variables, variables_BaseAddr);
    //dfg.addNode("momentum_nb_x", "Load", { "momentum_nb_x_addr" }, &variables, variables_BaseAddr);
    //dfg.addNode("momentum_nb_y", "Load", { "momentum_nb_y_addr" }, &variables, variables_BaseAddr);
    //dfg.addNode("momentum_nb_z", "Load", { "momentum_nb_z_addr" }, &variables, variables_BaseAddr);
    //dfg.addNode("density_energy_nb", "Load", { "density_energy_nb_addr" }, &variables, variables_BaseAddr);

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_blk", { "begin", "end", "blk", "blk_lc", "b_start", "b_end" });
    dfg.addNodes2CtrlTree("loop_i", { "i", "i_lc", "density_i_addr", "density_i", "momentum_x_addr", "momentum_y_addr", "momentum_z_addr",
        "momentum_x", "momentum_y", "momentum_z", "density_energy_i_addr", "density_energy_i", "velocity_x", "velocity_y", 
        "velocity_z", "velocity_x_2", "velocity_y_2", "velocity_z_2", "speed_sqd", "speed", "density_i_speed_sqd",
        "pressure", "pressure_density_i", /*"GAMMA_pressure_density_i",*/ /*"speed_of_sound",*/ "flux_i_momentum_x_x", "flux_i_momentum_x_y", "flux_i_momentum_x_z",
        "flux_i_momentum_y_x", "flux_i_momentum_y_y", "flux_i_momentum_y_z", "flux_i_momentum_z_x", "flux_i_momentum_z_y", "flux_i_momentum_z_z",
        "de_p", "flux_i_density_energy_x", "flux_i_density_energy_y", "flux_i_density_energy_z" });
    dfg.addNodes2CtrlTree("loop_j", { "j", "normal_x_addr", "normal_y_addr", "normal_z_addr", "normal_x", "normal_y", "normal_z",
        "normal_x_2", "normal_y_2", "normal_z_2", "normal_xyz", "normal_len", "density_nb_addr", "momentum_nb_x_addr", "momentum_nb_y_addr", "momentum_nb_z_addr",
        "density_energy_nb_addr"/*, "density_nb", "momentum_nb_x", "momentum_nb_y", "momentum_nb_z", "density_energy_nb"*/ });

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_blk", "i");
    dfg.setTheTailNode("loop_i", "j");
    dfg.setTheTailNode("loop_j", "normal_len");

    dfg.plotDot();
}


// Graph partition
void Cfd_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_density_i")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 0;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_speed")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 0;
        chanGraph.getNode("Chan_de_p")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 0;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 0;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 0;
        /*chanGraph.getNode("Lse_density_nb")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 0;
        chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 0;*/
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 0;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 0;

        break;
    }
    case 2:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_density_i")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 0;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_speed")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 0;
        chanGraph.getNode("Chan_de_p")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 1;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 1;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 1;
        /*chanGraph.getNode("Lse_density_nb")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 1;
        chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 1;*/
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 1;

        break;
    }
    case 3:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_blk")->subgraphId = 0;
        //chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        //chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        //chanGraph.getNode("Lse_density_i")->subgraphId = 0;
        //chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        //chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        //chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        //chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_z")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 0;
        //chanGraph.getNode("Chan_speed_sqd")->subgraphId = 0;
        //chanGraph.getNode("Chan_speed")->subgraphId = 0;
        //chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 0;
        //chanGraph.getNode("Chan_pressure")->subgraphId = 0;
        //chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_de_p")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 0;
        //chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 0;

        //chanGraph.getNode("Lc_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 1;
        //chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 1;
        //chanGraph.getNode("Lse_normal_x")->subgraphId = 1;
        //chanGraph.getNode("Lse_normal_y")->subgraphId = 1;
        //chanGraph.getNode("Lse_normal_z")->subgraphId = 1;
        ///*chanGraph.getNode("Lse_density_nb")->subgraphId = 1;
        //chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 2;
        //chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 2;
        //chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 2;
        //chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 2;*/
        //chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 1;
        //chanGraph.getNode("Chan_normal_x_2")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_y_2")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_z_2")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_xyz")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_len")->subgraphId = 2;

        //break;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_density_i")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 0;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_speed")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 0;
        chanGraph.getNode("Chan_de_p")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 0;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 0;

        chanGraph.getNode("Lc_j")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 2;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 2;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 2;
        /*chanGraph.getNode("Lse_density_nb")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 2;
        chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 2;*/
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        chanGraph.getNode("Lse_density_i")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 0;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 0;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 0;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_speed")->subgraphId = 0;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure")->subgraphId = 0;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 0;
        chanGraph.getNode("Chan_de_p")->subgraphId = 1;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 1;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 1;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 1;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 1;

        chanGraph.getNode("Lc_j")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 2;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 2;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 3;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 3;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 3;
        /*chanGraph.getNode("Lse_density_nb")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 3;
        chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 3;
        chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 3;
        chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 3;*/
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 3;

        //break;

        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_blk")->subgraphId = 0;
        //chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        //chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_density_i_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 0;
        //chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 0;
        //chanGraph.getNode("Lse_density_i")->subgraphId = 1;
        //chanGraph.getNode("Lse_momentum_x")->subgraphId = 0;
        //chanGraph.getNode("Lse_momentum_y")->subgraphId = 0;
        //chanGraph.getNode("Lse_momentum_z")->subgraphId = 0;
        //chanGraph.getNode("Lse_density_energy_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_velocity_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_velocity_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_velocity_z")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 1;
        //chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 1;
        //chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 1;
        //chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 1;
        //chanGraph.getNode("Chan_speed_sqd")->subgraphId = 1;
        //chanGraph.getNode("Chan_speed")->subgraphId = 1;
        //chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 1;
        //chanGraph.getNode("Chan_pressure")->subgraphId = 1;
        //chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 1;
        //chanGraph.getNode("Chan_de_p")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 1;
        //chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_lc")->subgraphId = 1;

        //chanGraph.getNode("Lc_j")->subgraphId = 2;
        //chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 2;
        //chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 2;
        //chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 2;
        //chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 2;
        //chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 2;
        //chanGraph.getNode("Lse_normal_x")->subgraphId = 3;
        //chanGraph.getNode("Lse_normal_y")->subgraphId = 3;
        //chanGraph.getNode("Lse_normal_z")->subgraphId = 3;
        ///*chanGraph.getNode("Lse_density_nb")->subgraphId = 2;
        //chanGraph.getNode("Lse_momentum_nb_x")->subgraphId = 3;
        //chanGraph.getNode("Lse_momentum_nb_y")->subgraphId = 3;
        //chanGraph.getNode("Lse_momentum_nb_z")->subgraphId = 3;
        //chanGraph.getNode("Lse_density_energy_nb")->subgraphId = 3;*/
        //chanGraph.getNode("Chan_normal_x_2")->subgraphId = 3;
        //chanGraph.getNode("Chan_normal_y_2")->subgraphId = 3;
        //chanGraph.getNode("Chan_normal_z_2")->subgraphId = 3;
        //chanGraph.getNode("Chan_normal_xyz")->subgraphId = 3;
        //chanGraph.getNode("Chan_normal_len")->subgraphId = 3;

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_density_i")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 1;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 1;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 2;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 2;
        chanGraph.getNode("Chan_speed")->subgraphId = 2;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 2;
        chanGraph.getNode("Chan_pressure")->subgraphId = 2;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 2;
        chanGraph.getNode("Chan_de_p")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 2;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 2;

        chanGraph.getNode("Lc_j")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 3;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 3;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 3;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 3;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 3;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 4;

        break;
    }
    case 6:
    {
        /*chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_density_i")->subgraphId = 3;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 2;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 3;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 3;
        chanGraph.getNode("Chan_speed")->subgraphId = 3;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 3;
        chanGraph.getNode("Chan_pressure")->subgraphId = 3;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 3;
        chanGraph.getNode("Chan_de_p")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 3;

        chanGraph.getNode("Lc_j")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 5;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 5;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 5;*/

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_density_i")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 1;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 1;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 2;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 2;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 2;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 2;
        chanGraph.getNode("Chan_speed")->subgraphId = 2;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 3;
        chanGraph.getNode("Chan_pressure")->subgraphId = 3;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 3;
        chanGraph.getNode("Chan_de_p")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 3;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 3;

        chanGraph.getNode("Lc_j")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 4;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 4;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 5;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 5;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 5;

        break;
    }
    case 7:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_blk")->subgraphId = 0;
        chanGraph.getNode("Chan_blk_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_b_start")->subgraphId = 0;
        chanGraph.getNode("Chan_b_end")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 1;
        chanGraph.getNode("Chan_density_i_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_x_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_y_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_momentum_z_addr")->subgraphId = 1;
        chanGraph.getNode("Chan_density_energy_i_addr")->subgraphId = 1;
        chanGraph.getNode("Lse_density_i")->subgraphId = 3;
        chanGraph.getNode("Lse_momentum_x")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_y")->subgraphId = 2;
        chanGraph.getNode("Lse_momentum_z")->subgraphId = 2;
        chanGraph.getNode("Lse_density_energy_i")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_x")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_y")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_x_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_z")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_y_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_x")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_y")->subgraphId = 3;
        chanGraph.getNode("Chan_flux_i_momentum_z_z")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_x_2")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_y_2")->subgraphId = 3;
        chanGraph.getNode("Chan_velocity_z_2")->subgraphId = 3;
        chanGraph.getNode("Chan_speed_sqd")->subgraphId = 3;
        chanGraph.getNode("Chan_speed")->subgraphId = 3;
        chanGraph.getNode("Chan_density_i_speed_sqd")->subgraphId = 4;
        chanGraph.getNode("Chan_pressure")->subgraphId = 4;
        chanGraph.getNode("Chan_pressure_density_i")->subgraphId = 4;
        chanGraph.getNode("Chan_de_p")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_x")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_y")->subgraphId = 4;
        chanGraph.getNode("Chan_flux_i_density_energy_z")->subgraphId = 4;
        chanGraph.getNode("Chan_i_lc")->subgraphId = 4;

        chanGraph.getNode("Lc_j")->subgraphId = 5;
        chanGraph.getNode("Chan_i_lc_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_flux_i_density_energy_x_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_flux_i_density_energy_y_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_flux_i_density_energy_z_scatter_loop_j")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_x_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_y_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_normal_z_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_density_nb_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_momentum_nb_x_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_momentum_nb_y_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_momentum_nb_z_addr")->subgraphId = 5;
        chanGraph.getNode("Chan_density_energy_nb_addr")->subgraphId = 5;
        chanGraph.getNode("Lse_normal_x")->subgraphId = 6;
        chanGraph.getNode("Lse_normal_y")->subgraphId = 6;
        chanGraph.getNode("Lse_normal_z")->subgraphId = 6;
        chanGraph.getNode("Chan_normal_x_2")->subgraphId = 6;
        chanGraph.getNode("Chan_normal_y_2")->subgraphId = 6;
        chanGraph.getNode("Chan_normal_z_2")->subgraphId = 6;
        chanGraph.getNode("Chan_normal_xyz")->subgraphId = 6;
        chanGraph.getNode("Chan_normal_len")->subgraphId = 6;

        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}