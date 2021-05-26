#include "./Lud.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

void LudTest::lud_Base_auto(Debug* debug)
{
    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(LudTest::dfg);
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
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc1 = registry->getChan("Chan_i_lc1");
    const auto& Chan_i_lc2 = registry->getChan("Chan_i_lc2");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_j1 = registry->getLc("Lc_j1");
    const auto& Chan_i_size_ = registry->getChan("Chan_i_size_");
    const auto& Lc_j2 = registry->getLc("Lc_j2");
    const auto& Chan_j1_lc = registry->getChan("Chan_j1_lc");
    const auto& Chan_i_size = registry->getChan("Chan_i_size");
    const auto& Chan_i_size_i = registry->getChan("Chan_i_size_i");
    const auto& Chan_j_size = registry->getChan("Chan_j_size");
    const auto& Chan_j2_lc = registry->getChan("Chan_j2_lc");
    const auto& Chan_i_lc2_relay_loop_j2 = registry->getChan("Chan_i_lc2_relay_loop_j2");
    const auto& Lc_k1 = registry->getLc("Lc_k1");
    const auto& Chan_i_size_shadow = registry->getChan("Chan_i_size_shadow");
    const auto& Chan_i_size_j = registry->getChan("Chan_i_size_j");
    const auto& Lse_a_i_size_i = registry->getChan("Lse_a_i_size_i");
    const auto& Chan_j_size_shadow = registry->getChan("Chan_j_size_shadow");
    const auto& Chan_j_size_i = registry->getChan("Chan_j_size_i");
    const auto& Lc_k2 = registry->getLc("Lc_k2");
    const auto& Chan_k1_size = registry->getChan("Chan_k1_size");
    const auto& Chan_i_size_k = registry->getChan("Chan_i_size_k");
    const auto& Lse_sum_j1 = registry->getChan("Lse_sum_j1");
    const auto& Lse_sum_j2 = registry->getChan("Lse_sum_j2");
    const auto& Chan_j_size_k = registry->getChan("Chan_j_size_k");
    const auto& Chan_k2_size = registry->getChan("Chan_k2_size");
    const auto& Chan_k_size_j = registry->getChan("Chan_k_size_j");
    const auto& Lse_a1 = registry->getChan("Lse_a1");
    const auto& Chan_sum_j1_shadow = registry->getChan("Chan_sum_j1_shadow");
    const auto& Chan_sum_j2_shadow = registry->getChan("Chan_sum_j2_shadow");
    const auto& Lse_a3 = registry->getChan("Lse_a3");
    const auto& Chan_k_size_i = registry->getChan("Chan_k_size_i");
    const auto& Lse_a2 = registry->getChan("Lse_a2");
    const auto& Lse_a4 = registry->getChan("Lse_a4");
    const auto& Chan_a1_a2 = registry->getChan("Chan_a1_a2");
    const auto& Chan_a3_a4 = registry->getChan("Chan_a3_a4");
    const auto& Chan_sum_update_k1 = registry->getChan("Chan_sum_update_k1");
    const auto& Chan_sum_update_k2 = registry->getChan("Chan_sum_update_k2");
    const auto& Chan_sum_update_k1_drain = registry->getChan("Chan_sum_update_k1_drain");
    const auto& Chan_sum_update_k2_drain = registry->getChan("Chan_sum_update_k2_drain");
    const auto& Lse_a_update_j1 = registry->getChan("Lse_a_update_j1");
    const auto& Chan_sum_div = registry->getChan("Chan_sum_div");
    const auto& Lse_a_update_j2 = registry->getChan("Lse_a_update_j2");
    //***********************************************************************

    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.chanPtr != nullptr)
        {
            if (entry.chanPtr->masterName == "None" && entry.chanPtr->moduleName != "Chan_begin" && entry.chanPtr->moduleName != "Chan_end")
            {
                entry.chanPtr->size = 20;
                //entry.chanPtr->cycle = 0;
                //entry.chanPtr->speedup = 3;
            }
        }
    }

    // Set speedup
    registry->setSpeedup(chanGraph, "loop_i", 1);
    registry->setSpeedup(chanGraph, "loop_j1", 1);
    registry->setSpeedup(chanGraph, "loop_k1", 10);
    registry->setSpeedup(chanGraph, "loop_j2", 1);
    registry->setSpeedup(chanGraph, "loop_k2", 10);

    // User defined
    registry->getLse("Lse_a_update_j1")->noLatencyMode = 1;
    registry->getLse("Lse_a_update_j2")->noLatencyMode = 1;

    //// Initiation
    registry->tableInit();  // Update registry and initial all the module in registry
    registry->pathBalance();
    profiler->init();
    watchdog.addCheckPointChan({ Lc_i->getEnd, Lc_j1->getEnd, Lc_k1->getEnd, Lc_j2->getEnd, Lc_k2->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint iter = 0;

    // Pre-defined
    Lc_j1->loopVar->enable = 1;
    Lc_j2->loopVar->enable = 0;

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

        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < matrix_size);

        //registry->getChan("Chan_begin")->get({ 1 });
        // Clear begin
        //if (!Lc_i->loopNumQ.empty())
        //{
        //    Chan_begin->valid = 0;
        //}
        Chan_begin->valid = 0;

        Chan_i_lc1->get();	// Nop
        Chan_i_lc1->value = Chan_i_lc1->assign(Lc_i->loopVar);

        Chan_i_lc2->get();	// Nop
        Chan_i_lc2->value = Chan_i_lc2->assign(Lc_i->loopVar);

        Chan_i_size->get();	// Mul
        Chan_i_size->value = Chan_i_size->assign(Chan_i_lc1) * matrix_size;

        Lc_j1->var = Lc_j1->mux->mux(Lc_j1->var, Chan_i_lc1->value, Lc_j1->sel);
        Lc_j1->mux->muxUpdate(Lc_j1->sel);
        Lc_j1->mux->outChan->value = Lc_j1->var;
        Lc_j1->loopVar->get();
        Lc_j1->loopVar->value = Lc_j1->loopVar->assign(Lc_j1->mux->outChan);  // After get(), must update chan's value
        Lc_j1->var = Lc_j1->loopVar->value + 1;
        Lc_j1->lcUpdate(Lc_j1->var < matrix_size);

        //if (Lc_j1->loopVar->valid && Lc_j1->loopVar->channel.front().last)
        //{
        //    Lc_j1->loopVar->enable = 0;
        //    Lc_j2->loopVar->enable = 1;
        //}

        Chan_i_size_->get();	// Mul
        Chan_i_size_->value = Chan_i_size_->assign(Chan_i_lc2) * matrix_size;

        Lc_j2->var = Lc_j2->mux->mux(Lc_j2->var, Chan_i_lc2->value + 1, Lc_j2->sel);
        Lc_j2->mux->muxUpdate(Lc_j2->sel);
        Lc_j2->mux->outChan->value = Lc_j2->var;
        Lc_j2->loopVar->get();
        Lc_j2->loopVar->value = Lc_j2->loopVar->assign(Lc_j2->mux->outChan);  // After get(), must update chan's value
        Lc_j2->var = Lc_j2->loopVar->value + 1;
        Lc_j2->lcUpdate(Lc_j2->var < matrix_size);

        //if (Lc_j2->loopVar->valid && Lc_j2->loopVar->channel.front().last)
        //{
        //    Lc_j2->loopVar->enable = 0;
        //    Lc_j1->loopVar->enable = 1;
        //}

        Chan_i_size_shadow->get();	// Nop
        Chan_i_size_shadow->value = Chan_i_size_shadow->assign(Chan_i_size);

        Chan_i_size_j->get();	// Add
        Chan_i_size_j->value = Chan_i_size_j->assign(Chan_i_size) + Chan_i_size_j->assign(Lc_j1->loopVar);

        Chan_j1_lc->get();	// Nop
        Chan_j1_lc->value = Chan_j1_lc->assign(Lc_j1->loopVar);

        Chan_i_size_i->get();	// Add
        Chan_i_size_i->value = Chan_i_size_i->assign(Chan_i_size_) + Chan_i_size_i->assign(Chan_i_lc2);

        Chan_j_size->get();	// Mul
        Chan_j_size->value = Chan_j_size->assign(Lc_j2->loopVar) * matrix_size;

        Chan_j2_lc->get();	// Nop
        Chan_j2_lc->value = Chan_j2_lc->assign(Lc_j2->loopVar);

        Chan_i_lc2_relay_loop_j2->get();	// Nop
        Chan_i_lc2_relay_loop_j2->value = Chan_i_lc2_relay_loop_j2->assign(Chan_i_lc2);

        Lse_sum_j1->get();	// Load
        Lse_sum_j1->value = matrix[Lse_sum_j1->assign()];

        Lc_k1->var = Lc_k1->mux->mux(Lc_k1->var, 0, Lc_k1->sel);
        Lc_k1->mux->muxUpdate(Lc_k1->sel);
        Lc_k1->mux->outChan->value = Lc_k1->var;
        Lc_k1->loopVar->get();
        Lc_k1->loopVar->value = Lc_k1->loopVar->assign(Lc_k1->mux->outChan);  // After get(), must update chan's value
        Lc_k1->var = Lc_k1->loopVar->value + 1;
        Lc_k1->lcUpdate(Lc_k1->var < /*matrix_size*/Chan_i_lc1->value);

        Lse_a_i_size_i->get();	// Load
        Lse_a_i_size_i->value = matrix[Lse_a_i_size_i->assign()];

        Chan_j_size_shadow->get();	// Nop
        Chan_j_size_shadow->value = Chan_j_size_shadow->assign(Chan_j_size);

        Lc_k2->var = Lc_k2->mux->mux(Lc_k2->var, 0, Lc_k2->sel);
        Lc_k2->mux->muxUpdate(Lc_k2->sel);
        Lc_k2->mux->outChan->value = Lc_k2->var;
        Lc_k2->loopVar->get();
        Lc_k2->loopVar->value = Lc_k2->loopVar->assign(Lc_k2->mux->outChan);  // After get(), must update chan's value
        Lc_k2->var = Lc_k2->loopVar->value + 1;
        Lc_k2->lcUpdate(Lc_k2->var < /*matrix_size*/Chan_i_lc2->value);

        Chan_i_size_k->get();	// Add
        Chan_i_size_k->value = Chan_i_size_k->assign(Chan_i_size_shadow) + Chan_i_size_k->assign(Lc_k1->loopVar);

        Chan_k1_size->get();	// Mul
        Chan_k1_size->value = Chan_k1_size->assign(Lc_k1->loopVar) * matrix_size;

        Chan_j_size_i->get();	// Add
        Chan_j_size_i->value = Chan_j_size_i->assign(Chan_j_size) + Chan_j_size_i->assign(Chan_i_lc2);

        Chan_j_size_k->get();	// Add
        Chan_j_size_k->value = Chan_j_size_k->assign(Chan_j_size_shadow) + Chan_j_size_k->assign(Lc_k2->loopVar);

        Chan_k2_size->get();	// Mul
        Chan_k2_size->value = Chan_k2_size->assign(Lc_k2->loopVar) * matrix_size;

        Lse_a1->get();	// Load
        Lse_a1->value = matrix[Lse_a1->assign()];

        Chan_k_size_j->get();	// Add
        Chan_k_size_j->value = Chan_k_size_j->assign(Chan_k1_size) + Chan_k_size_j->assign(Chan_j1_lc);

        Lse_sum_j2->get();	// Load
        Lse_sum_j2->value = matrix[Lse_sum_j2->assign()];

        Chan_sum_j1_shadow->get();	// Nop
        Chan_sum_j1_shadow->value = Chan_sum_j1_shadow->assign(Lse_sum_j1);

        Chan_sum_j2_shadow->get();	// Nop
        Chan_sum_j2_shadow->value = Chan_sum_j2_shadow->assign(Lse_sum_j2);

        Lse_a3->get();	// Load
        Lse_a3->value = matrix[Lse_a3->assign()];

        Chan_k_size_i->get();	// Add
        Chan_k_size_i->value = Chan_k_size_i->assign(Chan_k2_size) + Chan_k_size_i->assign(Chan_i_lc2_relay_loop_j2);

        Lse_a2->get();	// Load
        Lse_a2->value = matrix[Lse_a2->assign()];

        Lse_a4->get();	// Load
        Lse_a4->value = matrix[Lse_a4->assign()];

        Chan_a1_a2->get();	// Mul
        Chan_a1_a2->value = Chan_a1_a2->assign(Lse_a1) * Chan_a1_a2->assign(Lse_a2);

        Chan_a3_a4->get();	// Mul
        Chan_a3_a4->value = Chan_a3_a4->assign(Lse_a3) * Chan_a3_a4->assign(Lse_a4);

        Chan_sum_update_k1->get();	// Sub
        Chan_sum_update_k1->value = Chan_sum_update_k1->assign(Chan_sum_j1_shadow) - Chan_sum_update_k1->assign(Chan_a1_a2);

        Chan_sum_update_k2->get();	// Sub
        Chan_sum_update_k2->value = Chan_sum_update_k2->assign(Chan_sum_j2_shadow) - Chan_sum_update_k2->assign(Chan_a3_a4);

        Chan_sum_update_k1_drain->get();	// Nop
        Chan_sum_update_k1_drain->value = Chan_sum_update_k1_drain->assign(Chan_sum_update_k1);

        Chan_sum_update_k2_drain->get();	// Nop
        Chan_sum_update_k2_drain->value = Chan_sum_update_k2_drain->assign(Chan_sum_update_k2);

        Lse_a_update_j1->get();	// Store

        //if (Lse_a_update_j1->valid && Lse_a_update_j1->channel.front().last)
        //{
        //    Lc_j2->loopVar->enable = 1;
        //}

        Chan_sum_div->get();	// Div
        Chan_sum_div->value = Chan_sum_div->assign(Chan_sum_update_k2_drain) / max(Chan_sum_div->assign(Lse_a_i_size_i), 1);

        Lse_a_update_j2->get();	// Store

        //if (Lse_a_update_j2->valid && Lse_a_update_j2->channel.front().last)
        //{
        //    Lc_j1->loopVar->enable = 1;
        //}


        if (Lc_j1->loopVar->valid && Lc_j1->loopVar->channel.front().last)
        {
            Lc_j1->loopVar->enable = 0;
            Lc_j2->loopVar->enable = 1;
        }

        if (Lc_j2->loopVar->valid && Lc_j2->loopVar->channel.front().last)
        {
            Lc_j2->loopVar->enable = 0;
            Lc_j1->loopVar->enable = 1;
        }

        if (Lse_a_update_j2->valid && Lse_a_update_j2->channel.front().last)
        {
            Lc_j1->loopVar->enable = 1;
        }

        if (Lse_a_update_j1->valid && Lse_a_update_j1->channel.front().last)
        {
            Lc_j2->loopVar->enable = 1;
        }

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

        if (54656 > iter && iter > 50000 /*iter >= 0*/)
        {
            // Print channel
            debug->printSimInfo(simChans, simLcs);
            if (!Lc_i->loopNumQ.empty())
            {
                debug->getFile() << "Lc_i->loopNumQ.front(): " << Lc_i->loopNumQ.front() << std::endl;
            }

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