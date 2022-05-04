#include "./cfd.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"

using namespace DFSimTest;

void Cfd_Test::cfd_Base(Debug* debug)
{
    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    //*** Declare Profiler
    Profiler* profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 500);

    //*** Define subgraph scheduler
    GraphScheduler* graphScheduler = new GraphScheduler();

    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(Cfd_Test::dfg);
    chanGraph.addSpecialModeChan();

    int splitNum = 5;
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Cfd_Test::graphPartition(chanGraph, splitNum);

    //chanGraph.printSubgraphPartition(splitNum, debug);
    chanGraph.addChanDGSF();

    chanGraph.addNodeDelay();
    chanGraph.setSpeedup(debug);  // Set speedup
    chanGraph.plotDot();


    // Generate benchmark data
    generateData();

    registry->genModule(chanGraph);
    registry->genConnect(chanGraph);
    registry->setChanSize();
    registry->configGraphScheduler(graphScheduler);
    auto& regis = registry->getRegistryTable();  // For exposing registry in debugging
    //debug->printRegistry(registry);
    //debug->printSimNodes(chanGraph);
    registry->genSimConfig(chanGraph);  // Only used for initializing the first time sim
    const auto& debugPrint = registry->genDebugPrint(chanGraph);
    auto simChans = get<0>(debugPrint);
    auto simLcs = get<1>(debugPrint);


    ////*** Simulate
    // Declare
    //***********************************************************************

    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_blk = registry->getLc("Lc_blk");
    const auto& Chan_blk_lc = registry->getChan("Chan_blk_lc");
    const auto& Chan_b_start = registry->getChan("Chan_b_start");
    const auto& Chan_b_end = registry->getChan("Chan_b_end");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_density_i_addr = registry->getChan("Chan_density_i_addr");
    const auto& Chan_momentum_x_addr = registry->getChan("Chan_momentum_x_addr");
    const auto& Chan_momentum_y_addr = registry->getChan("Chan_momentum_y_addr");
    const auto& Chan_momentum_z_addr = registry->getChan("Chan_momentum_z_addr");
    const auto& Chan_density_energy_i_addr = registry->getChan("Chan_density_energy_i_addr");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Lse_density_i = registry->getChan("Lse_density_i");
    const auto& Lse_momentum_x = registry->getChan("Lse_momentum_x");
    const auto& Lse_momentum_y = registry->getChan("Lse_momentum_y");
    const auto& Lse_momentum_z = registry->getChan("Lse_momentum_z");
    const auto& Lse_density_energy_i = registry->getChan("Lse_density_energy_i");
    const auto& Chan_i_lc_scatter_loop_j = registry->getChan("Chan_i_lc_scatter_loop_j");
    const auto& Chan_velocity_x = registry->getChan("Chan_velocity_x");
    const auto& Chan_velocity_y = registry->getChan("Chan_velocity_y");
    const auto& Chan_velocity_z = registry->getChan("Chan_velocity_z");
    const auto& Chan_normal_x_addr = registry->getChan("Chan_normal_x_addr");
    const auto& Chan_normal_y_addr = registry->getChan("Chan_normal_y_addr");
    const auto& Chan_normal_z_addr = registry->getChan("Chan_normal_z_addr");
    const auto& Chan_density_nb_addr = registry->getChan("Chan_density_nb_addr");
    const auto& Chan_momentum_nb_x_addr = registry->getChan("Chan_momentum_nb_x_addr");
    const auto& Chan_momentum_nb_y_addr = registry->getChan("Chan_momentum_nb_y_addr");
    const auto& Chan_momentum_nb_z_addr = registry->getChan("Chan_momentum_nb_z_addr");
    const auto& Chan_density_energy_nb_addr = registry->getChan("Chan_density_energy_nb_addr");
    const auto& Chan_velocity_x_2 = registry->getChan("Chan_velocity_x_2");
    const auto& Chan_flux_i_momentum_x_x = registry->getChan("Chan_flux_i_momentum_x_x");
    const auto& Chan_flux_i_momentum_x_y = registry->getChan("Chan_flux_i_momentum_x_y");
    const auto& Chan_flux_i_momentum_x_z = registry->getChan("Chan_flux_i_momentum_x_z");
    const auto& Chan_velocity_y_2 = registry->getChan("Chan_velocity_y_2");
    const auto& Chan_flux_i_momentum_y_y = registry->getChan("Chan_flux_i_momentum_y_y");
    const auto& Chan_flux_i_momentum_y_z = registry->getChan("Chan_flux_i_momentum_y_z");
    const auto& Chan_velocity_z_2 = registry->getChan("Chan_velocity_z_2");
    const auto& Chan_flux_i_momentum_z_z = registry->getChan("Chan_flux_i_momentum_z_z");
    const auto& Lse_normal_x = registry->getChan("Lse_normal_x");
    const auto& Lse_normal_y = registry->getChan("Lse_normal_y");
    const auto& Lse_normal_z = registry->getChan("Lse_normal_z");
    /*const auto& Lse_density_nb = registry->getChan("Lse_density_nb");
    const auto& Lse_momentum_nb_x = registry->getChan("Lse_momentum_nb_x");
    const auto& Lse_momentum_nb_y = registry->getChan("Lse_momentum_nb_y");
    const auto& Lse_momentum_nb_z = registry->getChan("Lse_momentum_nb_z");
    const auto& Lse_density_energy_nb = registry->getChan("Lse_density_energy_nb");*/
    const auto& Chan_flux_i_momentum_y_x = registry->getChan("Chan_flux_i_momentum_y_x");
    const auto& Chan_flux_i_momentum_z_x = registry->getChan("Chan_flux_i_momentum_z_x");
    const auto& Chan_flux_i_momentum_z_y = registry->getChan("Chan_flux_i_momentum_z_y");
    const auto& Chan_speed_sqd = registry->getChan("Chan_speed_sqd");
    const auto& Chan_normal_x_2 = registry->getChan("Chan_normal_x_2");
    const auto& Chan_normal_y_2 = registry->getChan("Chan_normal_y_2");
    const auto& Chan_normal_z_2 = registry->getChan("Chan_normal_z_2");
    const auto& Chan_speed = registry->getChan("Chan_speed");
    const auto& Chan_density_i_speed_sqd = registry->getChan("Chan_density_i_speed_sqd");
    const auto& Chan_normal_xyz = registry->getChan("Chan_normal_xyz");
    const auto& Chan_pressure = registry->getChan("Chan_pressure");
    const auto& Chan_normal_len = registry->getChan("Chan_normal_len");
    const auto& Chan_pressure_density_i = registry->getChan("Chan_pressure_density_i");
    const auto& Chan_de_p = registry->getChan("Chan_de_p");
    //const auto& Chan_GAMMA_pressure_density_i = registry->getChan("Chan_GAMMA_pressure_density_i");
    const auto& Chan_flux_i_density_energy_x = registry->getChan("Chan_flux_i_density_energy_x");
    const auto& Chan_flux_i_density_energy_y = registry->getChan("Chan_flux_i_density_energy_y");
    const auto& Chan_flux_i_density_energy_z = registry->getChan("Chan_flux_i_density_energy_z");
    //const auto& Chan_speed_of_sound = registry->getChan("Chan_speed_of_sound");
    const auto& Chan_flux_i_density_energy_x_scatter_loop_j = registry->getChan("Chan_flux_i_density_energy_x_scatter_loop_j");
    const auto& Chan_flux_i_density_energy_y_scatter_loop_j = registry->getChan("Chan_flux_i_density_energy_y_scatter_loop_j");
    const auto& Chan_flux_i_density_energy_z_scatter_loop_j = registry->getChan("Chan_flux_i_density_energy_z_scatter_loop_j");

    //***********************************************************************

    // User defined
    /*registry->getLse("Lse_density_i")->noLatencyMode = 1;
    registry->getLse("Lse_momentum_y")->noLatencyMode = 1;
    registry->getLse("Lse_momentum_z")->noLatencyMode = 1;
    registry->getLse("Lse_density_energy_i")->noLatencyMode = 1;*/

    // Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //graphScheduler->graphSwitchO3 = 0;
    //registry->pathBalance();
    profiler->init();
    watchdog.addCheckPointChan({ Lc_blk->getEnd, Lc_i->getEnd, Lc_j->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint iter = 0;

    //vector<int> res;  // Result
    //vector<int> temp; // temp_result

    uint max_iter = 5000000;// 5000000;
    uint segment = max_iter / 100;
    uint percent = 0;


    //*** Record run time
    clock_t startTime, endTime;
    startTime = clock();

    // Execute
    while (iter < max_iter)
    {
        watchdog.feedDog(iter);

        // Print progress bar
        if (iter / segment > percent)
        {
            percent = iter / segment;
            std::cout << ">>>>>> Progress: " << percent << "%" << "\t Iter: " << iter << std::endl;
        }

        DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
        int clk = DFSim::ClkDomain::getInstance()->getClk();
        debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
        debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

        // Below is user-defined sim
        // Do not forget invalid Chan_begin after the outer-most Lc
        // *************************************************************************************

        // Lc_blk
        Lc_blk->var = Lc_blk->mux->mux(Lc_blk->var, 0, Lc_blk->sel);
        Lc_blk->mux->muxUpdate(Lc_blk->sel);
        Lc_blk->mux->outChan->value = Lc_blk->var;
        Lc_blk->loopVar->get();
        Lc_blk->loopVar->value = Lc_blk->loopVar->assign(Lc_blk->mux->outChan);  // After get(), must update chan's value
        Lc_blk->var = Lc_blk->loopVar->value + 1;
        Lc_blk->lcUpdate(Lc_blk->var < nelr / block_length);

        Chan_blk_lc->get();	// Nop	[0]Lc_blk 
        Chan_blk_lc->value = Chan_blk_lc->assign(uint(0));

        Chan_b_start->get();	// Mul	[0]Lc_blk 
        Chan_b_start->value = Chan_b_start->assign(uint(0)) * block_length;

        Chan_b_end->get();	// Mul	[0]Lc_blk 
        Chan_b_end->value = (Chan_b_end->assign(uint(0)) + 1) * block_length > nelr ? nelr : (Chan_b_end->assign(uint(0)) + 1) * block_length;

        // Lc_i
        Lc_i->var = Lc_i->mux->mux(Lc_i->var, Chan_b_start->value, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < Chan_b_end->value);

        Chan_i_lc->get();	// Nop	[0]Lc_i 
        Chan_i_lc->value = Chan_i_lc->assign(uint(0));

        Chan_density_i_addr->get();	// Add	[0]Lc_i 
        Chan_density_i_addr->value = Chan_density_i_addr->assign(uint(0)) + nelr * VAR_DENSITY;

        Chan_momentum_x_addr->get();	// Add	[0]Lc_i 
        Chan_momentum_x_addr->value = Chan_momentum_x_addr->assign(uint(0)) + (VAR_MOMENTUM + 0) * nelr;

        Chan_momentum_y_addr->get();	// Add	[0]Lc_i 
        Chan_momentum_y_addr->value = Chan_momentum_y_addr->assign(uint(0)) + (VAR_MOMENTUM + 1) * nelr;

        Chan_momentum_z_addr->get();	// Add	[0]Lc_i 
        Chan_momentum_z_addr->value = Chan_momentum_z_addr->assign(uint(0)) + (VAR_MOMENTUM + 2) * nelr;

        Chan_density_energy_i_addr->get();	// Add	[0]Lc_i 
        Chan_density_energy_i_addr->value = Chan_density_energy_i_addr->assign(uint(0)) + VAR_DENSITY_ENERGY * nelr;

        // Lc_j
        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var < 4);

        Lse_density_i->get();	// Load	[0]Chan_density_i_addr 
        Lse_density_i->value = variables[Lse_density_i->assign()];

        Lse_momentum_x->get();	// Load	[0]Chan_density_i_addr 
        Lse_momentum_x->value = variables[Lse_momentum_x->assign()];

        Lse_momentum_y->get();	// Load	[0]Chan_density_i_addr 
        Lse_momentum_y->value = variables[Lse_momentum_y->assign()];

        Lse_momentum_z->get();	// Load	[0]Chan_density_i_addr 
        Lse_momentum_z->value = variables[Lse_momentum_z->assign()];

        Lse_density_energy_i->get();	// Load	[0]Chan_density_energy_i_addr 
        Lse_density_energy_i->value = variables[Lse_density_energy_i->assign()];

        Chan_i_lc_scatter_loop_j->get();	// Nop	[0]Chan_i_lc 
        Chan_i_lc_scatter_loop_j->value = Chan_i_lc_scatter_loop_j->assign(uint(0));

        Chan_velocity_x->get();	// Div	[0]Lse_momentum_x [1]Lse_density_i 
        Chan_velocity_x->value = Chan_velocity_x->assign(uint(0)) / max(Chan_velocity_x->assign(uint(1)), 1);

        Chan_velocity_y->get();	// Div	[0]Lse_momentum_y [1]Lse_density_i 
        Chan_velocity_y->value = Chan_velocity_y->assign(uint(0)) / max(Chan_velocity_y->assign(uint(1)), 1);

        Chan_velocity_z->get();	// Div	[0]Lse_momentum_z [1]Lse_density_i 
        Chan_velocity_z->value = Chan_velocity_z->assign(uint(0)) / max(Chan_velocity_z->assign(uint(1)), 1);

        Chan_normal_x_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_normal_x_addr->value = Chan_normal_x_addr->assign(uint(0)) + Chan_normal_x_addr->assign(uint(1));

        Chan_normal_y_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_normal_y_addr->value = Chan_normal_y_addr->assign(uint(0)) + Chan_normal_y_addr->assign(uint(1));

        Chan_normal_z_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_normal_z_addr->value = Chan_normal_z_addr->assign(uint(0)) + Chan_normal_z_addr->assign(uint(1));

        Chan_density_nb_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_density_nb_addr->value = Chan_density_nb_addr->assign(uint(0)) + Chan_density_nb_addr->assign(uint(1));

        Chan_momentum_nb_x_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_momentum_nb_x_addr->value = Chan_momentum_nb_x_addr->assign(uint(0)) + Chan_momentum_nb_x_addr->assign(uint(1));

        Chan_momentum_nb_y_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_momentum_nb_y_addr->value = Chan_momentum_nb_y_addr->assign(uint(0)) + Chan_momentum_nb_y_addr->assign(uint(1));

        Chan_momentum_nb_z_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_momentum_nb_z_addr->value = Chan_momentum_nb_z_addr->assign(uint(0)) + Chan_momentum_nb_z_addr->assign(uint(1));

        Chan_density_energy_nb_addr->get();	// Add	[0]Chan_i_lc_scatter_loop_j [1]Lc_j 
        Chan_density_energy_nb_addr->value = Chan_density_energy_nb_addr->assign(uint(0)) + Chan_density_energy_nb_addr->assign(uint(1));

        Chan_velocity_x_2->get();	// Mul	[0]Chan_velocity_x 
        Chan_velocity_x_2->value = Chan_velocity_x_2->assign(uint(0)) * Chan_velocity_x_2->assign(uint(0));

        Chan_flux_i_momentum_x_x->get();	// Mul	[0]Chan_velocity_x [1]Lse_momentum_x 
        Chan_flux_i_momentum_x_x->value = Chan_flux_i_momentum_x_x->assign(uint(0)) * Chan_flux_i_momentum_x_x->assign(uint(1));

        Chan_flux_i_momentum_x_y->get();	// Mul	[0]Chan_velocity_x [1]Lse_momentum_y 
        Chan_flux_i_momentum_x_y->value = Chan_flux_i_momentum_x_y->assign(uint(0)) * Chan_flux_i_momentum_x_y->assign(uint(1));

        Chan_flux_i_momentum_x_z->get();	// Mul	[0]Chan_velocity_x [1]Lse_momentum_z 
        Chan_flux_i_momentum_x_z->value = Chan_flux_i_momentum_x_z->assign(uint(0)) * Chan_flux_i_momentum_x_z->assign(uint(1));

        Chan_velocity_y_2->get();	// Mul	[0]Chan_velocity_y 
        Chan_velocity_y_2->value = Chan_velocity_y_2->assign(uint(0)) * Chan_velocity_y_2->assign(uint(0));

        Chan_flux_i_momentum_y_y->get();	// Mul	[0]Chan_velocity_y [1]Lse_momentum_y 
        Chan_flux_i_momentum_y_y->value = Chan_flux_i_momentum_y_y->assign(uint(0)) * Chan_flux_i_momentum_y_y->assign(uint(1));

        Chan_flux_i_momentum_y_z->get();	// Mul	[0]Chan_velocity_y [1]Lse_momentum_z 
        Chan_flux_i_momentum_y_z->value = Chan_flux_i_momentum_y_z->assign(uint(0)) * Chan_flux_i_momentum_y_z->assign(uint(1));

        Chan_velocity_z_2->get();	// Mul	[0]Chan_velocity_z 
        Chan_velocity_z_2->value = Chan_velocity_z_2->assign(uint(0)) * Chan_velocity_z_2->assign(uint(0));

        Chan_flux_i_momentum_z_z->get();	// Mul	[0]Chan_velocity_z [1]Lse_momentum_z 
        Chan_flux_i_momentum_z_z->value = Chan_flux_i_momentum_z_z->assign(uint(0)) * Chan_flux_i_momentum_z_z->assign(uint(1));

        Lse_normal_x->get();	// Load	[0]Chan_normal_x_addr 
        Lse_normal_x->value = normals[Lse_normal_x->assign()];

        Lse_normal_y->get();	// Load	[0]Chan_normal_y_addr 
        Lse_normal_y->value = normals[Lse_normal_y->assign()];

        Lse_normal_z->get();	// Load	[0]Chan_normal_z_addr 
        Lse_normal_z->value = normals[Lse_normal_z->assign()];

        //Lse_density_nb->get();	// Load	[0]Chan_density_nb_addr 
        //Lse_density_nb->value = variables[Lse_density_nb->assign()];

        //Lse_momentum_nb_x->get();	// Load	[0]Chan_momentum_nb_x_addr 
        //Lse_momentum_nb_x->value = variables[Lse_momentum_nb_x->assign()];

        //Lse_momentum_nb_y->get();	// Load	[0]Chan_momentum_nb_y_addr 
        //Lse_momentum_nb_y->value = variables[Lse_momentum_nb_y->assign()];

        //Lse_momentum_nb_z->get();	// Load	[0]Chan_momentum_nb_z_addr 
        //Lse_momentum_nb_z->value = variables[Lse_momentum_nb_z->assign()];

        //Lse_density_energy_nb->get();	// Load	[0]Chan_density_energy_nb_addr 
        //Lse_density_energy_nb->value = variables[Lse_density_energy_nb->assign()];

        Chan_flux_i_momentum_y_x->get();	// Nop	[0]Chan_flux_i_momentum_x_y 
        Chan_flux_i_momentum_y_x->value = Chan_flux_i_momentum_y_x->assign(uint(0));

        Chan_flux_i_momentum_z_x->get();	// Nop	[0]Chan_flux_i_momentum_x_z 
        Chan_flux_i_momentum_z_x->value = Chan_flux_i_momentum_z_x->assign(uint(0));

        Chan_flux_i_momentum_z_y->get();	// Nop	[0]Chan_flux_i_momentum_y_z 
        Chan_flux_i_momentum_z_y->value = Chan_flux_i_momentum_z_y->assign(uint(0));

        Chan_speed_sqd->get();	// Add	[0]Chan_velocity_x_2 [1]Chan_velocity_y_2 [2]Chan_velocity_z_2 
        Chan_speed_sqd->value = Chan_speed_sqd->assign(uint(0)) + Chan_speed_sqd->assign(uint(1)) + Chan_speed_sqd->assign(uint(2));

        Chan_flux_i_density_energy_x_scatter_loop_j->get();	// Nop	[0]Chan_flux_i_density_energy_x 
        Chan_flux_i_density_energy_x_scatter_loop_j->value = Chan_flux_i_density_energy_x_scatter_loop_j->assign(uint(0));

        Chan_flux_i_density_energy_y_scatter_loop_j->get();	// Nop	[0]Chan_flux_i_density_energy_y 
        Chan_flux_i_density_energy_y_scatter_loop_j->value = Chan_flux_i_density_energy_y_scatter_loop_j->assign(uint(0));

        Chan_flux_i_density_energy_z_scatter_loop_j->get();	// Nop	[0]Chan_flux_i_density_energy_z 
        Chan_flux_i_density_energy_z_scatter_loop_j->value = Chan_flux_i_density_energy_z_scatter_loop_j->assign(uint(0));

        Chan_normal_x_2->get();	// Mul	[0]Lse_normal_x 
        Chan_normal_x_2->value = Chan_normal_x_2->assign(uint(0)) * Chan_normal_x_2->assign(uint(0));

        Chan_normal_y_2->get();	// Mul	[0]Lse_normal_y 
        Chan_normal_y_2->value = Chan_normal_y_2->assign(uint(0)) * Chan_normal_y_2->assign(uint(0));

        Chan_normal_z_2->get();	// Mul	[0]Lse_normal_z 
        Chan_normal_z_2->value = Chan_normal_z_2->assign(uint(0)) * Chan_normal_z_2->assign(uint(0));

        Chan_speed->get();	// Sqrt	[0]Chan_speed_sqd 
        Chan_speed->value = std::sqrt(Chan_speed->assign(uint(0)));

        Chan_density_i_speed_sqd->get();	// Mul	[0]Lse_density_i [1]Chan_speed_sqd 
        Chan_density_i_speed_sqd->value = Chan_density_i_speed_sqd->assign(uint(0)) * Chan_density_i_speed_sqd->assign(uint(1));

        Chan_normal_xyz->get();	// Add	[0]Chan_normal_x_2 [1]Chan_normal_y_2 [2]Chan_normal_z_2 
        Chan_normal_xyz->value = Chan_normal_xyz->assign(uint(0)) + Chan_normal_xyz->assign(uint(1)) + Chan_normal_xyz->assign(uint(2));

        Chan_pressure->get();	// Sub	[0]Lse_density_energy_i [1]Chan_density_i_speed_sqd 
        Chan_pressure->value = Chan_pressure->assign(uint(0)) - Chan_pressure->assign(uint(1));

        Chan_normal_len->get();	// Sqrt	[0]Chan_normal_xyz 
        Chan_normal_len->value = std::sqrt(Chan_normal_len->assign(uint(0)));

        Chan_pressure_density_i->get();	// Div	[0]Chan_pressure [1]Lse_density_i 
        Chan_pressure_density_i->value = Chan_pressure_density_i->assign(uint(0)) / max(Chan_pressure_density_i->assign(uint(1)), 1);

        Chan_de_p->get();	// Add	[0]Lse_density_energy_i [1]Chan_pressure 
        Chan_de_p->value = Chan_de_p->assign(uint(0)) + Chan_de_p->assign(uint(1));

        //Chan_GAMMA_pressure_density_i->get();	// Mul	[0]Chan_pressure_density_i 
        //Chan_GAMMA_pressure_density_i->value = Chan_GAMMA_pressure_density_i->assign(uint(0)) * 1.4;

        Chan_flux_i_density_energy_x->get();	// Mul	[0]Chan_velocity_x [1]Chan_de_p 
        Chan_flux_i_density_energy_x->value = Chan_flux_i_density_energy_x->assign(uint(0)) * Chan_flux_i_density_energy_x->assign(uint(1));

        Chan_flux_i_density_energy_y->get();	// Mul	[0]Chan_velocity_y [1]Chan_de_p 
        Chan_flux_i_density_energy_y->value = Chan_flux_i_density_energy_y->assign(uint(0)) * Chan_flux_i_density_energy_y->assign(uint(1));

        Chan_flux_i_density_energy_z->get();	// Mul	[0]Chan_velocity_z [1]Chan_de_p 
        Chan_flux_i_density_energy_z->value = Chan_flux_i_density_energy_z->assign(uint(0)) * Chan_flux_i_density_energy_z->assign(uint(1));

        //Chan_speed_of_sound->get();	// Sqrt	[0]Chan_GAMMA_pressure_density_i 
        //Chan_speed_of_sound->value = std::sqrt(Chan_speed_of_sound->assign(uint(0)));

        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            if (splitNum == 40 /*|| splitNum == 7*/)
            {
                // Patch
                if (graphScheduler->subgraphTimeout > 50)
                {
                    graphScheduler->switchGraphManually();
                }
                else
                {
                    graphScheduler->graphUpdate();
                }
            }
            else
            {
                graphScheduler->graphUpdate();
            }
        }

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** Profiler update
        profiler->updateBufferMaxDataNum();
        profiler->updateChanUtilization(graphScheduler->currSubgraphId);

        /*end->get();*/
        Chan_end->get();	// Nop

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (527104 > iter && iter > 525104 /*iter >= 0*/)
        {
            debug->printSimInfo(simChans, simLcs);
            debug->printGraphScheduler(graphScheduler);

            // Print MemorySystem
            //debug->memSysPrint(memSys);
        }

        if (!Chan_end->channel.empty())
        {
            debug->debug_mode = Debug_mode::Print_detail;
            std::cout << std::endl;
            std::cout << "Arch: " << xstr(ARCH) << std::endl;
            std::cout << "*******************************" << std::endl;
            std::cout << "Execution finished succussfully" << std::endl;
            std::cout << "*******************************" << std::endl;
            std::cout << "Total Cycle: " << clk << std::endl;
            std::cout << "Execution Iter: " << iter << std::endl;

            debug->getFile() << std::endl;
            debug->getFile() << "*******************************" << std::endl;
            debug->getFile() << "Execution finished succussfully" << std::endl;
            debug->getFile() << "*******************************" << std::endl;
            debug->getFile() << "Total Cycle: " << clk << std::endl;
            debug->getFile() << "Execution Iter: " << iter << std::endl;

            break;
        }

        ++iter;
    }

    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Profiling" << std::endl;
    debug->getFile() << "*******************************" << std::endl;

    // Print channel utilization
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Channel profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printChanProfiling(graphScheduler);

    //*** Print Lse access 
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Lse profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printLseProfiling();

    //*** Print cache 
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Cache miss rate: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printCacheMissRate();

    //*** Print power 
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Power profiling " << std::endl;
    debug->getFile() << std::endl;
    profiler->printPowerProfiling();

    //*** TIA profiling
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "TIA profiling " << std::endl;
    debug->getFile() << std::endl;
    if (splitNum == 1)
    {
        profiler->tiaProfiling();
    }

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;


    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
    delete profiler;
    delete graphScheduler;
}