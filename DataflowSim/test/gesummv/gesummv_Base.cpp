#include "./gesummv.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"

using namespace DFSimTest;

void Gesummv_Test::gesummv_Base(Debug* debug)
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
    ChanGraph chanGraph(Gesummv_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint splitNum = 7;
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Gesummv_Test::graphPartition(chanGraph, splitNum);

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
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_i_PB_N = registry->getChan("Chan_i_PB_N");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Lse_x = registry->getChan("Lse_x");
    //const auto& Chan_i_lc_scatter_loop_j = registry->getChan("Chan_i_lc_scatter_loop_j");
    const auto& Chan_i_PB_N_scatter_loop_j = registry->getChan("Chan_i_PB_N_scatter_loop_j");
    //const auto& Lse_y = registry->getChan("Lse_y");
    const auto& Chan_addr = registry->getChan("Chan_addr");
    const auto& Lse_A = registry->getChan("Lse_A");
    const auto& Lse_B = registry->getChan("Lse_B");
    const auto& Chan_A_x = registry->getChan("Chan_A_x");
    const auto& Chan_B_x = registry->getChan("Chan_B_x");
    const auto& Chan_tmp = registry->getChan("Chan_tmp");
    const auto& Chan_y_update = registry->getChan("Chan_y_update");
    const auto& Chan_alpha_tmp = registry->getChan("Chan_alpha_tmp");
    const auto& Chan_beta_y = registry->getChan("Chan_beta_y");
    const auto& Chan_y_update_i = registry->getChan("Chan_y_update_i");

    //***********************************************************************

    // User defined
    //registry->getLse("Lse_m2_data")->noLatencyMode = 1;
    //registry->getLse("Lse_prod_data_update_st")->noLatencyMode = 1;

    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    profiler->init();
    watchdog.addCheckPointChan({ Lc_i->getEnd, Lc_j->getEnd });

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

        // Lc: Lc_i
        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < _PB_N);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_i_lc->get();	// Nop	[0]Lc_i 
        Chan_i_lc->value = Chan_i_lc->assign(uint(0));

        Chan_i_PB_N->get();	// Mul	[0]Lc_i 
        Chan_i_PB_N->value = Chan_i_PB_N->assign(uint(0)) * _PB_N;

        // Lc: Lc_j
        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var < _PB_N);

        Lse_x->get();	// Load	[0]Lc_j 
        Lse_x->value = x[Lse_x->assign()];

        //Chan_i_lc_scatter_loop_j->get();	// Nop	[0]Chan_i_lc 
        //Chan_i_lc_scatter_loop_j->value = Chan_i_lc_scatter_loop_j->assign(uint(0));

        Chan_i_PB_N_scatter_loop_j->get();	// Nop	[0]Chan_i_PB_N 
        Chan_i_PB_N_scatter_loop_j->value = Chan_i_PB_N_scatter_loop_j->assign(uint(0));

        //Lse_y->get();	// Load	[0]Chan_i_lc_scatter_loop_j 
        //Lse_y->value = y[Lse_y->assign()];

        Chan_addr->get();	// ADD	[0]Chan_i_PB_N_scatter_loop_j [1]Lc_j 
        Chan_addr->value = Chan_addr->assign(uint(0)) + Chan_addr->assign(uint(1));

        Lse_A->get();	// Load	[0]Chan_addr 
        Lse_A->value = A[Lse_A->assign()];

        Lse_B->get();	// Load	[0]Chan_addr 
        Lse_B->value = B[Lse_B->assign()];

        Chan_A_x->get();	// Mul	[0]Lse_A [1]Lse_x 
        Chan_A_x->value = Chan_A_x->assign(uint(0)) * Chan_A_x->assign(uint(1));

        Chan_B_x->get();	// Mul	[0]Lse_B [1]Lse_x 
        Chan_B_x->value = Chan_B_x->assign(uint(0)) * Chan_B_x->assign(uint(1));

        Chan_tmp->get();	// Add	[0]Chan_A_x 
        Chan_tmp->value = Chan_tmp->assign(uint(0)) + Chan_tmp->value;

        Chan_y_update->get();	// Add	[0]Chan_B_x 
        Chan_y_update->value = Chan_y_update->assign(uint(0)) + Chan_y_update->value;

        Chan_alpha_tmp->get();	// Mul	[0]Chan_tmp 
        Chan_alpha_tmp->value = Chan_alpha_tmp->assign(uint(0)) * alpha;

        Chan_beta_y->get();	// Mul	[0]Chan_y_update 
        Chan_beta_y->value = Chan_beta_y->assign(uint(0)) * beta;

        Chan_y_update_i->get();	// Add	[0]Chan_alpha_tmp [1]Chan_beta_y 
        Chan_y_update_i->value = Chan_y_update_i->assign(uint(0)) + Chan_y_update_i->assign(uint(1));


        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            graphScheduler->graphUpdate();
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

        if (13056 > iter && iter > 0 /*iter >= 0*/)
        {
            // Print channel
            debug->printSimInfo(simChans, simLcs);
            debug->printGraphScheduler(graphScheduler);

            // Print MemorySystem
            debug->memSysPrint(memSys);
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
