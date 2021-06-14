#include "./Gemm.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"

using namespace DFSimTest;

void GemmTest::gemm_Base_auto_sim(Debug* debug)
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
    ChanGraph chanGraph(GemmTest::dfg);
    chanGraph.addSpecialModeChan();

    uint splitNum = 6;
    chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);

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

    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_jj = registry->getLc("Lc_jj");
    const auto& Chan_jj_lc = registry->getChan("Chan_jj_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_kk = registry->getLc("Lc_kk");
    const auto& Chan_kk_lc = registry->getChan("Chan_kk_lc");
    const auto& Chan_jj_lc_scatter_loop_kk = registry->getChan("Chan_jj_lc_scatter_loop_kk");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_jj_lc_relay_loop_kk = registry->getChan("Chan_jj_lc_relay_loop_kk");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_i_row = registry->getChan("Chan_i_row");
    const auto& Chan_kk_lc_scatter_loop_i = registry->getChan("Chan_kk_lc_scatter_loop_i");
    const auto& Chan_jj_lc_relay_loop_kk_scatter_loop_i = registry->getChan("Chan_jj_lc_relay_loop_kk_scatter_loop_i");
    const auto& Lc_k = registry->getLc("Lc_k");
    const auto& Chan_kk_lc_relay_loop_i = registry->getChan("Chan_kk_lc_relay_loop_i");
    const auto& Chan_jj_lc_relay_loop_i = registry->getChan("Chan_jj_lc_relay_loop_i");
    const auto& Chan_k_lc = registry->getChan("Chan_k_lc");
    const auto& Chan_i_row_scatter_loop_k = registry->getChan("Chan_i_row_scatter_loop_k");
    const auto& Chan_kk_lc_relay_loop_i_scatter_loop_k = registry->getChan("Chan_kk_lc_relay_loop_i_scatter_loop_k");
    const auto& Chan_jj_lc_relay_loop_i_scatter_loop_k = registry->getChan("Chan_jj_lc_relay_loop_i_scatter_loop_k");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Chan_i_row_relay_loop_k = registry->getChan("Chan_i_row_relay_loop_k");
    const auto& Chan_k_kk = registry->getChan("Chan_k_kk");
    const auto& Chan_jj_lc_relay_loop_k = registry->getChan("Chan_jj_lc_relay_loop_k");
    const auto& Chan_i_row_relay_loop_k_scatter_loop_j = registry->getChan("Chan_i_row_relay_loop_k_scatter_loop_j");
    const auto& Chan_k_row = registry->getChan("Chan_k_row");
    const auto& Chan_m1_addr = registry->getChan("Chan_m1_addr");
    const auto& Chan_jj_lc_relay_loop_k_scatter_loop_j = registry->getChan("Chan_jj_lc_relay_loop_k_scatter_loop_j");
    const auto& Chan_k_row_scatter_loop_j = registry->getChan("Chan_k_row_scatter_loop_j");
    const auto& Lse_temp_x = registry->getChan("Lse_temp_x");
    const auto& Chan_j_jj = registry->getChan("Chan_j_jj");
    const auto& Chan_m1_data = registry->getChan("Chan_m1_data");
    const auto& Chan_m2_addr = registry->getChan("Chan_m2_addr");
    const auto& Chan_prod_addr = registry->getChan("Chan_prod_addr");
    const auto& Chan_m1_data_scatter_loop_j = registry->getChan("Chan_m1_data_scatter_loop_j");
    const auto& Lse_m2_data = registry->getChan("Lse_m2_data");
    const auto& Lse_prod_data = registry->getChan("Lse_prod_data");
    const auto& Chan_mul = registry->getChan("Chan_mul");
    const auto& Chan_prod_data_update = registry->getChan("Chan_prod_data_update");
    const auto& Lse_prod_data_update_st = registry->getChan("Lse_prod_data_update_st");

    // User defined
    //registry->getLse("Lse_temp_x")->noLatencyMode = 1;
    //registry->getLse("Lse_m2_data")->noLatencyMode = 1;
    //registry->getLse("Lse_prod_data")->noLatencyMode = 1;
    registry->getLse("Lse_prod_data_update_st")->noLatencyMode = 1;

    // Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //registry->pathBalance();
    profiler->init();
    watchdog.addCheckPointChan({ Lc_jj->getEnd, Lc_kk->getEnd, Lc_i->getEnd, Lc_j->getEnd });

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

        Lc_jj->var = Lc_jj->mux->mux(Lc_jj->var, 0, Lc_jj->sel);
        Lc_jj->mux->muxUpdate(Lc_jj->sel);
        Lc_jj->mux->outChan->value = Lc_jj->var;
        Lc_jj->loopVar->get();
        Lc_jj->loopVar->value = Lc_jj->loopVar->assign(Lc_jj->mux->outChan);  // After get(), must update chan's value
        Lc_jj->var = Lc_jj->loopVar->value + block_size;
        Lc_jj->lcUpdate(Lc_jj->var < matrix_height);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_jj_lc->get();	// Nop	[0]Lc_jj 
        Chan_jj_lc->value = Chan_jj_lc->assign(uint(0));

        Lc_kk->var = Lc_kk->mux->mux(Lc_kk->var, 0, Lc_kk->sel);
        Lc_kk->mux->muxUpdate(Lc_kk->sel);
        Lc_kk->mux->outChan->value = Lc_kk->var;
        Lc_kk->loopVar->get();
        Lc_kk->loopVar->value = Lc_kk->loopVar->assign(Lc_kk->mux->outChan);  // After get(), must update chan's value
        Lc_kk->var = Lc_kk->loopVar->value + block_size;
        Lc_kk->lcUpdate(Lc_kk->var < matrix_height);

        Chan_kk_lc->get();	// Nop	[0]Lc_kk 
        Chan_kk_lc->value = Chan_kk_lc->assign(uint(0));

        Chan_jj_lc_scatter_loop_kk->get();	// Nop	[0]Chan_jj_lc 
        Chan_jj_lc_scatter_loop_kk->value = Chan_jj_lc_scatter_loop_kk->assign(uint(0));

        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < matrix_height);

        Chan_jj_lc_relay_loop_kk->get();	// Nop	[0]Chan_jj_lc 
        Chan_jj_lc_relay_loop_kk->value = Chan_jj_lc_relay_loop_kk->assign(uint(0));

        Chan_i_lc->get();	// Nop	[0]Lc_i 
        Chan_i_lc->value = Chan_i_lc->assign(uint(0));

        Chan_i_row->get();	// Mul	[0]Lc_i 
        Chan_i_row->value = Chan_i_row->assign(uint(0)) * matrix_width;

        Chan_kk_lc_scatter_loop_i->get();	// Nop	[0]Chan_kk_lc 
        Chan_kk_lc_scatter_loop_i->value = Chan_kk_lc_scatter_loop_i->assign(uint(0));

        Chan_jj_lc_relay_loop_kk_scatter_loop_i->get();	// Nop	[0]Chan_jj_lc_relay_loop_kk 
        Chan_jj_lc_relay_loop_kk_scatter_loop_i->value = Chan_jj_lc_relay_loop_kk_scatter_loop_i->assign(uint(0));

        Lc_k->var = Lc_k->mux->mux(Lc_k->var, 0, Lc_k->sel);
        Lc_k->mux->muxUpdate(Lc_k->sel);
        Lc_k->mux->outChan->value = Lc_k->var;
        Lc_k->loopVar->get();
        Lc_k->loopVar->value = Lc_k->loopVar->assign(Lc_k->mux->outChan);  // After get(), must update chan's value
        Lc_k->var = Lc_k->loopVar->value + 1;
        Lc_k->lcUpdate(Lc_k->var < block_size);

        Chan_jj_lc_relay_loop_i->get();	// Nop	[0]Chan_jj_lc_relay_loop_kk 
        Chan_jj_lc_relay_loop_i->value = Chan_jj_lc_relay_loop_i->assign(uint(0));

        Chan_kk_lc_relay_loop_i->get();	// Nop	[0]Chan_kk_lc 
        Chan_kk_lc_relay_loop_i->value = Chan_kk_lc_relay_loop_i->assign(uint(0));

        Chan_k_lc->get();	// Nop	[0]Lc_k_DGSF 
        Chan_k_lc->value = Chan_k_lc->assign(uint(0));

        Chan_i_row_scatter_loop_k->get();	// Nop	[0]Chan_i_row_DGSF 
        Chan_i_row_scatter_loop_k->value = Chan_i_row_scatter_loop_k->assign(uint(0));

        Chan_kk_lc_relay_loop_i_scatter_loop_k->get();	// Nop	[0]Chan_kk_lc_relay_loop_i_DGSF 
        Chan_kk_lc_relay_loop_i_scatter_loop_k->value = Chan_kk_lc_relay_loop_i_scatter_loop_k->assign(uint(0));

        Chan_jj_lc_relay_loop_i_scatter_loop_k->get();	// Nop	[0]Chan_jj_lc_relay_loop_i_DGSF 
        Chan_jj_lc_relay_loop_i_scatter_loop_k->value = Chan_jj_lc_relay_loop_i_scatter_loop_k->assign(uint(0));

        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var < block_size);

        Chan_k_kk->get();	// Add	[0]Lc_k_DGSF [1]Chan_kk_lc_relay_loop_i 
        Chan_k_kk->value = Chan_k_kk->assign(uint(0)) + Chan_k_kk->assign(uint(1));

        Chan_jj_lc_relay_loop_k->get();	// Nop	[0]Chan_jj_lc_relay_loop_i 
        Chan_jj_lc_relay_loop_k->value = Chan_jj_lc_relay_loop_k->assign(uint(0));

        Chan_i_row_relay_loop_k->get();	// Nop	[0]Chan_i_row 
        Chan_i_row_relay_loop_k->value = Chan_i_row_relay_loop_k->assign(uint(0));

        Chan_i_row_relay_loop_k_scatter_loop_j->get();	// Nop	[0]Chan_i_row_relay_loop_k 
        Chan_i_row_relay_loop_k_scatter_loop_j->value = Chan_i_row_relay_loop_k_scatter_loop_j->assign(uint(0));

        Chan_k_row->get();	// Mul	[0]Chan_k_kk 
        Chan_k_row->value = Chan_k_row->assign(uint(0)) * matrix_width;

        Chan_m1_addr->get();	// Add	[0]Chan_i_row [1]Chan_k_kk 
        Chan_m1_addr->value = Chan_m1_addr->assign(uint(0)) + Chan_m1_addr->assign(uint(1));

        Chan_jj_lc_relay_loop_k_scatter_loop_j->get();	// Nop	[0]Chan_jj_lc_relay_loop_k 
        Chan_jj_lc_relay_loop_k_scatter_loop_j->value = Chan_jj_lc_relay_loop_k_scatter_loop_j->assign(uint(0));

        Lse_temp_x->get();	// Load	[0]Chan_m1_addr 
        Lse_temp_x->value = m1_[Lse_temp_x->assign()];

        Chan_k_row_scatter_loop_j->get();	// Nop	[0]Chan_k_row_DGSF 
        Chan_k_row_scatter_loop_j->value = Chan_k_row_scatter_loop_j->assign(uint(0));

        Chan_j_jj->get();	// Add	[0]Lc_j [1]Chan_jj_lc_relay_loop_k_DGSF 
        Chan_j_jj->value = Chan_j_jj->assign(uint(0)) + Chan_j_jj->assign(uint(1));

        Chan_m1_data->get();	// Nop	[0]Lse_temp_x 
        Chan_m1_data->value = Chan_m1_data->assign(uint(0));

        Chan_m2_addr->get();	// Add	[0]Chan_k_row_DGSF [1]Chan_j_jj 
        Chan_m2_addr->value = Chan_m2_addr->assign(uint(0)) + Chan_m2_addr->assign(uint(1));

        Chan_prod_addr->get();	// Add	[0]Chan_j_jj [1]Chan_i_row_relay_loop_k_DGSF 
        Chan_prod_addr->value = Chan_prod_addr->assign(uint(0)) + Chan_prod_addr->assign(uint(1));

        Chan_m1_data_scatter_loop_j->get();	// Nop	[0]Chan_m1_data_DGSF 
        Chan_m1_data_scatter_loop_j->value = Chan_m1_data_scatter_loop_j->assign(uint(0));

        Lse_m2_data->get();	// Load	[0]Chan_m2_addr 
        Lse_m2_data->value = m2_[Lse_m2_data->assign()];

        Lse_prod_data->get();	// Load	[0]Chan_prod_addr 
        Lse_prod_data->value = prod_[Lse_prod_data->assign()];

        Chan_mul->get();	// Mul	[0]Chan_m1_data_DGSF [1]Lse_m2_data 
        Chan_mul->value = Chan_mul->assign(uint(0)) * Chan_mul->assign(uint(1));

        Chan_prod_data_update->get();	// Add	[0]Lse_prod_data [1]Chan_mul 
        Chan_prod_data_update->value = Chan_prod_data_update->assign(uint(0)) + Chan_prod_data_update->assign(uint(1));

        Lse_prod_data_update_st->get();	// Store	[0]Chan_prod_addr [1]Chan_prod_data_update 


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

        if (234810 > iter && iter > 0 /*iter >= 0*/)
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
    profiler->printChanProfiling();

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