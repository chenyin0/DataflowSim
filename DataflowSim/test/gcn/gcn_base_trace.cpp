#include "./gcn.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"

using namespace DFSimTest;

void GCN_Test::gcn_Base_trace(Debug* debug)
{
    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    //*** Declare Profiler
    Profiler* profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 50000000);

    //*** Define subgraph scheduler
    GraphScheduler* graphScheduler = new GraphScheduler();

    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(GCN_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint splitNum = 1;
    //chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    GCN_Test::graphPartition(chanGraph, splitNum);

    //chanGraph.addChanDGSF();
    chanGraph.addNodeDelay();
    //chanGraph.setSpeedup(debug);  // Set speedup
    chanGraph.plotDot();

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

    // Generate benchmark data
    generateData();

    // Read mem trace
    deque<uint> nodeTrace;
    deque<uint> featTrace;
    //string fileName = "./resource/gcn/mem_trace/" + dataset_name + "_delta_ngh_deg_30.txt";
    string fileName = "./resource/gcn/mem_trace/" + dataset_name + "_full_retrain.txt";
    readMemTrace(nodeTrace, fileName);
    readMemTrace(featTrace, fileName);
    for_each(featTrace.begin(), featTrace.end(), [](auto& p) {p *= feat_length; });

    ////*** Simulate
    // Declare
    //***********************************************************************

    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Chan_indptr = registry->getChan("Chan_indptr");
    const auto& Lse_access_indptr = registry->getChan("Lse_access_indptr");
    const auto& Chan_ngh_ind_base = registry->getChan("Chan_ngh_ind_base");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Chan_ngh_ind_base_scatter_loop_j = registry->getChan("Chan_ngh_ind_base_scatter_loop_j");
    const auto& Chan_j_lc = registry->getChan("Chan_j_lc");
    const auto& Chan_indices = registry->getChan("Chan_indices");
    const auto& Lse_access_ngh = registry->getChan("Lse_access_ngh");
    const auto& Chan_ngh_ind = registry->getChan("Chan_ngh_ind");
    const auto& Lse_ld_feat = registry->getChan("Lse_ld_feat");
    const auto& Chan_combine = registry->getChan("Chan_combine");
    const auto& Chan_active = registry->getChan("Chan_active");

    //***********************************************************************

    // User defined
    /*registry->getLse("Lse_x")->noLatencyMode = 1;
    registry->getLse("Lse_A")->noLatencyMode = 1;
    registry->getLse("Lse_B")->noLatencyMode = 1;*/

    registry->getLse("Lse_access_indptr")->noLatencyMode = 1;

    // Set speedup manually
    Chan_i_lc->speedup = speedup_aggr;
    Chan_indptr->speedup = speedup_aggr;
    Lse_access_indptr->speedup = speedup_aggr;
    Chan_ngh_ind_base->speedup = speedup_aggr;
    Chan_ngh_ind_base_scatter_loop_j->speedup = speedup_aggr;
    Chan_j_lc->speedup = speedup_aggr;
    Chan_indices->speedup = speedup_aggr;
    Lse_access_ngh->speedup = speedup_aggr;
    Chan_ngh_ind->speedup = speedup_aggr;
    Lse_ld_feat->speedup = speedup_aggr;
    Chan_combine->speedup = speedup_combine;
    Chan_active->speedup = speedup_active;


    //// Initiation
    registry->init();  // Update registry and initial all the module in registry
    graphScheduler->schedulerInit();  // Initial graph scheduler
    profiler->init();
    watchdog.addCheckPointChan({ Lc_i->getEnd });

    registry->getChan("Chan_begin")->get({ 1 });
    uint iter = 0;

    uint max_iter = 500;// 5000000;
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
        Lc_i->var = Lc_i->loopVar->value;  // 22.06.10_debug_yin Make an endless loop
        Lc_i->lcUpdate(Lc_i->var < vertex_num);

        // Clear begin
        Chan_begin->valid = 0;

        Chan_i_lc->get();	// Nop	[0]Lc_i 
        Chan_i_lc->value = Chan_i_lc->assign(uint(0));

        Chan_indptr->get();	// Nop	[0]Chan_i_lc 
        Chan_indptr->value = Chan_indptr->assign(uint(0));

        Lse_access_indptr->get();	// Load	[0]Chan_indptr 
        //Lse_access_indptr->value = indPtr[Lse_access_indptr->assign()];
        Lse_access_indptr->value = indPtr[Lse_access_indptr->assign()];

        Chan_ngh_ind_base->get();	// Nop	[0]Lse_access_indptr 
        Chan_ngh_ind_base->value = Chan_ngh_ind_base->assign(uint(0));

        Chan_ngh_ind_base_scatter_loop_j->get();	// Nop	[0]Chan_ngh_ind_base 
        Chan_ngh_ind_base_scatter_loop_j->value = Chan_ngh_ind_base_scatter_loop_j->assign(uint(0));

        ngh_num = indPtr[Chan_indptr->value + 1] - indPtr[Chan_indptr->value];

        // Lc: Lc_j
        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value;  // 22.06.10_debug_yin Make an endless loop
        Lc_j->lcUpdate(Lc_j->var < ngh_num);

        Chan_j_lc->get();	// Nop	[0]Lc_j 
        Chan_j_lc->value = Chan_j_lc->assign(uint(0));

        Chan_indices->get();	// Add	[0]Chan_j_lc [1]Chan_ngh_ind_base_scatter_loop_j 
        Chan_indices->value = Chan_indices->assign(uint(0)) + Chan_indices->assign(uint(1));

        // Inject mem trace here
        memTraceInjection(Chan_indices, Lse_access_ngh, nodeTrace);

        Lse_access_ngh->get();	// Load	[0]Chan_indices 
        Lse_access_ngh->value = indices[Lse_access_ngh->assign()];

        Chan_ngh_ind->get();	// Nop	[0]Lse_access_ngh 
        Chan_ngh_ind->value = Chan_ngh_ind->assign(uint(0));

        // Inject mem trace here
        memTraceInjection(Chan_ngh_ind, Lse_ld_feat, featTrace);

        /*if (featTrace.size() % 1000 == 0)
        {
            std::cout << "Trace size: " << featTrace.size() << " Node size: " << nodeTrace.size() << std::endl;
        }*/

        Lse_ld_feat->get();	// Load	[0]Chan_ngh_ind 
        Lse_ld_feat->value = feat[Lse_ld_feat->assign()];

        Chan_combine->get();	// Mac	[0]Lse_ld_feat 
        Chan_combine->value = Chan_combine->assign(uint(0));

        Chan_active->get();	// Relu	[0]Chan_combine 
        Chan_active->value = Chan_active->assign(uint(0));

        // *************************************************************************************

        //** Update each chanDGSF
        registry->updateChanDGSF();

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** Profiler update
        profiler->updateBufferMaxDataNum();
        profiler->updateChanUtilization(graphScheduler->currSubgraphId);

        /*end->get();*/
        Chan_end->get();	// Nop

        if (nodeTrace.empty() && featTrace.empty())
        {
            Chan_end->channel.push_back(Data());
        }

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
    profiler->printCacheProfiling();

    //*** Print DRAM 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "DRAM profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printDramProfiling();

    //*** Print power 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Power profiling " << std::endl;
    debug->getFile() << std::endl;
    profiler->printPowerProfiling();

    ////*** TIA profiling
    //debug->getFile() << std::endl;
    //debug->getFile() << "*******************************" << std::endl;
    //debug->getFile() << "TIA profiling " << std::endl;
    //debug->getFile() << std::endl;
    //if (splitNum == 1)
    //{
    //    profiler->tiaProfiling();
    //}

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;


    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete profiler;
    delete graphScheduler;
    delete memSys;
}
