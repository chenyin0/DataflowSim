#include "./ge.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

void Ge_Test::ge_Base(Debug* debug)
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
    ChanGraph chanGraph(Ge_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint64_t splitNum = 4;
    //chanGraph.subgraphPartition(splitNum, debug);
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Ge_Test::graphPartition(chanGraph, splitNum);

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
    auto simChans = std::get<0>(debugPrint);
    auto simLcs = std::get<1>(debugPrint);


    ////*** Simulate
    // Declare
    //***********************************************************************
    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_t = registry->getLc("Lc_t");
    const auto& Chan_t_lc1 = registry->getChan("Chan_t_lc1");
    const auto& Chan_t_lc2 = registry->getChan("Chan_t_lc2");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_i1 = registry->getLc("Lc_i1");
    const auto& Chan_i1_lc = registry->getChan("Chan_i1_lc");
    const auto& Chan_i1_n = registry->getChan("Chan_i1_n");
    const auto& Chan_i1_n_shadow = registry->getChan("Chan_i1_n_shadow");
    const auto& Chan_addr_i1n1 = registry->getChan("Chan_addr_i1n1");
    const auto& Chan_addr_i1i1 = registry->getChan("Chan_addr_i1i1");
    const auto& Lse_A_i1n1 = registry->getChan("Lse_A_i1n1");
    const auto& Lse_A_i1i1 = registry->getChan("Lse_A_i1i1");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Chan_j_lc = registry->getChan("Chan_j_lc");
    const auto& Chan_j_n = registry->getChan("Chan_j_n");
    const auto& Chan_j_n_shadow = registry->getChan("Chan_j_n_shadow");
    const auto& Chan_addr_jj = registry->getChan("Chan_addr_jj");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_i_n = registry->getChan("Chan_i_n");
    const auto& Chan_j_lc_scatter_loop_i = registry->getChan("Chan_j_lc_scatter_loop_i");
    const auto& Chan_j_n_shadow_scatter_loop_i = registry->getChan("Chan_j_n_shadow_scatter_loop_i");
    const auto& Chan_addr_jj_scatter_loop_i = registry->getChan("Chan_addr_jj_scatter_loop_i");
    const auto& Chan_i_n_shadow = registry->getChan("Chan_i_n_shadow");
    const auto& Chan_addr_ij = registry->getChan("Chan_addr_ij");
    const auto& Chan_j_n_shadow_relay_loop_i = registry->getChan("Chan_j_n_shadow_relay_loop_i");
    const auto& Lse_A_jj = registry->getChan("Lse_A_jj");
    const auto& Lse_A_ij = registry->getChan("Lse_A_ij");
    const auto& Chan_c = registry->getChan("Chan_c");
    const auto& Lc_k = registry->getLc("Lc_k");
    const auto& Chan_i_n_shadow_scatter_loop_k = registry->getChan("Chan_i_n_shadow_scatter_loop_k");
    const auto& Chan_j_n_shadow_relay_loop_i_scatter_loop_k = registry->getChan("Chan_j_n_shadow_relay_loop_i_scatter_loop_k");
    const auto& Chan_addr_ik = registry->getChan("Chan_addr_ik");
    const auto& Chan_addr_jk = registry->getChan("Chan_addr_jk");
    const auto& Lse_A_ik = registry->getChan("Lse_A_ik");
    const auto& Chan_c_scatter_loop_k = registry->getChan("Chan_c_scatter_loop_k");
    const auto& Lse_A_jk = registry->getChan("Lse_A_jk");
    const auto& Chan_c_A_jk = registry->getChan("Chan_c_A_jk");
    const auto& Chan_A_ik_update = registry->getChan("Chan_A_ik_update");
    const auto& Lc_j1 = registry->getLc("Lc_j1");
    const auto& Lse_x_j1 = registry->getChan("Lse_x_j1");
    const auto& Chan_i1_n_shadow_scatter_loop_j1 = registry->getChan("Chan_i1_n_shadow_scatter_loop_j1");
    const auto& Chan_addr_i1j1 = registry->getChan("Chan_addr_i1j1");
    const auto& Lse_A_i1j1 = registry->getChan("Lse_A_i1j1");
    const auto& Chan_A_i1j1_x_j1 = registry->getChan("Chan_A_i1j1_x_j1");
    const auto& Chan_sum_update = registry->getChan("Chan_sum_update");
    const auto& Chan_a_sum_shadow = registry->getChan("Chan_a_sum_shadow");
    const auto& Chan_a_sum = registry->getChan("Chan_a_sum");
    const auto& Chan_x_i1 = registry->getChan("Chan_x_i1");
    //***********************************************************************

    // User defined
    //registry->getLse("Lse_A_jj")->noLatencyMode = 1;
    //registry->getLse("Lse_A_i1n1")->noLatencyMode = 1;
    //registry->getLse("Lse_A_i1i1")->noLatencyMode = 1;
    //registry->getLse("Lse_A_ij")->noLatencyMode = 1;
    ////registry->getLse("Lse_x_j1")->noLatencyMode = 1;

    //registry->getLse("Lse_A_ik")->noLatencyMode = 1;
    //registry->getLse("Lse_A_jk")->noLatencyMode = 1;

    //registry->getLse("Lse_A_i1j1")->noLatencyMode = 1;

    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    //graphScheduler->graphSwitchO3 = false;
    profiler->init();
    watchdog.addCheckPointChan({ Lc_t->getEnd, Lc_j->getEnd, Lc_i->getEnd, Lc_k->getEnd, Lc_i1->getEnd, Lc_j1->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint64_t iter = 0;

    // Pre-defined
    Lc_i1->loopVar->enable = 0;
    uint64_t graphSwitchCntAdditional = 0;

    uint64_t max_iter = 5000000;// 5000000;
    uint64_t segment = max_iter / 100;
    uint64_t percent = 0;


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

        // Lc: Lc_t
        Lc_t->var = Lc_t->mux->mux(Lc_t->var, 1, Lc_t->sel);
        Lc_t->mux->muxUpdate(Lc_t->sel);
        Lc_t->mux->outChan->value = Lc_t->var;
        Lc_t->loopVar->get();
        Lc_t->loopVar->value = Lc_t->loopVar->assign(Lc_t->mux->outChan);  // After get(), must update chan's value
        Lc_t->var = Lc_t->loopVar->value + 1;
        Lc_t->lcUpdate(Lc_t->var <= 1);

        Chan_begin->valid = 0;

        Chan_t_lc1->get();	// Nop	[0]Lc_t 
        Chan_t_lc1->value = Chan_t_lc1->assign(uint64_t(0));

        Chan_t_lc2->get();	// Nop	[0]Lc_t 
        Chan_t_lc2->value = Chan_t_lc2->assign(uint64_t(0));

        // Lc: Lc_j
        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 1, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var <= matrix_size);

        // Lc: Lc_i1
        Lc_i1->var = Lc_i1->mux->mux(Lc_i1->var, matrix_size - 1, Lc_i1->sel);
        Lc_i1->mux->muxUpdate(Lc_i1->sel);
        Lc_i1->mux->outChan->value = Lc_i1->var;
        Lc_i1->loopVar->get();
        Lc_i1->loopVar->value = Lc_i1->loopVar->assign(Lc_i1->mux->outChan);  // After get(), must update chan's value
        Lc_i1->var = Lc_i1->loopVar->value - 1;
        Lc_i1->lcUpdate(Lc_i1->var >= 1);

        Chan_j_lc->get();	// Nop	[0]Lc_j 
        Chan_j_lc->value = Chan_j_lc->assign(uint64_t(0));

        Chan_j_n->get();	// Mul	[0]Lc_j 
        Chan_j_n->value = Chan_j_n->assign(uint64_t(0)) * matrix_size;

        Chan_i1_lc->get();	// Nop	[0]Lc_i1 
        Chan_i1_lc->value = Chan_i1_lc->assign(uint64_t(0));

        Chan_i1_n->get();	// Mul	[0]Lc_i1 
        Chan_i1_n->value = Chan_i1_n->assign(uint64_t(0)) * matrix_size;

        // Lc: Lc_i
        Lc_i->var = Lc_i->mux->mux(Lc_i->var, Lc_j->loopVar->upstream[1]->value + 1, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var <= matrix_size);

        Chan_j_n_shadow->get();	// Nop	[0]Chan_j_n 
        Chan_j_n_shadow->value = Chan_j_n_shadow->assign(uint64_t(0));

        // Lc: Lc_j1
        Lc_j1->var = Lc_j1->mux->mux(Lc_j1->var, Lc_j1->loopVar->upstream[1]->value + 1, Lc_j1->sel);
        Lc_j1->mux->muxUpdate(Lc_j1->sel);
        Lc_j1->mux->outChan->value = Lc_j1->var;
        Lc_j1->loopVar->get();
        Lc_j1->loopVar->value = Lc_j1->loopVar->assign(Lc_j1->mux->outChan);  // After get(), must update chan's value
        Lc_j1->var = Lc_j1->loopVar->value + 1;
        Lc_j1->lcUpdate(Lc_j1->var <= matrix_size);

        Chan_i1_n_shadow->get();	// Nop	[0]Chan_i1_n 
        Chan_i1_n_shadow->value = Chan_i1_n_shadow->assign(uint64_t(0));

        Chan_i_lc->get();	// Nop	[0]Lc_i 
        Chan_i_lc->value = Chan_i_lc->assign(uint64_t(0));

        Chan_i_n->get();	// Mul	[0]Lc_i 
        Chan_i_n->value = Chan_i_n->assign(uint64_t(0)) * matrix_size;

        Chan_j_lc_scatter_loop_i->get();	// Nop	[0]Chan_j_lc 
        Chan_j_lc_scatter_loop_i->value = Chan_j_lc_scatter_loop_i->assign(uint64_t(0));

        Chan_j_n_shadow_scatter_loop_i->get();	// Nop	[0]Chan_j_n_shadow 
        Chan_j_n_shadow_scatter_loop_i->value = Chan_j_n_shadow_scatter_loop_i->assign(uint64_t(0));

        Lse_x_j1->get();	// Load	[0]Lc_j1 
        Lse_x_j1->value = x[Lse_x_j1->assign()];

        Chan_i1_n_shadow_scatter_loop_j1->get();	// Nop	[0]Chan_i1_n_shadow 
        Chan_i1_n_shadow_scatter_loop_j1->value = Chan_i1_n_shadow_scatter_loop_j1->assign(uint64_t(0));

        // Lc: Lc_k
        Lc_k->var = Lc_k->mux->mux(Lc_k->var, 1, Lc_k->sel);
        Lc_k->mux->muxUpdate(Lc_k->sel);
        Lc_k->mux->outChan->value = Lc_k->var;
        Lc_k->loopVar->get();
        Lc_k->loopVar->value = Lc_k->loopVar->assign(Lc_k->mux->outChan);  // After get(), must update chan's value
        Lc_k->var = Lc_k->loopVar->value + 1;
        Lc_k->lcUpdate(Lc_k->var <= matrix_size + 1);

        Chan_addr_jj->get();	// Add	[0]Chan_j_n_scatter_loop_i [1]Lc_j 
        Chan_addr_jj->value = Chan_addr_jj->assign(uint64_t(0)) + Chan_addr_jj->assign(uint64_t(1));

        Chan_addr_i1n1->get();	// Add	[0]Chan_i1_n_scatter_loop_j1 
        Chan_addr_i1n1->value = Chan_addr_i1n1->assign(uint64_t(0)) + matrix_size + 1;

        Chan_addr_i1i1->get();	// Add	[0]Chan_i1_n_scatter_loop_j1 [1]Lc_i1 
        Chan_addr_i1i1->value = Chan_addr_i1i1->assign(uint64_t(0)) + Chan_addr_i1i1->assign(uint64_t(1));

        Chan_addr_i1j1->get();	// Add	[0]Chan_i1_n_scatter_loop_j1 [1]Lc_j1 
        Chan_addr_i1j1->value = Chan_addr_i1j1->assign(uint64_t(0)) + Chan_addr_i1j1->assign(uint64_t(1));

        Chan_addr_jj_scatter_loop_i->get();	// Nop	[0]Chan_addr_jj 
        Chan_addr_jj_scatter_loop_i->value = Chan_addr_jj_scatter_loop_i->assign(uint64_t(0));

        Chan_j_n_shadow_relay_loop_i->get();	// Nop	[0]Chan_j_n_shadow_scatter_loop_i 
        Chan_j_n_shadow_relay_loop_i->value = Chan_j_n_shadow_relay_loop_i->assign(uint64_t(0));

        Chan_j_n_shadow_relay_loop_i_scatter_loop_k->get();	// Nop	[0]Chan_j_n_shadow_relay_loop_i 
        Chan_j_n_shadow_relay_loop_i_scatter_loop_k->value = Chan_j_n_shadow_relay_loop_i_scatter_loop_k->assign(uint64_t(0));

        Lse_A_i1n1->get();	// Load	[0]Chan_addr_i1n1 
        Lse_A_i1n1->value = A[Lse_A_i1n1->assign()];

        Lse_A_i1i1->get();	// Load	[0]Chan_addr_i1i1 
        Lse_A_i1i1->value = A[Lse_A_i1i1->assign()];

        Lse_A_i1j1->get();	// Load	[0]Chan_addr_i1j1 
        Lse_A_i1j1->value = A[Lse_A_i1j1->assign()];

        Chan_addr_ij->get();	// Add	[0]Chan_i_n_scatter_loop_k [1]Chan_j_lc_scatter_loop_i 
        Chan_addr_ij->value = Chan_addr_ij->assign(uint64_t(0)) + Chan_addr_ij->assign(uint64_t(1));

        Chan_i_n_shadow->get();	// Nop	[0]Chan_i_n_scatter_loop_k 
        Chan_i_n_shadow->value = Chan_i_n_shadow->assign(uint64_t(0));

        Chan_addr_ik->get();	// Add	[0]Chan_i_n_scatter_loop_k [1]Lc_k 
        Chan_addr_ik->value = Chan_addr_ik->assign(uint64_t(0)) + Chan_addr_ik->assign(uint64_t(1));

        Lse_A_jj->get();	// Load	[0]Chan_addr_jj_scatter_loop_i 
        Lse_A_jj->value = A[Lse_A_jj->assign()];

        Chan_addr_jk->get();	// Add	[0]Lc_k [1]Chan_j_n_relay_loop_i_scatter_loop_k 
        Chan_addr_jk->value = Chan_addr_jk->assign(uint64_t(0)) + Chan_addr_jk->assign(uint64_t(1));

        Chan_A_i1j1_x_j1->get();	// Mul	[0]Lse_A_i1j1 [1]Lse_x_j1 
        Chan_A_i1j1_x_j1->value = Chan_A_i1j1_x_j1->assign(uint64_t(0)) * Chan_A_i1j1_x_j1->assign(uint64_t(1));

        Chan_i_n_shadow_scatter_loop_k->get();	// Nop	[0]Chan_i_n_shadow 
        Chan_i_n_shadow_scatter_loop_k->value = Chan_i_n_shadow_scatter_loop_k->assign(uint64_t(0));

        Lse_A_ij->get();	// Load	[0]Chan_addr_ij 
        Lse_A_ij->value = A[Lse_A_ij->assign()];

        Lse_A_ik->get();	// Load	[0]Chan_addr_ik
        Lse_A_ik->value = A[Lse_A_ik->assign()];

        Lse_A_jk->get();	// Load	[0]Chan_addr_jk 
        Lse_A_jk->value = A[Lse_A_jk->assign()];

        Chan_sum_update->get();	// Add	[0]Chan_A_i1j1_x_j1 
        Chan_sum_update->value = Chan_sum_update->assign(uint64_t(0)) + Chan_sum_update->value;

        Chan_c->get();	// Div	[0]Lse_A_ij [1]Lse_A_jj 
        Chan_c->value = Chan_c->assign(uint64_t(0)) / std::max(Chan_c->assign(uint64_t(1)), 1);

        //Lse_A_ik_update_store->get();	// Store	[0]Chan_addr_ik [1]Chan_A_ik_update 

        Chan_a_sum_shadow->get();	// Nop	[0]Chan_sum_update 
        Chan_a_sum_shadow->value = Chan_a_sum_shadow->assign(uint64_t(0));

        Chan_a_sum->get();	// Sub	[0]Lse_A_i1n1 [1]Chan_sum_update 
        Chan_a_sum->value = Chan_a_sum->assign(uint64_t(0)) - Chan_a_sum->assign(uint64_t(1));

        Chan_c_scatter_loop_k->get();	// Nop	[0]Chan_c 
        Chan_c_scatter_loop_k->value = Chan_c_scatter_loop_k->assign(uint64_t(0));

        Chan_x_i1->get();	// Div	[0]Chan_a_sum [1]Lse_A_i1i1 
        Chan_x_i1->value = Chan_x_i1->assign(uint64_t(0)) / std::max(Chan_x_i1->assign(uint64_t(1)), 1);

        Chan_c_A_jk->get();	// Mul	[0]Chan_c_scatter_loop_k [1]Lse_A_jk 
        Chan_c_A_jk->value = Chan_c_A_jk->assign(uint64_t(0)) * Chan_c_A_jk->assign(uint64_t(1));

        Chan_A_ik_update->get();	// Sub	[0]Lse_A_ik [1]Chan_c_A_jk 
        Chan_A_ik_update->value = Chan_A_ik_update->assign(uint64_t(0)) - Chan_A_ik_update->assign(uint64_t(1));

        //Lse_x_i1_store->get();	// Store	[0]Lc_i1_DGSF [1]Chan_x_i1_DGSF 

        //if (Lc_j->loopVar->valid && Lc_j->loopVar->channel.front().last)
        //{
        //    Lc_i1->loopVar->enable = 1;
        //    //Lc_j->loopVar->enable = 0;
        //}

        if (Chan_A_ik_update->getTheLastData.front())
        {
            Lc_i1->loopVar->enable = 1;
            //Lc_j->loopVar->enable = 0;
        }

        if (Chan_c->pushChannelSuccess || Chan_a_sum->pushChannelSuccess)
        {
            ++graphSwitchCntAdditional;
        }

        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** GraphScheduler update
        if (splitNum > 1 && ClkDomain::getInstance()->checkClkAdd())
        {
            //graphScheduler->graphUpdate();

            // Patch
            if (splitNum >= 6 && graphScheduler->subgraphTimeout > 30)
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

        if (217856 > iter && iter > 215856 /*iter >= 0*/)
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

            std::cout << "Additional Graph switch times: " << graphSwitchCntAdditional << std::endl;

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
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Channel profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printChanProfiling(graphScheduler);

    //*** Print Lse access 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Lse profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printLseProfiling();

    //*** Print cache 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Cache miss rate: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printCacheMissRate();

    //*** Print power 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Power profiling " << std::endl;
    debug->getFile() << std::endl;
    profiler->printPowerProfiling();

    //*** TIA profiling
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "TIA profiling " << std::endl;
    debug->getFile() << std::endl;
    if (splitNum == 1)
    {
        profiler->tiaProfiling();
    }

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;


    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
    delete profiler;
    delete graphScheduler;
}