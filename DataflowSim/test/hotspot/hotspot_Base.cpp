#include "./hotspot.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/util/util.hpp"

using namespace DFSimTest;

void HotSpot_Test::hotSpot_Base(Debug* debug)
{
    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    //*** Declare Profiler
    Profiler* profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 50);

    //*** Define subgraph scheduler
    GraphScheduler* graphScheduler = new GraphScheduler();

    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(HotSpot_Test::dfg);
    chanGraph.addSpecialModeChan();

    int splitNum = 7;
    //chanGraph.subgraphPartition(splitNum, debug);
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    HotSpot_Test::graphPartition(chanGraph, splitNum);

    // User defined patch (User defined subgraphId)
    //chanGraph.getNode("Chan_branch_merge")->subgraphId = chanGraph.getNode("Chan_real_odd_update")->subgraphId;

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
    const auto& Lc_r = registry->getLc("Lc_r");
    const auto& Chan_r_lc = registry->getChan("Chan_r_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_c = registry->getLc("Lc_c");
    const auto& Chan_r_lc_scatter_loop_c = registry->getChan("Chan_r_lc_scatter_loop_c");
    const auto& Chan_cond1 = registry->getChan("Chan_cond1");
    const auto& Chan_temp_rc_addr = registry->getChan("Chan_temp_rc_addr");
    const auto& Lse_temp_rc = registry->getChan("Lse_temp_rc");
    const auto& Chan_delta_cond1 = registry->getChan("Chan_delta_cond1");
    const auto& Chan_cond2 = registry->getChan("Chan_cond2");
    const auto& Chan_cond3 = registry->getChan("Chan_cond3");
    const auto& Chan_temp_c_cond2_addr = registry->getChan("Chan_temp_c_cond2_addr");
    const auto& Lse_temp_c_col_cond2 = registry->getChan("Lse_temp_c_col_cond2");
    const auto& Lse_temp_c_cond2 = registry->getChan("Lse_temp_c_cond2");
    const auto& Lse_temp_c_1_cond2 = registry->getChan("Lse_temp_c_1_cond2");
    const auto& Chan_power_c_cond2_addr = registry->getChan("Chan_power_c_cond2_addr");
    const auto& Lse_power_c_cond2 = registry->getChan("Lse_power_c_cond2");
    const auto& Chan_delta_cond2 = registry->getChan("Chan_delta_cond2");
    const auto& Chan_power_c_cond3_t_addr = registry->getChan("Chan_power_c_cond3_t_addr");
    const auto& Chan_temp_c_cond3_t_addr = registry->getChan("Chan_temp_c_cond3_t_addr");
    const auto& Lse_power_c_cond3_t = registry->getChan("Lse_power_c_cond3_t");
    const auto& Lse_temp_c_1_cond3_t = registry->getChan("Lse_temp_c_1_cond3_t");
    const auto& Lse_temp_c_cond3_t = registry->getChan("Lse_temp_c_cond3_t");
    const auto& Lse_temp_c_col_cond3_t = registry->getChan("Lse_temp_c_col_cond3_t");
    const auto& Chan_delta_cond3_t = registry->getChan("Chan_delta_cond3_t");
    const auto& Chan_power_c_cond3_f_addr = registry->getChan("Chan_power_c_cond3_f_addr");
    const auto& Chan_temp_c_cond3_f_addr = registry->getChan("Chan_temp_c_cond3_f_addr");
    const auto& Lse_power_c_cond3_f = registry->getChan("Lse_power_c_cond3_f");
    const auto& Lse_temp_c_1_cond3_f = registry->getChan("Lse_temp_c_1_cond3_f");
    const auto& Lse_temp_c_cond3_f = registry->getChan("Lse_temp_c_cond3_f");
    const auto& Lse_temp_c_col_cond3_f = registry->getChan("Lse_temp_c_col_cond3_f");
    const auto& Chan_delta_cond3_f = registry->getChan("Chan_delta_cond3_f");
    const auto& Chan_delta_merge_cond3 = registry->getChan("Chan_delta_merge_cond3");
    const auto& Chan_delta_merge_cond2 = registry->getChan("Chan_delta_merge_cond2");
    const auto& Chan_delta_merge_cond1 = registry->getChan("Chan_delta_merge_cond1");
    const auto& Chan_result = registry->getChan("Chan_result");
    //***********************************************************************

    // User-defined (Chan size)
    Chan_cond3->size = 200;

    Chan_power_c_cond3_t_addr->size = 200;
    Chan_temp_c_cond3_t_addr->size = 200;
    Chan_power_c_cond3_f_addr->size = 200;
    Chan_temp_c_cond3_f_addr->size = 200;

    Chan_delta_merge_cond1->size = 200;
    Chan_delta_merge_cond2->size = 200;
    Chan_delta_merge_cond3->size = 200;

    Chan_result->size = 200;

    // User defined (Mem req bypass -> noLatencyMode)
    registry->getLse("Lse_temp_rc")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_col_cond2")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_cond2")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_1_cond2")->noLatencyMode = 1;
    registry->getLse("Lse_power_c_cond2")->noLatencyMode = 1;

    registry->getLse("Lse_power_c_cond3_t")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_1_cond3_t")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_cond3_t")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_col_cond3_t")->noLatencyMode = 1;

    registry->getLse("Lse_power_c_cond3_f")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_1_cond3_f")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_cond3_f")->noLatencyMode = 1;
    registry->getLse("Lse_temp_c_col_cond3_f")->noLatencyMode = 1;

    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //graphScheduler->graphSwitchO3 = false;
    profiler->init();
    watchdog.addCheckPointChan({ Lc_r->getEnd, Lc_c->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint iter = 0;

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

        // Lc: Lc_r
        Lc_r->var = Lc_r->mux->mux(Lc_r->var, 0, Lc_r->sel);
        Lc_r->mux->muxUpdate(Lc_r->sel);
        Lc_r->mux->outChan->value = Lc_r->var;
        Lc_r->loopVar->get();
        Lc_r->loopVar->value = Lc_r->loopVar->assign(Lc_r->mux->outChan);  // After get(), must update chan's value
        Lc_r->var = Lc_r->loopVar->value + 1;
        Lc_r->lcUpdate(Lc_r->var < block_size);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_r_lc->get();	// Nop	[0]Lc_r 
        Chan_r_lc->value = Chan_r_lc->assign(uint(0));

        Chan_r_lc_scatter_loop_c->get();	// Nop	[0]Chan_r_lc 
        Chan_r_lc_scatter_loop_c->value = Chan_r_lc_scatter_loop_c->assign(uint(0));

        // Lc: Lc_c
        Lc_c->var = Lc_c->mux->mux(Lc_c->var, 0, Lc_c->sel);
        Lc_c->mux->muxUpdate(Lc_c->sel);
        Lc_c->mux->outChan->value = Lc_c->var;
        Lc_c->loopVar->get();
        Lc_c->loopVar->value = Lc_c->loopVar->assign(Lc_c->mux->outChan);  // After get(), must update chan's value
        Lc_c->var = Lc_c->loopVar->value + 1;
        Lc_c->lcUpdate(Lc_c->var < block_size);

        Chan_cond1->get();	// And	[0]Chan_r_lc_scatter_loop_c [1]Lc_c 
        //Chan_cond1->value = (Chan_cond1->assign(uint(0)) == 0) && (Chan_cond1->assign(uint(1)) == 0);
        Chan_cond1->value = Util::uRandom(1, 10) > 3 ? 1 : 0;

        Chan_temp_rc_addr->get();	// Add	[0]Chan_r_lc_scatter_loop_c [1]Lc_c 
        Chan_temp_rc_addr->value = Chan_temp_rc_addr->assign(uint(0)) + Chan_temp_rc_addr->assign(uint(1));

        Chan_cond2->get();	// And	[0]Chan_cond1 [1]Chan_r_lc_scatter_loop_c [2]Lc_c 
        //Chan_cond2->value = (Chan_cond2->assign(uint(1)) == 0) && (Chan_cond2->assign(uint(2)) == 16 - 1);
        Chan_cond2->value = Util::uRandom(1, 10) > 7 ? 1 : 0;

        Chan_cond3->get();	// Nop	[0]Chan_cond2 [1]Chan_r_lc_scatter_loop_c 
        //Chan_cond3->value = Chan_cond3->assign(uint(1));
        Chan_cond3->value = Util::uRandom(1, 10) > 7 ? 1 : 0;

        Chan_power_c_cond2_addr->get();	// And	[0]Chan_cond2 [1]Lc_c 
        Chan_power_c_cond2_addr->value = Chan_power_c_cond2_addr->assign(uint(0)) + Chan_power_c_cond2_addr->assign(uint(1));

        Chan_temp_c_cond2_addr->get();	// And	[0]Chan_cond2 [1]Lc_c 
        Chan_temp_c_cond2_addr->value = Chan_temp_c_cond2_addr->assign(uint(0)) + Chan_temp_c_cond2_addr->assign(uint(1));

        Chan_power_c_cond3_t_addr->get();	// And	[0]Chan_cond3 [1]Lc_c 
        Chan_power_c_cond3_t_addr->value = Chan_power_c_cond3_t_addr->assign(uint(0)) + Chan_power_c_cond3_t_addr->assign(uint(1));

        Chan_temp_c_cond3_t_addr->get();	// And	[0]Chan_cond3 [1]Lc_c 
        Chan_temp_c_cond3_t_addr->value = Chan_temp_c_cond3_t_addr->assign(uint(0)) + Chan_temp_c_cond3_t_addr->assign(uint(1));

        Chan_power_c_cond3_f_addr->get();	// And	[0]Chan_cond3 [1]Lc_c 
        Chan_power_c_cond3_f_addr->value = Chan_power_c_cond3_f_addr->assign(uint(0)) + Chan_power_c_cond3_f_addr->assign(uint(1));
        
        //std::cout << Chan_power_c_cond3_f_addr->channelCond << "\t" << Chan_cond3->value << std::endl;
        /*if (Chan_power_c_cond3_f_addr->valid)
            std::cout << Chan_power_c_cond3_f_addr->chanBuffer[0].front().cond << "\t"<< Chan_power_c_cond3_f_addr->channel.front().cond << std::endl;*/

        Chan_temp_c_cond3_f_addr->get();	// And	[0]Chan_cond3 [1]Lc_c 
        Chan_temp_c_cond3_f_addr->value = Chan_temp_c_cond3_f_addr->assign(uint(0)) + Chan_temp_c_cond3_f_addr->assign(uint(1));

        Chan_delta_merge_cond1->get();	// selPartial	[0]Chan_cond1 [1]Chan_delta_cond1 [2]Chan_delta_merge_cond2 
        Chan_delta_merge_cond1->value = Chan_delta_merge_cond1->assign(uint(0)) ? Chan_delta_merge_cond1->assign(uint(1)) : Chan_delta_merge_cond1->assign(uint(2));

        Chan_delta_cond1->get();	// Mul	[0]Chan_cond1 
        Chan_delta_cond1->value = Chan_delta_cond1->assign(uint(0)) * CAP * RX * RY * RZ;

        Lse_temp_rc->get();	// Load	[0]Chan_temp_rc_addr 
        uint i = Lse_temp_rc->assign();
        Lse_temp_rc->value = temp[Lse_temp_rc->assign()];

        Chan_delta_merge_cond2->get();	// selPartial	[0]Chan_cond2 [1]Chan_delta_cond2 [2]Chan_delta_merge_cond3 
        Chan_delta_merge_cond2->value = Chan_delta_merge_cond2->assign(uint(0)) ? Chan_delta_merge_cond2->assign(uint(1)) : Chan_delta_merge_cond2->assign(uint(2));

        Chan_delta_cond2->get();	// Mul	[0]Chan_cond2 [1]Lse_power_c_cond2 [2]Lse_temp_c_1_cond2 [3]Lse_temp_c_cond2 [4]Lse_temp_c_col_cond2 
        Chan_delta_cond2->value = Chan_delta_cond2->assign(uint(1)) * Chan_delta_cond2->assign(uint(2)) * Chan_delta_cond2->assign(uint(3)) * Chan_delta_cond2->assign(uint(4));

        Chan_delta_merge_cond3->get();	// selPartial	[0]Chan_cond3 [1]Chan_delta_cond3_t [2]Chan_delta_cond3_f 
        Chan_delta_merge_cond3->value = Chan_delta_merge_cond3->assign(uint(0)) ? Chan_delta_merge_cond3->assign(uint(1)) : Chan_delta_merge_cond3->assign(uint(2));

        Chan_delta_cond3_t->get();	// Mul	[0]Chan_cond3 [1]Lse_power_c_cond3_t [2]Lse_temp_c_1_cond3_t [3]Lse_temp_c_cond3_t [4]Lse_temp_c_col_cond3_t 
        Chan_delta_cond3_t->value = Chan_delta_cond3_t->assign(uint(0)) * Chan_delta_cond3_t->assign(uint(1)) * Chan_delta_cond3_t->assign(uint(2)) * Chan_delta_cond3_t->assign(uint(3)) * Chan_delta_cond3_t->assign(uint(4));

        Chan_delta_cond3_f->get();	// Mul	[0]Chan_cond3 [1]Lse_power_c_cond3_f [2]Lse_temp_c_1_cond3_f [3]Lse_temp_c_cond3_f [4]Lse_temp_c_col_cond3_f 
        Chan_delta_cond3_f->value = Chan_delta_cond3_f->assign(uint(0)) * Chan_delta_cond3_f->assign(uint(1)) * Chan_delta_cond3_f->assign(uint(2)) * Chan_delta_cond3_f->assign(uint(3)) * Chan_delta_cond3_f->assign(uint(4));

        Lse_power_c_cond2->get();	// Load	[0]Chan_power_c_cond2_addr 
        Lse_power_c_cond2->value = power[Lse_power_c_cond2->assign()];

        Lse_temp_c_1_cond2->get();	// Load	[0]Chan_temp_c_cond2_addr 
        Lse_temp_c_1_cond2->value = temp[Lse_temp_c_1_cond2->assign()];

        Lse_temp_c_cond2->get();	// Load	[0]Chan_temp_c_cond2_addr 
        Lse_temp_c_cond2->value = temp[Lse_temp_c_cond2->assign()];

        Lse_temp_c_col_cond2->get();	// Load	[0]Chan_temp_c_cond2_addr 
        Lse_temp_c_col_cond2->value = temp[Lse_temp_c_col_cond2->assign()];

        Lse_power_c_cond3_t->get();	// Load	[0]Chan_power_c_cond3_t_addr 
        Lse_power_c_cond3_t->value = temp[Lse_power_c_cond3_t->assign()];

        Lse_temp_c_1_cond3_t->get();	// Load	[0]Chan_temp_c_cond3_t_addr 
        Lse_temp_c_1_cond3_t->value = temp[Lse_temp_c_1_cond3_t->assign()];

        Lse_temp_c_cond3_t->get();	// Load	[0]Chan_temp_c_cond3_t_addr 
        Lse_temp_c_cond3_t->value = temp[Lse_temp_c_cond3_t->assign()];

        Lse_temp_c_col_cond3_t->get();	// Load	[0]Chan_temp_c_cond3_t_addr 
        Lse_temp_c_col_cond3_t->value = temp[Lse_temp_c_col_cond3_t->assign()];

        Lse_power_c_cond3_f->get();	// Load	[0]Chan_power_c_cond3_f_addr 
        Lse_power_c_cond3_f->value = temp[Lse_power_c_cond3_f->assign()];

        Lse_temp_c_1_cond3_f->get();	// Load	[0]Chan_temp_c_cond3_f_addr 
        Lse_temp_c_1_cond3_f->value = temp[Lse_temp_c_1_cond3_f->assign()];

        Lse_temp_c_cond3_f->get();	// Load	[0]Chan_temp_c_cond3_f_addr 
        Lse_temp_c_cond3_f->value = temp[Lse_temp_c_cond3_f->assign()];

        Lse_temp_c_col_cond3_f->get();	// Load	[0]Chan_temp_c_cond3_f_addr 
        Lse_temp_c_col_cond3_f->value = temp[Lse_temp_c_col_cond3_f->assign()];

        Chan_result->get();	// Add	[0]Lse_temp_rc [1]Chan_delta_merge_cond1 
        Chan_result->value = Chan_result->assign(uint(0)) + Chan_result->assign(uint(1));

        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            //graphScheduler->graphUpdate();

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

        if (7000 > iter && iter > 5000 /*iter >= 0*/)
        {
            // Print channel
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
