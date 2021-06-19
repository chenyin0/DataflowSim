#include "./viterbi.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"

using namespace DFSimTest;

void Viterbi_Test::viterbi_Base(Debug* debug)
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
    ChanGraph chanGraph(Viterbi_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint splitNum = 1;
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    Viterbi_Test::graphPartition(chanGraph, splitNum);

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
    const auto& Lc_t = registry->getLc("Lc_t");
    const auto& Chan_t_lc = registry->getChan("Chan_t_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_curr = registry->getLc("Lc_curr");
    const auto& Chan_curr_lc = registry->getChan("Chan_curr_lc");
    const auto& Chan_curr_N = registry->getChan("Chan_curr_N");
    const auto& Chan_prev = registry->getChan("Chan_prev");
    const auto& Chan_t_lc_scatter_loop_curr = registry->getChan("Chan_t_lc_scatter_loop_curr");
    const auto& Lc_prev_ = registry->getLc("Lc_prev_");
    const auto& Chan_curr_N_obs = registry->getChan("Chan_curr_N_obs");
    const auto& Chan_prev_N = registry->getChan("Chan_prev_N");
    const auto& Lse_llike = registry->getChan("Lse_llike");
    const auto& Chan_t_lc_relay_loop_curr = registry->getChan("Chan_t_lc_relay_loop_curr");
    const auto& Chan_prev_N_ = registry->getChan("Chan_prev_N_");
    const auto& Chan_curr_lc_scatter_loop_prev_ = registry->getChan("Chan_curr_lc_scatter_loop_prev_");
    const auto& Lse_emission = registry->getChan("Lse_emission");
    const auto& Chan_prev_N_curr = registry->getChan("Chan_prev_N_curr");
    const auto& Chan_t_lc_relay_loop_curr_scatter_loop_prev_ = registry->getChan("Chan_t_lc_relay_loop_curr_scatter_loop_prev_");
    const auto& Chan_prev_N_curr_ = registry->getChan("Chan_prev_N_curr_");
    const auto& Chan_curr_N_ = registry->getChan("Chan_curr_N_");
    const auto& Chan_prev_N_curr_shadow = registry->getChan("Chan_prev_N_curr_shadow");
    const auto& Lse_transition = registry->getChan("Lse_transition");
    const auto& Lse_llike_ = registry->getChan("Lse_llike_");
    const auto& Chan_curr_N_obs_ = registry->getChan("Chan_curr_N_obs_");
    const auto& Chan_prev_N_curr_shadow_scatter_loop_prev_ = registry->getChan("Chan_prev_N_curr_shadow_scatter_loop_prev_");
    const auto& Chan_llike_transition = registry->getChan("Chan_llike_transition");
    const auto& Lse_emission_ = registry->getChan("Lse_emission_");
    const auto& Lse_transition_ = registry->getChan("Lse_transition_");
    const auto& Chan_min_p = registry->getChan("Chan_min_p");
    const auto& Chan_llike_transition_ = registry->getChan("Chan_llike_transition_");
    const auto& Lse_llike_update = registry->getChan("Lse_llike_update");
    const auto& Chan_p = registry->getChan("Chan_p");

    //***********************************************************************

    // User defined
    //registry->getLse("Lse_m2_data")->noLatencyMode = 1;
    //registry->getLse("Lse_prod_data_update_st")->noLatencyMode = 1;

    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    profiler->init();
    watchdog.addCheckPointChan({ Lc_t->getEnd, Lc_curr->getEnd, Lc_prev_->getEnd });

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

        Lc_t->var = Lc_t->mux->mux(Lc_t->var, 1, Lc_t->sel);
        Lc_t->mux->muxUpdate(Lc_t->sel);
        Lc_t->mux->outChan->value = Lc_t->var;
        Lc_t->loopVar->get();
        Lc_t->loopVar->value = Lc_t->loopVar->assign(Lc_t->mux->outChan);  // After get(), must update chan's value
        Lc_t->var = Lc_t->loopVar->value + 1;
        Lc_t->lcUpdate(Lc_t->var < N_OBS);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_t_lc->get();	// Nop	[0]Lc_t 
        Chan_t_lc->value = Chan_t_lc->assign(uint(0));

        Lc_curr->var = Lc_curr->mux->mux(Lc_curr->var, 0, Lc_curr->sel);
        Lc_curr->mux->muxUpdate(Lc_curr->sel);
        Lc_curr->mux->outChan->value = Lc_curr->var;
        Lc_curr->loopVar->get();
        Lc_curr->loopVar->value = Lc_curr->loopVar->assign(Lc_curr->mux->outChan);  // After get(), must update chan's value
        Lc_curr->var = Lc_curr->loopVar->value + 1;
        Lc_curr->lcUpdate(Lc_curr->var < N_STATES);

        Chan_curr_lc->get();	// Nop	[0]Lc_curr 
        Chan_curr_lc->value = Chan_curr_lc->assign(uint(0));

        Chan_curr_N->get();	// Mul	[0]Lc_curr 
        Chan_curr_N->value = Chan_curr_N->assign(uint(0)) * N_TOKENS;

        Chan_prev->get();	// Nop	
        Chan_prev->value = 0;

        Chan_t_lc_scatter_loop_curr->get();	// Nop	[0]Chan_t_lc 
        Chan_t_lc_scatter_loop_curr->value = Chan_t_lc_scatter_loop_curr->assign(uint(0));

        Lc_prev_->var = Lc_prev_->mux->mux(Lc_prev_->var, 1, Lc_prev_->sel);
        Lc_prev_->mux->muxUpdate(Lc_prev_->sel);
        Lc_prev_->mux->outChan->value = Lc_prev_->var;
        Lc_prev_->loopVar->get();
        Lc_prev_->loopVar->value = Lc_prev_->loopVar->assign(Lc_prev_->mux->outChan);  // After get(), must update chan's value
        Lc_prev_->var = Lc_prev_->loopVar->value + 1;
        Lc_prev_->lcUpdate(Lc_prev_->var < N_STATES);

        Chan_curr_N_obs->get();	// Add	[0]Chan_curr_N 
        Chan_curr_N_obs->value = Chan_curr_N_obs->assign(uint(0)) + obs[Lc_t->var];

        Chan_prev_N->get();	// Mul	[0]Chan_prev 
        Chan_prev_N->value = Chan_prev_N->assign(uint(0)) * N_STATES;

        Lse_llike->get();	// Load	[0]Chan_t_lc_scatter_loop_curr [1]Chan_prev 
        Lse_llike->value = llike[Lse_llike->assign()];

        Chan_t_lc_relay_loop_curr->get();	// Nop	[0]Chan_t_lc_scatter_loop_curr 
        Chan_t_lc_relay_loop_curr->value = Chan_t_lc_relay_loop_curr->assign(uint(0));

        Chan_prev_N_->get();	// Mul	[0]Lc_prev_ 
        Chan_prev_N_->value = Chan_prev_N_->assign(uint(0)) * N_STATES;

        Chan_curr_lc_scatter_loop_prev_->get();	// Nop	[0]Chan_curr_lc 
        Chan_curr_lc_scatter_loop_prev_->value = Chan_curr_lc_scatter_loop_prev_->assign(uint(0));

        Lse_emission->get();	// Load	[0]Chan_curr_N_obs 
        int a = Lse_emission->assign();
        Lse_emission->value = emission[Lse_emission->assign()];

        Chan_prev_N_curr->get();	// Add	[0]Chan_prev_N [1]Lc_curr 
        Chan_prev_N_curr->value = Chan_prev_N_curr->assign(uint(0)) + Chan_prev_N_curr->assign(uint(1));

        Chan_t_lc_relay_loop_curr_scatter_loop_prev_->get();	// Nop	[0]Chan_t_lc_relay_loop_curr 
        Chan_t_lc_relay_loop_curr_scatter_loop_prev_->value = Chan_t_lc_relay_loop_curr_scatter_loop_prev_->assign(uint(0));

        Chan_prev_N_curr_->get();	// Add	[0]Chan_prev_N_ [1]Chan_curr_lc_scatter_loop_prev_ 
        Chan_prev_N_curr_->value = Chan_prev_N_curr_->assign(uint(0)) + Chan_prev_N_curr_->assign(uint(1));

        Chan_curr_N_->get();	// Mul	[0]Chan_curr_lc_scatter_loop_prev_ 
        Chan_curr_N_->value = Chan_curr_N_->assign(uint(0)) * N_TOKENS;

        Chan_prev_N_curr_shadow->get();	// Nop	[0]Chan_prev_N_curr 
        Chan_prev_N_curr_shadow->value = Chan_prev_N_curr_shadow->assign(uint(0));

        Lse_transition->get();	// Load	[0]Chan_prev_N_curr 
        Lse_transition->value = transition[Lse_transition->assign()];

        Lse_llike_->get();	// Load	[0]Lc_prev_ [1]Chan_t_lc_relay_loop_curr_scatter_loop_prev_ 
        Lse_llike_->value = llike[Lse_llike_->assign()];

        Chan_curr_N_obs_->get();	// Add	[0]Chan_curr_N_ 
        Chan_curr_N_obs_->value = Chan_curr_N_obs_->assign(uint(0)) + obs[Lc_t->var];

        Chan_prev_N_curr_shadow_scatter_loop_prev_->get();	// Nop	[0]Chan_prev_N_curr_shadow 
        Chan_prev_N_curr_shadow_scatter_loop_prev_->value = Chan_prev_N_curr_shadow_scatter_loop_prev_->assign(uint(0));

        Chan_llike_transition->get();	// Add	[0]Lse_llike [1]Lse_transition 
        Chan_llike_transition->value = Chan_llike_transition->assign(uint(0)) + Chan_llike_transition->assign(uint(1));

        Lse_emission_->get();	// Load	[0]Chan_curr_N_obs_ 
        Lse_emission_->value = emission[Lse_emission_->assign()];

        Lse_transition_->get();	// Load	[0]Chan_prev_N_curr_shadow_scatter_loop_prev_ 
        Lse_transition_->value = transition[Lse_transition_->assign()];

        Chan_min_p->get();	// Add	[0]Chan_llike_transition [1]Lse_emission 
        Chan_min_p->value = Chan_min_p->assign(uint(0)) + Chan_min_p->assign(uint(1));

        Chan_llike_transition_->get();	// Add	[0]Lse_llike_ [1]Lse_transition_ 
        Chan_llike_transition_->value = Chan_llike_transition_->assign(uint(0)) + Chan_llike_transition_->assign(uint(1));

        Lse_llike_update->get();	// Store	[0]Lc_curr [1]Chan_min_p 

        Chan_p->get();	// Add	[0]Chan_llike_transition_ [1]Lse_emission_ 
        Chan_p->value = Chan_p->assign(uint(0)) + Chan_p->assign(uint(1));

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

        if (13056 > iter && iter > 11056 /*iter >= 0*/)
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
