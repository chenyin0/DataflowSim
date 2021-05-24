#include "./fft.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

void FFT_Test::fft_Base(Debug* debug)
{
    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(FFT_Test::dfg);
    chanGraph.addSpecialModeChan();
    chanGraph.subgraphPartition(2, 10);
    chanGraph.addNodeDelay();
    chanGraph.plotDot();

    // Generate benchmark data
    generateData();

    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    //*** Declare Profiler
    Profiler* profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 50);

    registry->genModule(chanGraph);
    registry->genConnect(chanGraph);
    auto& regis = registry->getRegistryTable();  // For exposing registry in debugging
    //debug->printRegistry(registry);
    debug->printSimNodes(chanGraph);
    registry->genSimConfig(chanGraph);  // Only used for initializing the first time sim
    const auto& debugPrint = registry->genDebugPrint(chanGraph);
    auto simChans = get<0>(debugPrint);
    auto simLcs = get<1>(debugPrint);


    ////*** Simulate
    // Declare
    //***********************************************************************
    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_span = registry->getLc("Lc_span");
    const auto& Chan_span_lc = registry->getChan("Chan_span_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_odd_lp = registry->getLc("Lc_odd_lp");
    const auto& Chan_odd = registry->getChan("Chan_odd");
    const auto& Chan_log = registry->getChan("Chan_log");
    const auto& Chan_even = registry->getChan("Chan_even");
    const auto& Lse_real_odd_val = registry->getChan("Lse_real_odd_val");
    const auto& Lse_img_odd_val = registry->getChan("Lse_img_odd_val");
    const auto& Lse_real_even_val = registry->getChan("Lse_real_even_val");
    const auto& Lse_img_even_val = registry->getChan("Lse_img_even_val");
    const auto& Chan_tt = registry->getChan("Chan_tt");
    const auto& Chan_real_even_val_update = registry->getChan("Chan_real_even_val_update");
    const auto& Chan_real_odd_val_update = registry->getChan("Chan_real_odd_val_update");
    const auto& Chan_img_even_val_update = registry->getChan("Chan_img_even_val_update");
    const auto& Chan_img_odd_val_update = registry->getChan("Chan_img_odd_val_update");
    const auto& Chan_rootindex = registry->getChan("Chan_rootindex");
    const auto& Chan_rootindex_cmp = registry->getChan("Chan_rootindex_cmp");
    const auto& Chan_rootindex_active = registry->getChan("Chan_rootindex_active");
    const auto& Lse_real_twid_val = registry->getChan("Lse_real_twid_val");
    const auto& Lse_img_twid_val = registry->getChan("Lse_img_twid_val");
    const auto& Chan_real_twid_real = registry->getChan("Chan_real_twid_real");
    const auto& Chan_real_twid_img = registry->getChan("Chan_real_twid_img");
    const auto& Chan_img_twid_img = registry->getChan("Chan_img_twid_img");
    const auto& Chan_img_twid_real = registry->getChan("Chan_img_twid_real");
    const auto& Chan_real_odd_update = registry->getChan("Chan_real_odd_update");
    const auto& Chan_img_odd_update = registry->getChan("Chan_img_odd_update");
    const auto& Chan_branch_merge = registry->getChan("Chan_branch_merge");
    //***********************************************************************

    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.chanPtr != nullptr)
        {
            if (entry.chanPtr->masterName == "None" && entry.chanPtr->moduleName != "Chan_begin" && entry.chanPtr->moduleName != "Chan_end")
            {
                entry.chanPtr->size = 30;
                //entry.chanPtr->cycle = 0;
                //entry.chanPtr->speedup = 3;
            }
        }
    }

    // Set speedup
    registry->setSpeedup(chanGraph, "loop_odd", 1);
    registry->setSpeedup(chanGraph, "branch_rootindex", 1);

    // User defined
    //registry->getLse("Lse_m2_data")->noLatencyMode = 1;
    //registry->getLse("Lse_prod_data_update_st")->noLatencyMode = 1;

    //// Initiation
    registry->tableInit();  // Update registry and initial all the module in registry
    registry->pathBalance();
    profiler->init();
    watchdog.addCheckPointChan({ Lc_span->getEnd, Lc_odd_lp->getEnd });

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

        Lc_span->var = Lc_span->mux->mux(Lc_span->var, fft_size, Lc_span->sel);
        Lc_span->mux->muxUpdate(Lc_span->sel);
        Lc_span->mux->outChan->value = Lc_span->var;
        Lc_span->loopVar->get();
        Lc_span->loopVar->value = Lc_span->loopVar->assign(Lc_span->mux->outChan);  // After get(), must update chan's value
        Lc_span->var = Lc_span->loopVar->value >> 1;
        Lc_span->lcUpdate(Lc_span->var != 0);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_span_lc->get();	// Nop
        Chan_span_lc->value = Chan_span_lc->assign(Lc_span->loopVar);

        // Lc: Lc_odd_lp
        Lc_odd_lp->var = Lc_odd_lp->mux->mux(Lc_odd_lp->var, Lc_span->var, Lc_odd_lp->sel);
        Lc_odd_lp->mux->muxUpdate(Lc_odd_lp->sel);
        Lc_odd_lp->mux->outChan->value = Lc_odd_lp->var;
        Lc_odd_lp->loopVar->get();
        Lc_odd_lp->loopVar->value = Lc_odd_lp->loopVar->assign(Lc_odd_lp->mux->outChan);  // After get(), must update chan's value
        Lc_odd_lp->var = Lc_odd_lp->loopVar->value + 1;
        Lc_odd_lp->lcUpdate(Lc_odd_lp->var < fft_size);

        Chan_odd->get();	// Or
        Chan_odd->value = Chan_odd->assign(Lc_odd_lp->loopVar) | Chan_odd->assign(Chan_span_lc);

        Chan_log->get();	// Add
        Chan_log->value = Chan_log->value + 1;

        Chan_even->get();	// Xor
        Chan_even->value = Chan_even->assign(Chan_odd) ^ Chan_even->assign(Chan_span_lc);

        Lse_real_odd_val->get();	// Load
        Lse_real_odd_val->value = real[Lse_real_odd_val->assign()];

        Lse_img_odd_val->get();	// Load
        Lse_img_odd_val->value = img[Lse_img_odd_val->assign()];

        Lse_real_even_val->get();	// Load
        Lse_real_even_val->value = real[Lse_real_even_val->assign()];

        Lse_img_even_val->get();	// Load
        Lse_img_even_val->value = img[Lse_img_even_val->assign()];

        Chan_tt->get();	// Shl
        Chan_tt->value = Chan_tt->assign(Chan_even) << Chan_tt->assign(Chan_log);

        Chan_real_even_val_update->get();	// Add
        Chan_real_even_val_update->value = Chan_real_even_val_update->assign(Lse_real_even_val) + Chan_real_even_val_update->assign(Lse_real_odd_val);

        Chan_real_odd_val_update->get();	// Sub
        Chan_real_odd_val_update->value = Chan_real_odd_val_update->assign(Lse_real_even_val) - Chan_real_odd_val_update->assign(Lse_real_odd_val);

        Chan_img_even_val_update->get();	// Add
        Chan_img_even_val_update->value = Chan_img_even_val_update->assign(Lse_img_even_val) + Chan_img_even_val_update->assign(Lse_img_odd_val);

        Chan_img_odd_val_update->get();	// Sub
        Chan_img_odd_val_update->value = Chan_img_odd_val_update->assign(Lse_img_even_val) - Chan_img_odd_val_update->assign(Lse_img_odd_val);

        Chan_rootindex->get();	// And
        Chan_rootindex->value = Chan_rootindex->assign(Chan_tt) & (fft_size - 1);

        Chan_rootindex_cmp->get();	// Bne
        Chan_rootindex_cmp->value = Chan_rootindex_cmp->assign(Chan_rootindex) > 0 ? 1 : 0;

        Chan_rootindex_active->get();	// Nop
        Chan_rootindex_active->value = Chan_rootindex_active->assign(Chan_rootindex);

        Lse_real_twid_val->get();	// Load
        Lse_real_twid_val->value = real_twid[Lse_real_twid_val->assign()];

        Lse_img_twid_val->get();	// Load
        Lse_img_twid_val->value = img_twid[Lse_img_twid_val->assign()];

        Chan_real_twid_real->get();	// Mul
        Chan_real_twid_real->value = Chan_real_twid_real->assign(Lse_real_twid_val) * Chan_real_twid_real->assign(Chan_real_odd_val_update);

        Chan_real_twid_img->get();	// Mul
        Chan_real_twid_img->value = Chan_real_twid_img->assign(Lse_real_twid_val) * Chan_real_twid_img->assign(Chan_img_odd_val_update);

        Chan_img_twid_img->get();	// Mul
        Chan_img_twid_img->value = Chan_img_twid_img->assign(Lse_img_twid_val) * Chan_img_twid_img->assign(Chan_img_odd_val_update);

        Chan_img_twid_real->get();	// Mul
        Chan_img_twid_real->value = Chan_img_twid_real->assign(Lse_img_twid_val) * Chan_img_twid_real->assign(Chan_real_odd_val_update);

        Chan_real_odd_update->get();	// Sub
        Chan_real_odd_update->value = Chan_real_odd_update->assign(Chan_real_twid_real) - Chan_real_odd_update->assign(Chan_img_twid_img);

        Chan_img_odd_update->get();	// Add
        Chan_img_odd_update->value = Chan_img_odd_update->assign(Chan_real_twid_img) + Chan_img_odd_update->assign(Chan_img_twid_real);

        Chan_branch_merge->get();	// selPartial
        Chan_branch_merge->value = Chan_branch_merge->assign(Chan_rootindex_cmp) ? Chan_branch_merge->assign(Chan_real_odd_update) : Chan_branch_merge->assign(Chan_rootindex);


        // *************************************************************************************

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** Profiler update
        profiler->updateBufferMaxDataNum();
        profiler->updateChanUtilization();

        /*end->get();*/
        Chan_end->get();	// Nop

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (/*37500 > iter && iter > 34500*/ iter >= 0)
        {
            // Print channel
            debug->printSimInfo(simChans, simLcs);

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

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;


    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
    delete profiler;
}
