#include "./sort.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

void Sort_Test::sort_Base(Debug* debug)
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
    ChanGraph chanGraph(Sort_Test::dfg);
    chanGraph.addSpecialModeChan();

    int splitNum = 3;
    //chanGraph.subgraphPartition(splitNum, debug);
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Sort_Test::graphPartition(chanGraph, splitNum);

    // User defined patch (User defined subgraphId)
    //chanGraph.getNode("Chan_branch_merge")->subgraphId = chanGraph.getNode("Chan_real_odd_update")->subgraphId;

    //chanGraph.printSubgraphPartition(splitNum, debug);
    chanGraph.addChanDGSF();

    chanGraph.addNodeDelay();
    chanGraph.setSpeedup(debug);  // Set speedup
    chanGraph.plotDot();


    // Generate benchmark data
    //genInputData();  // Only execute once
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
    const auto& Lc_m = registry->getLc("Lc_m");
    const auto& Chan_m_lc = registry->getChan("Chan_m_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_k1 = registry->getChan("Chan_i_k1");
    const auto& Chan_i_k2 = registry->getChan("Chan_i_k2");
    const auto& Chan_from = registry->getChan("Chan_from");
    const auto& Chan_m_lc_scatter_loop_i = registry->getChan("Chan_m_lc_scatter_loop_i");
    const auto& Chan_m_m = registry->getChan("Chan_m_m");
    const auto& Chan_i_m_m = registry->getChan("Chan_i_m_m");
    const auto& Chan_to = registry->getChan("Chan_to");
    const auto& Chan_to_k1 = registry->getChan("Chan_to_k1");
    const auto& Chan_to_k2 = registry->getChan("Chan_to_k2");
    const auto& Chan_to_cmp = registry->getChan("Chan_to_cmp");
    const auto& Chan_to_cmp_k1 = registry->getChan("Chan_to_cmp_k1");
    const auto& Chan_to_cmp_k2 = registry->getChan("Chan_to_cmp_k2");
    const auto& Lc_k1 = registry->getLc("Lc_k1");
    const auto& Lc_k2 = registry->getLc("Lc_k2");
    const auto& Chan_i_k1_scatter_loop_k1 = registry->getChan("Chan_i_k1_scatter_loop_k1");
    const auto& Chan_to_k1_scatter_loop_k1 = registry->getChan("Chan_to_k1_scatter_loop_k1");
    const auto& Chan_i_k2_scatter_loop_k2 = registry->getChan("Chan_i_k2_scatter_loop_k2");
    const auto& Chan_to_k2_scatter_loop_k2 = registry->getChan("Chan_to_k2_scatter_loop_k2");
    const auto& Lse_tmp_i1 = registry->getChan("Lse_tmp_i1");
    const auto& Lse_tmp_j1 = registry->getChan("Lse_tmp_j1");
    const auto& Lse_tmp_i2 = registry->getChan("Lse_tmp_i2");
    const auto& Lse_tmp_j2 = registry->getChan("Lse_tmp_j2");
    const auto& Chan_tmp1_cmp = registry->getChan("Chan_tmp1_cmp");
    const auto& Chan_tmp2_cmp = registry->getChan("Chan_tmp2_cmp");
    const auto& Chan_k1_cond = registry->getChan("Chan_k1_cond");
    const auto& Chan_to1_update = registry->getChan("Chan_to1_update");
    const auto& Chan_i1_update = registry->getChan("Chan_i1_update");
    const auto& Chan_k2_cond = registry->getChan("Chan_k2_cond");
    const auto& Chan_to2_update = registry->getChan("Chan_to2_update");
    const auto& Chan_i2_update = registry->getChan("Chan_i2_update");
    const auto& Lse_a_update_k1_true = registry->getChan("Lse_a_update_k1_true");
    const auto& Lse_a_update_k1_false = registry->getChan("Lse_a_update_k1_false");
    const auto& Chan_br_merge_tmp1 = registry->getChan("Chan_br_merge_tmp1");
    const auto& Lse_a_update_k2_true = registry->getChan("Lse_a_update_k2_true");
    const auto& Lse_a_update_k2_false = registry->getChan("Lse_a_update_k2_false");
    const auto& Chan_br_merge_tmp2 = registry->getChan("Chan_br_merge_tmp2");
    const auto& Chan_br_merge_tmp1_shadow = registry->getChan("Chan_br_merge_tmp1_shadow");
    const auto& Chan_br_merge_tmp2_shadow = registry->getChan("Chan_br_merge_tmp2_shadow");
    const auto& Chan_br_merge_to = registry->getChan("Chan_br_merge_to");
    //***********************************************************************

    // User-defined (Chan size)
    Chan_br_merge_tmp1->size = 2000;
    Chan_br_merge_tmp2->size = 2000;
    Chan_br_merge_to->size = 2000;

    // User defined (Mem req bypass -> noLatencyMode)
    registry->getLse("Lse_tmp_j2")->noLatencyMode = 1;
    registry->getLse("Lse_tmp_i2")->noLatencyMode = 1;

    registry->getLse("Lse_a_update_k2_true")->noLatencyMode = 1;
    registry->getLse("Lse_a_update_k2_false")->noLatencyMode = 1;


    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //graphScheduler->graphSwitchO3 = false;
    profiler->init();
    watchdog.addCheckPointChan({ Lc_m->getEnd, Lc_i->getEnd, Lc_k1->getEnd, Lc_k2->getEnd });

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

        // Lc: Lc_m
        Lc_m->var = Lc_m->mux->mux(Lc_m->var, 1, Lc_m->sel);
        Lc_m->mux->muxUpdate(Lc_m->sel);
        Lc_m->mux->outChan->value = Lc_m->var;
        Lc_m->loopVar->get();
        Lc_m->loopVar->value = Lc_m->loopVar->assign(Lc_m->mux->outChan);  // After get(), must update chan's value
        Lc_m->var = Lc_m->loopVar->value + Lc_m->loopVar->value;
        Lc_m->lcUpdate(Lc_m->var < SIZE);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_m_lc->get();	// Nop	[0]Lc_m 
        Chan_m_lc->value = Chan_m_lc->assign(uint(0));

        // Lc: Lc_i
        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + Chan_m_lc->value + Chan_m_lc->value;
        Lc_i->lcUpdate(Lc_i->var < SIZE);

        Chan_i_k1->get();	// Nop	[0]Lc_i 
        Chan_i_k1->value = Chan_i_k1->assign(uint(0));

        Chan_i_k2->get();	// Nop	[0]Lc_i 
        Chan_i_k2->value = Chan_i_k2->assign(uint(0));

        Chan_m_lc_scatter_loop_i->get();	// Nop	[0]Chan_m_lc 
        Chan_m_lc_scatter_loop_i->value = Chan_m_lc_scatter_loop_i->assign(uint(0));

        Chan_m_m->get();	// Add	[0]Chan_m_lc_scatter_loop_i 
        Chan_m_m->value = Chan_m_m->assign(uint(0)) + Chan_m_m->assign(uint(0));

        Chan_i_m_m->get();	// Add	[0]Chan_i_m [1]Chan_m_lc_scatter_loop_i 
        Chan_i_m_m->value = Chan_i_m_m->assign(uint(0)) + Chan_i_m_m->assign(uint(1));

        Chan_to->get();	// Sub	[0]Chan_i_m_m 
        Chan_to->value = Chan_to->assign(uint(0)) - 1;

        //std::cout << std::endl;
        //std::cout << Chan_m_lc_scatter_loop_i->value << std::endl;
        //std::cout << Chan_m_m->value << std::endl;
        //std::cout << Lc_i->loopVar->value << std::endl;

        Chan_to_cmp->get();	// Cmp	[0]Chan_to 
        Chan_to_cmp->value = Chan_to_cmp->assign(uint(0)) < SIZE ? 1 : 0;

        Chan_to_k1->get();	// Nop	[0]Chan_to 
        Chan_to_k1->value = Chan_to_k1->assign(uint(0));

        Chan_to_k2->get();	// Nop	[0]Chan_to 
        Chan_to_k2->value = Chan_to_k2->assign(uint(0));

        Chan_to_cmp_k1->get();	// Nop	[0]Chan_to_cmp 
        Chan_to_cmp_k1->value = Chan_to_cmp_k1->assign(uint(0));

        Chan_to_cmp_k2->get();	// Nop	[0]Chan_to_cmp 
        Chan_to_cmp_k2->value = Chan_to_cmp_k2->assign(uint(0));

        // Lc: Lc_k1
        Lc_k1->var = Lc_k1->mux->mux(Lc_k1->var, Lc_k1->loopVar->upstream[1]->value, Lc_k1->sel);
        Lc_k1->mux->muxUpdate(Lc_k1->sel);
        Lc_k1->mux->outChan->value = Lc_k1->var;
        Lc_k1->loopVar->get();
        Lc_k1->loopVar->value = Lc_k1->loopVar->assign(Lc_k1->mux->outChan);  // After get(), must update chan's value
        Lc_k1->var = Lc_k1->loopVar->value + 1;
        Lc_k1->lcUpdate(Lc_k1->var < Chan_to_k1->value);

        // Lc: Lc_k2
        Lc_k2->var = Lc_k2->mux->mux(Lc_k2->var, Lc_k2->loopVar->upstream[1]->value, Lc_k2->sel);
        Lc_k2->mux->muxUpdate(Lc_k2->sel);
        Lc_k2->mux->outChan->value = Lc_k2->var;
        Lc_k2->loopVar->get();
        Lc_k2->loopVar->value = Lc_k2->loopVar->assign(Lc_k2->mux->outChan);  // After get(), must update chan's value
        Lc_k2->var = Lc_k2->loopVar->value + 1;
        Lc_k2->lcUpdate(Lc_k2->var < Chan_to_k2->value);

        Chan_i_k1_scatter_loop_k1->get();	// Nop	[0]Chan_i_k1 
        Chan_i_k1_scatter_loop_k1->value = Chan_i_k1_scatter_loop_k1->assign(uint(0));

        Chan_to_k1_scatter_loop_k1->get();	// Nop	[0]Chan_to_k1 
        Chan_to_k1_scatter_loop_k1->value = Chan_to_k1_scatter_loop_k1->assign(uint(0));

        Chan_i_k2_scatter_loop_k2->get();	// Nop	[0]Chan_i_k2 
        Chan_i_k2_scatter_loop_k2->value = Chan_i_k2_scatter_loop_k2->assign(uint(0));

        Chan_to_k2_scatter_loop_k2->get();	// Nop	[0]Chan_to_k2 
        Chan_to_k2_scatter_loop_k2->value = Chan_to_k2_scatter_loop_k2->assign(uint(0));

        Lse_tmp_i1->get();	// Load	[0]Chan_i_k1_scatter_loop_k1 
        Lse_tmp_i1->value = temp[Lse_tmp_i1->assign()];

        Lse_tmp_j1->get();	// Load	[0]Chan_to_k1_scatter_loop_k1 
        Lse_tmp_j1->value = temp[Lse_tmp_j1->assign()];

        Lse_tmp_i2->get();	// Load	[0]Chan_i_k2_scatter_loop_k2 
        Lse_tmp_i2->value = temp[Lse_tmp_i2->assign()];

        Lse_tmp_j2->get();	// Load	[0]Chan_to_k2_scatter_loop_k2 
        Lse_tmp_j2->value = temp[Lse_tmp_j2->assign()];

        Chan_tmp1_cmp->get();	// Cmp	[0]Lse_tmp_j1 [1]Lse_tmp_i1 
        Chan_tmp1_cmp->value = Chan_tmp1_cmp->assign(uint(0)) < Chan_tmp1_cmp->assign(uint(1)) ? 1 : 0;

        Chan_tmp2_cmp->get();	// Cmp	[0]Lse_tmp_j2 [1]Lse_tmp_i2 
        Chan_tmp2_cmp->value = Chan_tmp2_cmp->assign(uint(0)) < Chan_tmp2_cmp->assign(uint(1)) ? 1 : 0;

        Chan_k1_cond->get();	// Nop	[0]Chan_tmp1_cmp [1]Lc_k1 
        Chan_k1_cond->value = Chan_k1_cond->assign(uint(1));

        Chan_to1_update->get();	// Sub	[0]Chan_tmp1_cmp [1]Chan_to_k1_scatter_loop_k1 
        Chan_to1_update->value = Chan_to1_update->assign(uint(0)) - Chan_to1_update->assign(uint(1));

        Chan_i1_update->get();	// Add	[0]Chan_tmp1_cmp [1]Chan_i_k1_scatter_loop_k1 
        Chan_i1_update->value = Chan_i1_update->assign(uint(0)) + Chan_i1_update->assign(uint(1));

        Chan_k2_cond->get();	// Nop	[0]Chan_tmp2_cmp [1]Lc_k2 
        Chan_k2_cond->value = Chan_k2_cond->assign(uint(1));

        Chan_to2_update->get();	// Sub	[0]Chan_tmp2_cmp [1]Chan_to_k2_scatter_loop_k2 
        Chan_to2_update->value = Chan_to2_update->assign(uint(0)) - Chan_to2_update->assign(uint(1));

        Chan_i2_update->get();	// Add	[0]Chan_tmp2_cmp [1]Chan_i_k2_scatter_loop_k2 
        Chan_i2_update->value = Chan_i2_update->assign(uint(0)) + Chan_i2_update->assign(uint(1));

        Lse_a_update_k1_true->get();	// Store	[0]Chan_k1_cond [1]Lse_tmp_j1 

        Lse_a_update_k1_false->get();	// Store	[0]Chan_k1_cond [1]Lse_tmp_j1 

        Chan_br_merge_tmp1->get();	// selPartial	[0]Chan_tmp1_cmp [1]Chan_to1_update [2]Chan_i1_update 
        Chan_br_merge_tmp1->value = Chan_br_merge_tmp1->assign(uint(0)) ? Chan_br_merge_tmp1->assign(uint(1)) : Chan_br_merge_tmp1->assign(uint(2));

        Lse_a_update_k2_true->get();	// Store	[0]Chan_k2_cond [1]Lse_tmp_j2 

        Lse_a_update_k2_false->get();	// Store	[0]Chan_k2_cond [1]Lse_tmp_j2 

        Chan_br_merge_tmp2->get();	// selPartial	[0]Chan_tmp2_cmp [1]Chan_to2_update [2]Chan_i2_update 
        Chan_br_merge_tmp2->value = Chan_br_merge_tmp2->assign(uint(0)) ? Chan_br_merge_tmp2->assign(uint(1)) : Chan_br_merge_tmp2->assign(uint(2));

        Chan_br_merge_tmp1_shadow->get();	// Nop	[0]Chan_br_merge_tmp1 
        Chan_br_merge_tmp1_shadow->value = Chan_br_merge_tmp1_shadow->assign(uint(0));

        Chan_br_merge_tmp2_shadow->get();	// Nop	[0]Chan_br_merge_tmp2 
        Chan_br_merge_tmp2_shadow->value = Chan_br_merge_tmp2_shadow->assign(uint(0));

        Chan_br_merge_to->get();	// selPartial	[0]Chan_to_cmp [1]Chan_br_merge_tmp1_shadow [2]Chan_br_merge_tmp2_shadow 
        Chan_br_merge_to->value = Chan_br_merge_to->assign(uint(0)) ? Chan_br_merge_to->assign(uint(1)) : Chan_br_merge_to->assign(uint(2));


        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            graphScheduler->graphUpdate();

            //// Patch
            //if (watchdog.getFeedTimes() > 20 && graphScheduler->subgraphTimeout > 30)
            //{
            //    graphScheduler->switchGraphManually();
            //}
            //else
            //{
            //    graphScheduler->graphUpdate();
            //}
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

        if (58496 > iter && iter > 56496 /*iter >= 0*/)
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
