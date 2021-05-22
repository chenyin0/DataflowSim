#include "./Gemm.h"
#include "../../src/sim/Watchdog.h"

using namespace DFSimTest;

/*
Src Code(from MachSuite, blocked gemm):

void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
    int i, k, j, jj, kk;
    int i_row, k_row;
    TYPE temp_x, mul;

    loopjj:for (jj = 0; jj < row_size; jj += block_size){
        loopkk:for (kk = 0; kk < row_size; kk += block_size){
            loopi:for ( i = 0; i < row_size; ++i){
                i_row = i * row_size;  // Chan_i_row
                loopk:for (k = 0; k < block_size; ++k){
                    k_row = (k  + kk) * row_size;  // Chan_k_row
                    temp_x = m1[i_row + k + kk];  // Chan_m1_addr, Lse_ld_m1, Chan_m1_getData
                    loopj:for (j = 0; j < block_size; ++j){
                        mul = temp_x * m2[k_row + j + jj];  // Chan_m2_addr, Lse_ld_m2
                        prod[i_row + j + jj] += mul;  // Chan_partialSum_addr, Lse_ld_partialSum, Chan_partialSum, Lse_st_partialSum
                    }
                }
            }
        }
    }
}

*/


/*
Config tips:

1. 如果channel的来源是外层loop_index，则upstream来自外层的keepMode的loop_index channel；
    如果channel的来源是同层loop_index，则upstream来自同层的loopVar

2. 即使channel的数据来源不是同层的loop_index，upstream中也要添加当前循环Lc的loopVar，用来接收当前循环产生的last_tag

*/

void GemmTest::gemm_Base_auto_sim(Debug* debug)
{
    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(GemmTest::dfg);
    chanGraph.addSpecialModeChan();
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
    auto& regis = registry->getRegistryTable();
    //debug->printRegistry(registry);
    debug->printSimNodes(chanGraph);
    registry->genSimConfig(chanGraph);  // Only used for initializing the first time sim

    ////*** Generate gold results
    //for (size_t i = 0; i < matrix_height; ++i)
    //{
    //    for (size_t j = 0; j < matrix_width; ++j)
    //    {
    //        for (size_t k = 0; k < matrix_height; ++k)
    //        {
    //            result[i][j] += m1[i][k] * m2[k][j];
    //        }
    //    }
    //}


    ////*** Simulate
    // Declare
    const auto& Chan_begin = registry->getChan("Chan_begin");
    const auto& Lc_jj = registry->getLc("Lc_jj");
    const auto& Chan_jj_lc = registry->getChan("Chan_jj_lc");
    const auto& Chan_end = registry->getChan("Chan_end");
    const auto& Lc_kk = registry->getLc("Lc_kk");
    const auto& Chan_kk_lc = registry->getChan("Chan_kk_lc");
    const auto& Chan_jj_lc_relay_loop_kk = registry->getChan("Chan_jj_lc_relay_loop_kk");
    const auto& Lc_i = registry->getLc("Lc_i");
    const auto& Chan_i_lc = registry->getChan("Chan_i_lc");
    const auto& Chan_i_row = registry->getChan("Chan_i_row");
    const auto& Chan_jj_lc_relay_loop_i = registry->getChan("Chan_jj_lc_relay_loop_i");
    const auto& Chan_kk_lc_relay_loop_i = registry->getChan("Chan_kk_lc_relay_loop_i");
    const auto& Lc_k = registry->getLc("Lc_k");
    const auto& Chan_k_lc = registry->getChan("Chan_k_lc");
    const auto& Chan_k_kk = registry->getChan("Chan_k_kk");
    const auto& Chan_jj_lc_relay_loop_k = registry->getChan("Chan_jj_lc_relay_loop_k");
    const auto& Chan_i_row_relay_loop_k = registry->getChan("Chan_i_row_relay_loop_k");
    const auto& Lc_j = registry->getLc("Lc_j");
    const auto& Chan_k_row = registry->getChan("Chan_k_row");
    const auto& Chan_m1_addr = registry->getChan("Chan_m1_addr");
    const auto& Chan_j_jj = registry->getChan("Chan_j_jj");
    const auto& Lse_temp_x = registry->getChan("Lse_temp_x");
    const auto& Chan_m2_addr = registry->getChan("Chan_m2_addr");
    const auto& Chan_prod_addr = registry->getChan("Chan_prod_addr");
    const auto& Chan_m1_data = registry->getChan("Chan_m1_data");
    const auto& Lse_m2_data = registry->getChan("Lse_m2_data");
    const auto& Lse_prod_data = registry->getChan("Lse_prod_data");
    const auto& Chan_mul = registry->getChan("Chan_mul");
    const auto& Chan_prod_data_update = registry->getChan("Chan_prod_data_update");
    const auto& Lse_prod_data_update_st = registry->getChan("Lse_prod_data_update_st");

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
    registry->setSpeedup(chanGraph, "loop_j", 2);

    // User defined
    registry->getLse("Lse_prod_data_update_st")->noLatencyMode = 1;

    //// Initiation
    registry->tableInit();  // Update registry and initial all the module in registry
    registry->pathBalance();
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

        Chan_jj_lc->get();	// Nop
        Chan_jj_lc->value = Chan_jj_lc->assign(Lc_jj->loopVar);

        Lc_kk->var = Lc_kk->mux->mux(Lc_kk->var, 0, Lc_kk->sel);
        Lc_kk->mux->muxUpdate(Lc_kk->sel);
        Lc_kk->mux->outChan->value = Lc_kk->var;
        Lc_kk->loopVar->get();
        Lc_kk->loopVar->value = Lc_kk->loopVar->assign(Lc_kk->mux->outChan);  // After get(), must update chan's value
        Lc_kk->var = Lc_kk->loopVar->value + block_size;
        Lc_kk->lcUpdate(Lc_kk->var < matrix_height);

        Chan_kk_lc->get();	// Nop
        Chan_kk_lc->value = Chan_kk_lc->assign(Lc_kk->loopVar);

        Chan_jj_lc_relay_loop_kk->get();	// Nop
        Chan_jj_lc_relay_loop_kk->value = Chan_jj_lc_relay_loop_kk->assign(Chan_jj_lc);

        Lc_i->var = Lc_i->mux->mux(Lc_i->var, 0, Lc_i->sel);
        Lc_i->mux->muxUpdate(Lc_i->sel);
        Lc_i->mux->outChan->value = Lc_i->var;
        Lc_i->loopVar->get();
        Lc_i->loopVar->value = Lc_i->loopVar->assign(Lc_i->mux->outChan);  // After get(), must update chan's value
        Lc_i->var = Lc_i->loopVar->value + 1;
        Lc_i->lcUpdate(Lc_i->var < matrix_height);

        Chan_i_lc->get();	// Nop
        Chan_i_lc->value = Chan_i_lc->assign(Lc_i->loopVar);

        Chan_i_row->get();	// Mul
        Chan_i_row->value = Chan_i_row->assign(Lc_i->loopVar) * matrix_width;

        Chan_jj_lc_relay_loop_i->get();	// Nop
        Chan_jj_lc_relay_loop_i->value = Chan_jj_lc_relay_loop_i->assign(Chan_jj_lc_relay_loop_kk);

        Chan_kk_lc_relay_loop_i->get();	// Nop
        Chan_kk_lc_relay_loop_i->value = Chan_kk_lc_relay_loop_i->assign(Chan_kk_lc);

        Lc_k->var = Lc_k->mux->mux(Lc_k->var, 0, Lc_k->sel);
        Lc_k->mux->muxUpdate(Lc_k->sel);
        Lc_k->mux->outChan->value = Lc_k->var;
        Lc_k->loopVar->get();
        Lc_k->loopVar->value = Lc_k->loopVar->assign(Lc_k->mux->outChan);  // After get(), must update chan's value
        Lc_k->var = Lc_k->loopVar->value + 1;
        Lc_k->lcUpdate(Lc_k->var < block_size);

        Chan_k_lc->get();	// Nop
        Chan_k_lc->value = Chan_k_lc->assign(Lc_k->loopVar);

        Chan_k_kk->get();	// Add
        Chan_k_kk->value = Chan_k_kk->assign(Lc_k->loopVar) + Chan_k_kk->assign(Chan_kk_lc_relay_loop_i);

        Chan_jj_lc_relay_loop_k->get();	// Nop
        Chan_jj_lc_relay_loop_k->value = Chan_jj_lc_relay_loop_k->assign(Chan_jj_lc_relay_loop_i);

        Chan_i_row_relay_loop_k->get();	// Nop
        Chan_i_row_relay_loop_k->value = Chan_i_row_relay_loop_k->assign(Chan_i_row);

        Lc_j->var = Lc_j->mux->mux(Lc_j->var, 0, Lc_j->sel);
        Lc_j->mux->muxUpdate(Lc_j->sel);
        Lc_j->mux->outChan->value = Lc_j->var;
        Lc_j->loopVar->get();
        Lc_j->loopVar->value = Lc_j->loopVar->assign(Lc_j->mux->outChan);  // After get(), must update chan's value
        Lc_j->var = Lc_j->loopVar->value + 1;
        Lc_j->lcUpdate(Lc_j->var < block_size);

        Chan_k_row->get();	// Mul
        Chan_k_row->value = Chan_k_row->assign(Chan_k_kk) * matrix_width;

        Chan_m1_addr->get();	// Add
        Chan_m1_addr->value = Chan_m1_addr->assign(Chan_i_row) + Chan_m1_addr->assign(Chan_k_kk)/* + m1_BaseAddr*/;

        Chan_j_jj->get();	// Add
        Chan_j_jj->value = Chan_j_jj->assign(Lc_j->loopVar) + Chan_j_jj->assign(Chan_jj_lc_relay_loop_k);

        Lse_temp_x->get();	// Load
        Lse_temp_x->value = m1_[Lse_temp_x->assign()/* - m1_BaseAddr*/];

        Chan_m2_addr->get();	// Add
        Chan_m2_addr->value = Chan_m2_addr->assign(Chan_k_row) + Chan_m2_addr->assign(Chan_j_jj)/* + m2_BaseAddr*/;

        Chan_prod_addr->get();	// Add
        Chan_prod_addr->value = Chan_prod_addr->assign(Chan_j_jj) + Chan_prod_addr->assign(Chan_i_row_relay_loop_k)/* + partialSum_BaseAddr*/;

        Chan_m1_data->get();	// Nop
        Chan_m1_data->value = Chan_m1_data->assign(Lse_temp_x);

        Lse_m2_data->get();	// Load
        Lse_m2_data->value = m2_[Lse_m2_data->assign()/* - m2_BaseAddr*/];

        Lse_prod_data->get();	// Load
        Lse_prod_data->value = prod_[Lse_prod_data->assign()/* - partialSum_BaseAddr*/];

        Chan_mul->get();	// Mul
        Chan_mul->value = Chan_mul->assign(Chan_m1_data) * Chan_mul->assign(Lse_m2_data);

        Chan_prod_data_update->get();	// Add
        Chan_prod_data_update->value = Chan_prod_data_update->assign(Lse_prod_data) + Chan_prod_data_update->assign(Chan_mul);

        Lse_prod_data_update_st->get();	// Store


        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** Profiler update
        profiler->updateBufferMaxDataNum();
        profiler->updateChanUtilization();

        /*end->get();*/
        Chan_end->get();	// Nop

        /* std::cout << std::endl;
         if (ldm1Cnt != lse_ld_m1->memAccessCnt)
         {
             std::cout << "ldm1 " << "cnt: " << lse_ld_m1->memAccessCnt << " avg: " << lse_ld_m1->avgMemAccessLat << std::endl;
         }
         if (ldm2Cnt != lse_ld_m2->memAccessCnt)
         {
             std::cout << "ldm2 " << "cnt: " << lse_ld_m2->memAccessCnt << " avg: " << lse_ld_m2->avgMemAccessLat << std::endl;
         }

         if (ldPartialCnt != lse_ld_partialSum->memAccessCnt)
         {
             std::cout << "ldSum " << "cnt: " << lse_ld_partialSum->memAccessCnt << " avg: " << lse_ld_partialSum->avgMemAccessLat << std::endl;
         }

         ldm1Cnt = lse_ld_m1->memAccessCnt;
         ldm2Cnt = lse_ld_m2->memAccessCnt;
         ldPartialCnt = lse_ld_partialSum->memAccessCnt;*/

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (/*37500 > iter && iter > 34500*/ iter >= 0)
        {
            //debug->getFile() << std::endl;
            //debug->getFile() << "Loop index jj: " << Chan_jj_lc_relay_loop_k->value << std::endl;  // Inner most relay channel
            //debug->getFile() << "Loop index kk: " << Chan_kk_lc_relay_loop_i->value << std::endl;  // Inner most relay channel
            //debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;
            //debug->getFile() << "Loop index k: " << chan_k_lc->value << std::endl;
            //debug->getFile() << "Loop index j: " << lc_j->loopVar->value << std::endl;

            ///*debug->vecPrint("Result", res, 15);*/

            //** Print channle
            debug->chanPrint("Chan_begin", Chan_begin);
            debug->getFile() << std::endl;
            debug->getFile() << "************ Lc: " << "Lc_jj" << "***********" << std::endl;
            debug->chanPrint("Lc_jj->loopVar", Lc_jj->loopVar);
            debug->chanPrint("Chan_jj_lc", Chan_jj_lc);
            debug->chanPrint("Chan_end", Chan_end);
            debug->getFile() << std::endl;
            debug->getFile() << "************ Lc: " << "Lc_kk" << "***********" << std::endl;
            debug->chanPrint("Lc_kk->loopVar", Lc_kk->loopVar);
            debug->chanPrint("Chan_kk_lc", Chan_kk_lc);
            debug->chanPrint("Chan_jj_lc_relay_loop_kk", Chan_jj_lc_relay_loop_kk);
            debug->getFile() << std::endl;
            debug->getFile() << "************ Lc: " << "Lc_i" << "***********" << std::endl;
            debug->chanPrint("Lc_i->loopVar", Lc_i->loopVar);
            debug->chanPrint("Chan_i_lc", Chan_i_lc);
            debug->chanPrint("Chan_i_row", Chan_i_row);
            debug->chanPrint("Chan_jj_lc_relay_loop_i", Chan_jj_lc_relay_loop_i);
            debug->chanPrint("Chan_kk_lc_relay_loop_i", Chan_kk_lc_relay_loop_i);
            debug->getFile() << std::endl;
            debug->getFile() << "************ Lc: " << "Lc_k" << "***********" << std::endl;
            debug->chanPrint("Lc_k->loopVar", Lc_k->loopVar);
            debug->chanPrint("Chan_k_lc", Chan_k_lc);
            debug->chanPrint("Chan_k_kk", Chan_k_kk);
            debug->chanPrint("Chan_jj_lc_relay_loop_k", Chan_jj_lc_relay_loop_k);
            debug->chanPrint("Chan_i_row_relay_loop_k", Chan_i_row_relay_loop_k);
            debug->getFile() << std::endl;
            debug->getFile() << "************ Lc: " << "Lc_j" << "***********" << std::endl;
            debug->chanPrint("Lc_j->loopVar", Lc_j->loopVar);
            debug->chanPrint("Chan_k_row", Chan_k_row);
            debug->chanPrint("Chan_m1_addr", Chan_m1_addr);
            debug->chanPrint("Chan_j_jj", Chan_j_jj);
            debug->lsePrint("Lse_temp_x", dynamic_cast<Lse*>(Lse_temp_x));
            debug->chanPrint("Chan_m2_addr", Chan_m2_addr);
            debug->chanPrint("Chan_prod_addr", Chan_prod_addr);
            debug->chanPrint("Chan_m1_data", Chan_m1_data);
            debug->lsePrint("Lse_m2_data", dynamic_cast<Lse*>(Lse_m2_data));
            debug->lsePrint("Lse_prod_data", dynamic_cast<Lse*>(Lse_prod_data));
            debug->chanPrint("Chan_mul", Chan_mul);
            debug->chanPrint("Chan_prod_data_update", Chan_prod_data_update);
            debug->lsePrint("Lse_prod_data_update_st", dynamic_cast<Lse*>(Lse_prod_data_update_st));


            debug->getFile() << std::endl;
            debug->getFile() << "*****************  End signal  *****************" << std::endl;
            debug->chanPrint("Lc_jj->getEnd", Lc_jj->getEnd); debug->getFile() << "Lc_jj loopEnd: " << Lc_jj->loopEnd << std::endl;
            debug->chanPrint("Lc_kk->getEnd", Lc_kk->getEnd); debug->getFile() << "Lc_kk loopEnd: " << Lc_kk->loopEnd << std::endl;
            debug->chanPrint("Lc_i->getEnd", Lc_i->getEnd); debug->getFile() << "Lc_i loopEnd: " << Lc_i->loopEnd << std::endl;
            debug->chanPrint("Lc_k->getEnd", Lc_k->getEnd); debug->getFile() << "Lc_k loopEnd: " << Lc_k->loopEnd << std::endl;
            debug->chanPrint("Lc_j->getEnd", Lc_j->getEnd); debug->getFile() << "Lc_j loopEnd: " << Lc_j->loopEnd << std::endl;

            // Print MemorySystem
            debug->memSysPrint(memSys);

            //    // Debug_yin_12.30
            //    if (iter % 500 == 0)
            //    {
            //        debug->debug_mode = Debug_mode::Print_detail;

            //        debug->getFile() << endl;
            //        debug->getFile() << "*******************************" << endl;
            //        debug->getFile() << "Lse profiling: " << std::endl;
            //        debug->getFile() << std::endl;
            //        profiler->printLseProfiling("lse_ld_m1", lse_ld_m1);
            //        profiler->printLseProfiling("lse_ld_m2", lse_ld_m2);
            //        profiler->printLseProfiling("lse_ld_partialSum", lse_ld_partialSum);
            //        profiler->printLseProfiling("lse_st_partialSum", lse_st_partialSum);
            //    }
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

    //profiler->printBufferMaxDataNum("chan_jj_lc", chan_jj_lc);
    //profiler->printBufferMaxDataNum("chan_kk_lc", chan_kk_lc);
    //profiler->printBufferMaxDataNum("chan_jj_relay_loop_kk", chan_jj_relay_loop_kk);
    //profiler->printBufferMaxDataNum("chan_i_lc", chan_i_lc);
    //profiler->printBufferMaxDataNum("chan_i_row", chan_i_row);
    //profiler->printBufferMaxDataNum("chan_jj_relay_loop_i", chan_jj_relay_loop_i);
    //profiler->printBufferMaxDataNum("chan_kk_relay_loop_i", chan_kk_relay_loop_i);
    //profiler->printBufferMaxDataNum("chan_k_lc", chan_k_lc);
    //profiler->printBufferMaxDataNum("chan_m1_addr", chan_m1_addr);
    //profiler->printBufferMaxDataNum("chan_k_row", chan_k_row);
    //profiler->printBufferMaxDataNum("chan_i_row_relay_loop_k", chan_i_row_relay_loop_k);
    //profiler->printBufferMaxDataNum("lse_ld_m1", lse_ld_m1);
    //profiler->printBufferMaxDataNum("chan_m1_getData", chan_m1_getData);
    //profiler->printBufferMaxDataNum("chan_jj_relay_loop_k", chan_jj_relay_loop_k);
    //profiler->printBufferMaxDataNum("chan_m2_addr", chan_m2_addr);
    //profiler->printBufferMaxDataNum("chan_m2_addr_delay", chan_m2_addr_delay);
    //profiler->printBufferMaxDataNum("lse_ld_m2", lse_ld_m2);
    //profiler->printBufferMaxDataNum("chan_mul", chan_mul);
    //profiler->printBufferMaxDataNum("chan_mul_delay", chan_mul_delay);
    //profiler->printBufferMaxDataNum("chan_partialSum_addr", chan_partialSum_addr);
    //profiler->printBufferMaxDataNum("chan_partialSum_addr_delay", chan_partialSum_addr_delay);
    //profiler->printBufferMaxDataNum("lse_ld_partialSum", lse_ld_partialSum);
    //profiler->printBufferMaxDataNum("chan_partialSum", chan_partialSum);
    //profiler->printBufferMaxDataNum("lse_st_partialSum", lse_st_partialSum);


    //*** Print Lse access 
    debug->getFile() << endl;
    debug->getFile() << "*******************************" << endl;
    debug->getFile() << "Lse profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printLseProfiling("Lse_temp_x", dynamic_cast<Lse*>(Lse_temp_x));
    profiler->printLseProfiling("Lse_m2_data", dynamic_cast<Lse*>(Lse_m2_data));
    profiler->printLseProfiling("Lse_prod_data", dynamic_cast<Lse*>(Lse_prod_data));
    profiler->printLseProfiling("Lse_prod_data_update_st", dynamic_cast<Lse*>(Lse_prod_data_update_st));

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