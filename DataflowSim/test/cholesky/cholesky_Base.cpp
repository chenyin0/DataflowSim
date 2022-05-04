#include "./cholesky.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

void Cholesky_Test::cholesky_Base(Debug* debug)
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
    ChanGraph chanGraph(Cholesky_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint splitNum = 2;
    //chanGraph.subgraphPartition(splitNum, debug);
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Cholesky_Test::graphPartition(chanGraph, splitNum);

    chanGraph.addChanDGSF();

    chanGraph.addNodeDelay();
    chanGraph.setSpeedup(debug);  // Set speedup
    chanGraph.plotDot();

    // Generate benchmark data
    generateData();

    registry->genModule(chanGraph);
    registry->genConnect(chanGraph);
    registry->setChanSize();
    // Patch
    //registry->getChan("Chan_Aij_div")->size = 20;

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
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc1 = registry->getChan("Chan_i_lc1");
    const auto& Chan_i_lc2 = registry->getChan("Chan_i_lc2");
    const auto& Chan_i_base = registry->getChan("Chan_i_base");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Chan_i_base_k = registry->getChan("Chan_i_base_k");
    const auto& Chan_i_base_k1 = registry->getChan("Chan_i_base_k1");
    const auto& Chan_Aii_addr = registry->getChan("Chan_Aii_addr");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Chan_j_lc = registry->getChan("Chan_j_lc");
    const auto& Chan_j_base = registry->getChan("Chan_j_base");
    const auto& Chan_i_base_k_scatter_loop_j = registry->getChan("Chan_i_base_k_scatter_loop_j");
    const auto& Chan_j_base_shadow = registry->getChan("Chan_j_base_shadow");
    const auto& Chan_Ajj_addr = registry->getChan("Chan_Ajj_addr");
    const auto& Chan_Aij_addr = registry->getChan("Chan_Aij_addr");
    const auto& Chan_i_base_k_relay_loop_j = registry->getChan("Chan_i_base_k_relay_loop_j");
    const auto& Lse_Ajj = registry->getChan("Lse_Ajj");
    const auto& Lse_Aij = registry->getChan("Lse_Aij");
    const auto& Chan_Aij_shadow = registry->getChan("Chan_Aij_shadow");
    const auto& Lc_k = registry->getLc("Lc_k");
    const auto& Chan_j_base_shadow_scatter_loop_k = registry->getChan("Chan_j_base_shadow_scatter_loop_k");
    const auto& Chan_i_base_k_relay_loop_j_scatter_loop_k = registry->getChan("Chan_i_base_k_relay_loop_j_scatter_loop_k");
    const auto& Chan_Ajk_addr = registry->getChan("Chan_Ajk_addr");
    const auto& Chan_Aik_addr = registry->getChan("Chan_Aik_addr");
    const auto& Lse_Ajk = registry->getChan("Lse_Ajk");
    const auto& Chan_Aij_shadow_scatter_loop_k = registry->getChan("Chan_Aij_shadow_scatter_loop_k");
    const auto& Lse_Aik = registry->getChan("Lse_Aik");
    const auto& Chan_Aik_Ajk = registry->getChan("Chan_Aik_Ajk");
    const auto& Chan_Aij_update = registry->getChan("Chan_Aij_update");
    const auto& Chan_Aij_update_shadow = registry->getChan("Chan_Aij_update_shadow");
    const auto& Chan_Aij_div = registry->getChan("Chan_Aij_div");
    const auto& Lc_k1 = registry->getLc("Lc_k1");
    const auto& Chan_i_base_k1_scatter_loop_k1 = registry->getChan("Chan_i_base_k1_scatter_loop_k1");
    const auto& Chan_Aii_addr_scatter_loop_k1 = registry->getChan("Chan_Aii_addr_scatter_loop_k1");
    const auto& Chan_Aik1_addr = registry->getChan("Chan_Aik1_addr");
    const auto& Lse_Aii = registry->getChan("Lse_Aii");
    const auto& Lse_Aik1 = registry->getChan("Lse_Aik1");
    const auto& Chan_Aik1_Aik1 = registry->getChan("Chan_Aik1_Aik1");
    const auto& Chan_Aii_update = registry->getChan("Chan_Aii_update");
    const auto& Chan_Aii_sqrt = registry->getChan("Chan_Aii_sqrt");
    //***********************************************************************

    // User defined
    registry->getLse("Lse_Ajj")->noLatencyMode = 1;
    registry->getLse("Lse_Aij")->noLatencyMode = 1;
    registry->getLse("Lse_Ajk")->noLatencyMode = 1;
    //registry->getLse("Lse_Aik")->noLatencyMode = 1;
    registry->getLse("Lse_Aii")->noLatencyMode = 1;
    registry->getLse("Lse_Aik1")->noLatencyMode = 1;

    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //graphScheduler->graphSwitchO3 = false;
    profiler->init();
    watchdog.addCheckPointChan({ Lc_i->getEnd, Lc_j->getEnd, Lc_k->getEnd, Lc_k1->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint iter = 0;

    // Pre-defined
    Lc_k1->loopVar->enable = 0;

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

        // Lc: Lc_i
        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < matrix_size);

        Chan_begin->valid = 0;

        Chan_i_lc1->get();	// Nop	[0]Lc_i 
        Chan_i_lc1->value = Chan_i_lc1->assign(uint(0));

        Chan_i_lc2->get();	// Nop	[0]Lc_i 
        Chan_i_lc2->value = Chan_i_lc2->assign(uint(0));

        Chan_i_base->get();	// Mul	[0]Lc_i 
        Chan_i_base->value = Chan_i_base->assign(uint(0)) * matrix_size;

        // Lc: Lc_j
        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var < Lc_j->loopVar->upstream[1]->value);

        // Lc: Lc_k1
        Lc_k1->var = Lc_k1->mux->mux(Lc_k1->var, 0, Lc_k1->sel);
        Lc_k1->mux->muxUpdate(Lc_k1->sel);
        Lc_k1->mux->outChan->value = Lc_k1->var;
        Lc_k1->loopVar->get();
        Lc_k1->loopVar->value = Lc_k1->loopVar->assign(Lc_k1->mux->outChan);  // After get(), must update chan's value
        Lc_k1->var = Lc_k1->loopVar->value + 1;
        Lc_k1->lcUpdate(Lc_k1->var < Lc_k1->loopVar->upstream[1]->value);

        Chan_i_base_k->get();	// Nop	[0]Chan_i_base 
        Chan_i_base_k->value = Chan_i_base_k->assign(uint(0));

        Chan_i_base_k1->get();	// Nop	[0]Chan_i_base 
        Chan_i_base_k1->value = Chan_i_base_k1->assign(uint(0));

        Chan_Aii_addr->get();	// Add	[0]Chan_i_base [1]Lc_i 
        Chan_Aii_addr->value = Chan_Aii_addr->assign(uint(0)) + Chan_Aii_addr->assign(uint(1));

        Chan_j_lc->get();	// Nop	[0]Lc_j 
        Chan_j_lc->value = Chan_j_lc->assign(uint(0));

        Chan_j_base->get();	// Mul	[0]Lc_j 
        Chan_j_base->value = Chan_j_base->assign(uint(0)) * matrix_size;

        Chan_i_base_k_scatter_loop_j->get();	// Nop	[0]Chan_i_base_k 
        Chan_i_base_k_scatter_loop_j->value = Chan_i_base_k_scatter_loop_j->assign(uint(0));

        Chan_i_base_k1_scatter_loop_k1->get();	// Nop	[0]Chan_i_base_k1 
        Chan_i_base_k1_scatter_loop_k1->value = Chan_i_base_k1_scatter_loop_k1->assign(uint(0));

        Chan_Aii_addr_scatter_loop_k1->get();	// Nop	[0]Chan_Aii_addr 
        Chan_Aii_addr_scatter_loop_k1->value = Chan_Aii_addr_scatter_loop_k1->assign(uint(0));

        // Lc: Lc_k
        Lc_k->var = Lc_k->mux->mux(Lc_k->var, 0, Lc_k->sel);
        Lc_k->mux->muxUpdate(Lc_k->sel);
        Lc_k->mux->outChan->value = Lc_k->var;
        Lc_k->loopVar->get();
        Lc_k->loopVar->value = Lc_k->loopVar->assign(Lc_k->mux->outChan);  // After get(), must update chan's value
        Lc_k->var = Lc_k->loopVar->value + 1;
        Lc_k->lcUpdate(Lc_k->var < Lc_k->loopVar->upstream[1]->value);

        Chan_j_base_shadow->get();	// Nop	[0]Chan_j_base 
        Chan_j_base_shadow->value = Chan_j_base_shadow->assign(uint(0));

        Chan_Ajj_addr->get();	// Add	[0]Chan_j_base [1]Lc_j 
        Chan_Ajj_addr->value = Chan_Ajj_addr->assign(uint(0)) + Chan_Ajj_addr->assign(uint(1));

        Chan_Aij_addr->get();	// Add	[0]Chan_i_base_k_scatter_loop_j [1]Lc_j 
        Chan_Aij_addr->value = Chan_Aij_addr->assign(uint(0)) + Chan_Aij_addr->assign(uint(1));

        Chan_i_base_k_relay_loop_j->get();	// Nop	[0]Chan_i_base_k_scatter_loop_j 
        Chan_i_base_k_relay_loop_j->value = Chan_i_base_k_relay_loop_j->assign(uint(0));

        Chan_Aik1_addr->get();	// Add	[0]Chan_i_base_k1_scatter_loop_k1 [1]Lc_k1 
        Chan_Aik1_addr->value = Chan_Aik1_addr->assign(uint(0)) + Chan_Aik1_addr->assign(uint(1));

        Lse_Aii->get();	// Load	[0]Chan_Aii_addr_scatter_loop_k1 
        Lse_Aii->value = A[Lse_Aii->assign()];

        Chan_j_base_shadow_scatter_loop_k->get();	// Nop	[0]Chan_j_base_shadow 
        Chan_j_base_shadow_scatter_loop_k->value = Chan_j_base_shadow_scatter_loop_k->assign(uint(0));

        Lse_Ajj->get();	// Load	[0]Chan_Ajj_addr 
        Lse_Ajj->value = A[Lse_Ajj->assign()];

        Lse_Aij->get();	// Load	[0]Chan_Aij_addr 
        Lse_Aij->value = A[Lse_Aij->assign()];

        Chan_i_base_k_relay_loop_j_scatter_loop_k->get();	// Nop	[0]Chan_i_base_k_relay_loop_j 
        Chan_i_base_k_relay_loop_j_scatter_loop_k->value = Chan_i_base_k_relay_loop_j_scatter_loop_k->assign(uint(0));

        Lse_Aik1->get();	// Load	[0]Chan_Aik1_addr 
        Lse_Aik1->value = A[Lse_Aik1->assign()];

        Chan_Ajk_addr->get();	// Add	[0]Chan_j_base_shadow_scatter_loop_k [1]Lc_k 
        Chan_Ajk_addr->value = Chan_Ajk_addr->assign(uint(0)) + Chan_Ajk_addr->assign(uint(1));

        Chan_Aij_shadow->get();	// Nop	[0]Lse_Aij 
        Chan_Aij_shadow->value = Chan_Aij_shadow->assign(uint(0));

        Chan_Aik_addr->get();	// Add	[0]Lc_k [1]Chan_i_base_k_relay_loop_j_scatter_loop_k 
        Chan_Aik_addr->value = Chan_Aik_addr->assign(uint(0)) + Chan_Aik_addr->assign(uint(1));

        Chan_Aik1_Aik1->get();	// Mul	[0]Lse_Aik1 
        Chan_Aik1_Aik1->value = Chan_Aik1_Aik1->assign(uint(0)) * Chan_Aik1_Aik1->assign(uint(0));

        Lse_Ajk->get();	// Load	[0]Chan_Ajk_addr 
        Lse_Ajk->value = A[Lse_Ajk->assign()];

        Chan_Aij_shadow_scatter_loop_k->get();	// Nop	[0]Chan_Aij_shadow 
        Chan_Aij_shadow_scatter_loop_k->value = Chan_Aij_shadow_scatter_loop_k->assign(uint(0));

        Lse_Aik->get();	// Load	[0]Chan_Aik_addr 
        Lse_Aik->value = A[Lse_Aik->assign()];

        Chan_Aii_update->get();	// Sub	[0]Lse_Aii [1]Chan_Aik1_Aik1 
        Chan_Aii_update->value = Chan_Aii_update->assign(uint(0)) - Chan_Aii_update->assign(uint(1));

        Chan_Aik_Ajk->get();	// Mul	[0]Lse_Aik [1]Lse_Ajk 
        Chan_Aik_Ajk->value = Chan_Aik_Ajk->assign(uint(0)) * Chan_Aik_Ajk->assign(uint(1));

        Chan_Aii_sqrt->get();	// Sqrt	[0]Chan_Aii_update 
        Chan_Aii_sqrt->value = sqrt(Chan_Aii_sqrt->assign(uint(0)));

        Chan_Aij_update->get();	// Sub	[0]Chan_Aij_shadow_scatter_loop_k [1]Chan_Aik_Ajk 
        Chan_Aij_update->value = Chan_Aij_update->assign(uint(0)) - Chan_Aij_update->assign(uint(1));

        Chan_Aij_update_shadow->get();	// Nop	[0]Chan_Aij_update 
        Chan_Aij_update_shadow->value = Chan_Aij_update_shadow->assign(uint(0));

        Chan_Aij_div->get();	// Div	[0]Chan_Aij_update_shadow [1]Lse_Ajj 
        Chan_Aij_div->value = Chan_Aij_div->assign(uint(0)) / max(Chan_Aij_div->assign(uint(1)), 1);

        if (Chan_Aij_div->valid && Chan_Aij_div->channel.front().last)
        {
            /* Lc_k1->loopVar->enable = 1;
             Lc_j->loopVar->enable = 0;*/

            if (splitNum >= 2)
            {
                graphScheduler->switchGraphManually();
            }
        }

        if (Lc_k1->loopVar->valid && Lc_k1->loopVar->channel.front().last)
        {
            Lc_k1->loopVar->enable = 0;
            Lc_j->loopVar->enable = 1;
        }

        if (Lc_j->loopVar->valid && Lc_j->loopVar->channel.front().last)
        {
            Lc_j->loopVar->enable = 0;
            Lc_k1->loopVar->enable = 1;
        }

        if (Chan_Aii_update->valid && Chan_Aii_update->channel.front().last)
        {
            /*Lc_j->loopVar->enable = 1;
            Lc_k1->loopVar->enable = 0;*/

            if (splitNum >= 2)
            {
                graphScheduler->switchGraphManually();
            }
        }

        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            //graphScheduler->graphUpdate();

            // Patch
            if (splitNum >= 2 && graphScheduler->subgraphTimeout > 30)
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

        // Patch
        //if (Lc_k1->loopVar->getTheLastData[1] && Lc_k2->loopVar->getTheLastData[1])
        //{
        //    Chan_end->channel.push_back(Data());	// Nop
        //}

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (55040 > iter && iter > 53040 /*iter >= 0*/)
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