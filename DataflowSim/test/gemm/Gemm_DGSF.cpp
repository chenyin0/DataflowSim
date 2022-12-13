#include "./Gemm.h"
#include "../../src/module/execution/GraphScheduler.h"
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
                    temp_x = m1[i_row + k + kk];  // Chan_m1_addr, Lse_ld_m1, chan_m1_getData_DGSF_LOOP
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

1. 如果channel的来源是外层循环变量，则upstream来自外层的keepMode的loopIndex channel；
    如果channel的来源是同层循环变量，则upstream来自同层的loopVar

2. 即使channel的数据来源不是同层的循环变量，upstream中也要添加当前循环的loopVar，用来接收当前循环产生的last_tag

*/

void GemmTest::gemm_DGSF(Debug* debug)
{
    // Generate benchmark data
    generateData();


    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    //*** Declare profiler
    Profiler* profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 50);

    uint64_t BASE_INPUT_BUFF_SIZE = Global::base_input_buffer_size;
    uint64_t LSE_QUEUE_SIZE = Global::lse_queue_size;
    uint64_t DGSF_INPUT_BUFF_SIZE = Global::DGSF_input_buffer_size;
    uint64_t BRAM_ACCESS_DELAY = Global::bram_access_delay;
    uint64_t ADD = Global::ADD;
    uint64_t SUB = Global::SUB;
    uint64_t MUL = Global::MUL;
    uint64_t DIV = Global::DIV;

    //*** Declare Lse
    Lse* lse_ld_m1 = new Lse(LSE_QUEUE_SIZE * DGSF_loop_k_speedup, 0, false, memSys, DGSF_loop_k_speedup);  // Load M1
    //lse_ld_m1->noLatencyMode = 1;
    Lse* lse_ld_m2 = new Lse(LSE_QUEUE_SIZE * DGSF_loop_j_speedup, 0, false, memSys, DGSF_loop_j_speedup);  // Load M2
    //lse_ld_m2->noLatencyMode = 1;
    Lse* lse_ld_partialSum = new Lse(LSE_QUEUE_SIZE * DGSF_loop_j_speedup, 0, false, memSys, DGSF_loop_j_speedup);  // load partial sum
    //lse_ld_partialSum->noLatencyMode = 1;
    Lse* lse_st_partialSum = new Lse(20*LSE_QUEUE_SIZE * DGSF_loop_j_speedup, 0, true, memSys, DGSF_loop_j_speedup);  // Store back partial sum
    lse_st_partialSum->noLatencyMode = 1;


    //*** Declare Lc
    // Loop jj
    ChanBase* lc_jj_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_jj_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_jj_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_jj_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_jj_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_jj_mux_outChan = new ChanBase(2, 0, 1);

    ChanBase* chan_jj_lc = new ChanBase(2, 0, 1);
    chan_jj_lc->keepMode = 1;

    Mux* lc_jj_mux = new Mux(lc_jj_mux_trueChan, lc_jj_mux_falseChan, lc_jj_mux_outChan);
    lc_jj_mux->addPort({ lc_jj_loopVar }, { }, { lc_jj_loopVar });
    Lc* lc_jj = new Lc(lc_jj_loopVar, lc_jj_getEnd, lc_jj_sendEnd, lc_jj_mux);
    // Set outer-most loop
    lc_jj->isOuterMostLoop = 1; 

    // Loop kk
    ChanBase* lc_kk_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_kk_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_kk_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_kk_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_kk_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_kk_mux_outChan = new ChanBase(2, 0, 1);

    ChanBase* chan_kk_lc = new ChanBase(2, 0, 1);
    chan_kk_lc->keepMode = 1;

    Mux* lc_kk_mux = new Mux(lc_kk_mux_trueChan, lc_kk_mux_falseChan, lc_kk_mux_outChan);
    lc_kk_mux->addPort({ lc_kk_loopVar }, { }, { lc_kk_loopVar });
    Lc* lc_kk = new Lc(lc_kk_loopVar, lc_kk_getEnd, lc_kk_sendEnd, lc_kk_mux);

    // Loop i
    ChanBase* lc_i_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_i_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_i_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_i_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_i_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_i_mux_outChan = new ChanBase(2, 0, 1);

    ChanBase* chan_i_lc = new ChanBase(2, 0, 1);
    chan_i_lc->keepMode = 1;

    Mux* lc_i_mux = new Mux(lc_i_mux_trueChan, lc_i_mux_falseChan, lc_i_mux_outChan);
    lc_i_mux->addPort({ lc_i_loopVar }, { }, { lc_i_loopVar });
    Lc* lc_i = new Lc(lc_i_loopVar, lc_i_getEnd, lc_i_sendEnd, lc_i_mux);

    // Loop k
    ChanBase* lc_k_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_k_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_k_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_k_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k_mux_outChan = new ChanBase(2, 0, 1);

    ChanDGSF* chan_k_lc_DGSF_LOOP = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_k_speedup);
    chan_k_lc_DGSF_LOOP->keepMode = 1;

    Mux* lc_k_mux = new Mux(lc_k_mux_trueChan, lc_k_mux_falseChan, lc_k_mux_outChan);
    lc_k_mux->addPort({ lc_k_loopVar }, { }, { lc_k_loopVar });
    Lc* lc_k = new Lc(lc_k_loopVar, lc_k_getEnd, lc_k_sendEnd, lc_k_mux);

    // Loop j
    ChanBase* lc_j_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_j_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_j_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_j_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j_mux_outChan = new ChanBase(2, 0, 1);

    //// Inner most loop not need to declare the loop index channel below
    //ChanBase* chan_j_lc = new ChanBase(2, 0);
    //chan_j_lc->keepMode = 1;  // Inner-most loop, not in keepMode

    Mux* lc_j_mux = new Mux(lc_j_mux_trueChan, lc_j_mux_falseChan, lc_j_mux_outChan);
    lc_j_mux->addPort({ lc_j_loopVar }, { }, { lc_j_loopVar });
    Lc* lc_j = new Lc(lc_j_loopVar, lc_j_getEnd, lc_j_sendEnd, lc_j_mux);


    //*** Declare channel
    ChanBase* begin = new ChanBase(1, 0);
    begin->noUpstream = 1;
    ChanBase* end = new ChanBase(1, 0);
    end->noDownstream = 1;

    // loop kk
    ChanBase* chan_jj_relay_loop_kk = new ChanBase(/*BASE_INPUT_BUFF_SIZE*/2, 0, 1);  // Relay channel in loop kk for chan_jj_lc
    chan_jj_relay_loop_kk->keepMode = 1;

    // loop i
    ChanBase* chan_i_row = new ChanBase(10*BASE_INPUT_BUFF_SIZE, MUL, 1);
    chan_i_row->keepMode = 1;

    ChanBase* chan_jj_relay_loop_i = new ChanBase(20*BASE_INPUT_BUFF_SIZE, 0, 1);  // Relay channel in loop i for chan_jj_lc
    chan_jj_relay_loop_i->keepMode = 1;

    ChanBase* chan_kk_relay_loop_i = new ChanBase(2*BASE_INPUT_BUFF_SIZE, 0, 1);  // Relay channel in loop i for chan_kk_lc
    chan_kk_relay_loop_i->keepMode = 1;

    // loop k
    ChanBase* chan_m1_addr = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_loop_k_speedup, 0, DGSF_loop_k_speedup);
    ChanBase* chan_m1_addr_delay = new ChanBase(2 * BASE_INPUT_BUFF_SIZE * DGSF_loop_k_speedup, 2 * ADD, DGSF_loop_k_speedup);

    ChanBase* chan_k_row = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_loop_k_speedup, 0, DGSF_loop_k_speedup);
    ChanBase* chan_k_row_delay = new ChanBase(10*(1 + MUL) * BASE_INPUT_BUFF_SIZE * DGSF_loop_k_speedup, ADD + MUL, DGSF_loop_k_speedup);

    ChanDGSF* chan_k_row_DGSF_LOOP = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_k_speedup);
    chan_k_row_DGSF_LOOP->keepMode = 1;

    ChanDGSF* chan_m1_getData_DGSF_LOOP = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_k_speedup);
    chan_m1_getData_DGSF_LOOP->keepMode = 1;

    ChanBase* chan_jj_relay_loop_k = new ChanBase(20*BASE_INPUT_BUFF_SIZE, 0, DGSF_loop_k_speedup);  // Relay channel in loop k for chan_jj_lc
    //chan_jj_relay_loop_k->keepMode = 1;

    ChanDGSF* chan_jj_relay_loop_k_DGSF_LOOP = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_k_speedup);  // Relay channel in loop k for chan_jj_lc
    chan_jj_relay_loop_k_DGSF_LOOP->keepMode = 1;

    ChanBase* chan_i_row_relay_loop_k = new ChanBase(20*BASE_INPUT_BUFF_SIZE, 0, DGSF_loop_k_speedup);
    //chan_i_row_relay_loop_k->keepMode = 1;

    ChanDGSF* chan_i_row_relay_loop_k_DGSF_LOOP = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_k_speedup);
    chan_i_row_relay_loop_k_DGSF_LOOP->keepMode = 1;

    // loop j
    ChanBase* chan_m2_addr = new ChanBase(10*BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, 0, DGSF_loop_j_speedup);
    ChanBase* chan_m2_addr_delay = new ChanBase(10 * BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, 2 * ADD, DGSF_loop_j_speedup);

    ChanBase* chan_m1_getData_DGSF_DAE_temp = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, DGSF_loop_j_speedup);  // Separate Channel in DGSF & channel in keepMode 
    ChanDGSF* chan_m1_getData_DGSF_DAE = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_j_speedup);  // DAE dual-buffer

    ChanDGSF* lse_ld_m2_DGSF_DAE = new ChanDGSF(DGSF_INPUT_BUFF_SIZE, BRAM_ACCESS_DELAY, DGSF_loop_j_speedup);  // DAE dual-buffer

    ChanBase* chan_mul = new ChanBase(10*BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, 0, DGSF_loop_j_speedup);
    ChanBase* chan_mul_delay = new ChanBase(10*MUL * BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, MUL, DGSF_loop_j_speedup);

    ChanBase* chan_partialSum_addr = new ChanBase(10*BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, 0, DGSF_loop_j_speedup);
    ChanBase* chan_partialSum_addr_delay = new ChanBase(10*BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, 2 * ADD, DGSF_loop_j_speedup);

    ChanBase* chan_partialSum = new ChanBase(10*BASE_INPUT_BUFF_SIZE * DGSF_loop_j_speedup, ADD, DGSF_loop_j_speedup);

    //// Debug_yin_04.15
    //for (auto& entry : registry->getRegistryTable())
    //{
    //    if (entry.chanPtr != nullptr)
    //    {
    //        if (entry.chanPtr->keepMode)
    //        {
    //            entry.chanPtr->size = 2;
    //        }
    //    }
    //}

    //*** Define interconnect
    // Begin & end connect to the outer most loop
    begin->addDownstream({ lc_jj_mux_falseChan });
    end->addUpstream({ lc_jj->sendEnd });

    // Loop index channel
    chan_jj_lc->addUpstream({ lc_jj->loopVar });
    chan_jj_lc->addDownstream({ lc_kk->loopVar, chan_jj_relay_loop_kk });

    chan_kk_lc->addUpstream({ lc_kk->loopVar });
    chan_kk_lc->addDownstream({ lc_i->loopVar, chan_kk_relay_loop_i });

    chan_i_lc->addUpstream({ lc_i->loopVar });
    chan_i_lc->addDownstream({ lc_k->loopVar });

    chan_k_lc_DGSF_LOOP->addUpstream({ lc_k->loopVar });
    chan_k_lc_DGSF_LOOP->addDownstream({ lc_j->loopVar });  // Only add inner loop channels as downstream; As for same loop channel, added as loopVar's downstream

    // loop kk
    chan_jj_relay_loop_kk->addUpstream({ chan_jj_lc, lc_kk->loopVar });
    chan_jj_relay_loop_kk->addDownstream({ chan_jj_relay_loop_i });

    // loop i
    chan_i_row->addUpstream({ lc_i->loopVar });
    chan_i_row->addDownstream({ chan_m1_addr, /*chan_partialSum_addr*/ chan_i_row_relay_loop_k });

    chan_kk_relay_loop_i->addUpstream({ chan_kk_lc, lc_i->loopVar });
    chan_kk_relay_loop_i->addDownstream({ chan_m1_addr, chan_k_row });

    chan_jj_relay_loop_i->addUpstream({ chan_jj_relay_loop_kk, lc_i->loopVar });
    chan_jj_relay_loop_i->addDownstream({ chan_jj_relay_loop_k });

    // loop k
    chan_m1_addr->addUpstream({ chan_i_row, lc_k->loopVar, chan_kk_relay_loop_i });
    chan_m1_addr->addDownstream({ chan_m1_addr_delay });

    chan_m1_addr_delay->addUpstream({ chan_m1_addr });
    chan_m1_addr_delay->addDownstream({ lse_ld_m1 });

    lse_ld_m1->addUpstream({ chan_m1_addr_delay });
    lse_ld_m1->addDownstream({ chan_m1_getData_DGSF_LOOP });

    chan_m1_getData_DGSF_LOOP->addUpstream({ lse_ld_m1 });
    chan_m1_getData_DGSF_LOOP->addDownstream({ chan_m1_getData_DGSF_DAE_temp });

    chan_k_row->addUpstream({ lc_k->loopVar, chan_kk_relay_loop_i });
    chan_k_row->addDownstream({ chan_k_row_delay });

    chan_k_row_delay->addUpstream({ chan_k_row });
    chan_k_row_delay->addDownstream({ chan_k_row_DGSF_LOOP });

    chan_k_row_DGSF_LOOP->addUpstream({ chan_k_row_delay });
    chan_k_row_DGSF_LOOP->addDownstream({ chan_m2_addr });

    chan_jj_relay_loop_k->addUpstream({ chan_jj_relay_loop_i, lc_k->loopVar });
    chan_jj_relay_loop_k->addDownstream({ chan_jj_relay_loop_k_DGSF_LOOP });

    chan_jj_relay_loop_k_DGSF_LOOP->addUpstream({ chan_jj_relay_loop_k });
    chan_jj_relay_loop_k_DGSF_LOOP->addDownstream({ chan_m2_addr, chan_partialSum_addr });

    chan_i_row_relay_loop_k->addUpstream({ lc_k->loopVar, chan_i_row });
    chan_i_row_relay_loop_k->addDownstream({ chan_i_row_relay_loop_k_DGSF_LOOP });

    chan_i_row_relay_loop_k_DGSF_LOOP->addUpstream({ chan_i_row_relay_loop_k });
    chan_i_row_relay_loop_k_DGSF_LOOP->addDownstream({ chan_partialSum_addr });

    // loop j
    chan_m2_addr->addUpstream({ chan_k_row_DGSF_LOOP, lc_j->loopVar, chan_jj_relay_loop_k_DGSF_LOOP });
    chan_m2_addr->addDownstream({ chan_m2_addr_delay });

    chan_m2_addr_delay->addUpstream({ chan_m2_addr });
    chan_m2_addr_delay->addDownstream({ lse_ld_m2 });

    lse_ld_m2->addUpstream({ chan_m2_addr_delay });
    lse_ld_m2->addDownstream({ lse_ld_m2_DGSF_DAE });

    lse_ld_m2_DGSF_DAE->addUpstream({ lse_ld_m2 });
    lse_ld_m2_DGSF_DAE->addDownstream({ chan_mul });

    chan_m1_getData_DGSF_DAE_temp->addUpstream({ lc_j->loopVar, chan_m1_getData_DGSF_LOOP });
    chan_m1_getData_DGSF_DAE_temp->addDownstream({ chan_m1_getData_DGSF_DAE });

    chan_m1_getData_DGSF_DAE->addUpstream({ chan_m1_getData_DGSF_DAE_temp });
    chan_m1_getData_DGSF_DAE->addDownstream({ chan_mul });

    chan_mul->addUpstream({ chan_m1_getData_DGSF_DAE, lse_ld_m2_DGSF_DAE });
    chan_mul->addDownstream({ chan_mul_delay });

    chan_mul_delay->addUpstream({ chan_mul });
    chan_mul_delay->addDownstream({ chan_partialSum });

    chan_partialSum_addr->addUpstream({ /*chan_i_row*/ chan_i_row_relay_loop_k_DGSF_LOOP, lc_j->loopVar, chan_jj_relay_loop_k_DGSF_LOOP });
    chan_partialSum_addr->addDownstream({ chan_partialSum_addr_delay });

    chan_partialSum_addr_delay->addUpstream({ chan_partialSum_addr });
    chan_partialSum_addr_delay->addDownstream({ lse_ld_partialSum, lse_st_partialSum });

    lse_ld_partialSum->addUpstream({ chan_partialSum_addr_delay });
    lse_ld_partialSum->addDownstream({ chan_partialSum });

    chan_partialSum->addUpstream({ lse_ld_partialSum, chan_mul_delay });
    chan_partialSum->addDownstream({ lse_st_partialSum, lc_j->getEnd });

    lse_st_partialSum->addUpstream({ chan_partialSum_addr_delay, chan_partialSum });
    lse_st_partialSum->noDownstream = 1;


    //*** LC addPort : getAct, sendAct, getEnd, sendEnd
    lc_jj->addPort({ }, { chan_jj_lc }, { lc_kk->sendEnd }, { end });
    lc_jj->addDependence({ begin }, {});  // No loop dependence

    lc_kk->addPort({ chan_jj_lc }, { chan_kk_lc, chan_jj_relay_loop_kk }, { lc_i->sendEnd }, { lc_jj->getEnd });
    lc_kk->addDependence({}, {});  // No loop dependence

    lc_i->addPort({ chan_kk_lc }, { chan_i_lc, chan_i_row, chan_kk_relay_loop_i, chan_jj_relay_loop_i }, { lc_k->sendEnd }, { lc_kk->getEnd });
    lc_i->addDependence({}, {});  // No loop dependence

    lc_k->addPort({ chan_i_lc }, { chan_k_lc_DGSF_LOOP, chan_k_row, chan_m1_addr, chan_jj_relay_loop_k, chan_i_row_relay_loop_k }, { lc_j->sendEnd }, { lc_i->getEnd });
    lc_k->addDependence({}, {});  // No loop dependence

    lc_j->addPort({ chan_k_lc_DGSF_LOOP }, { chan_m2_addr, chan_partialSum_addr, chan_m1_getData_DGSF_DAE_temp }, { chan_partialSum }, { lc_k->getEnd });
    lc_j->addDependence({}, {});  // No loop dependence


    //*** Define subgraph
    GraphScheduler* graphScheduler = new GraphScheduler();
    // Subgraph 0
    graphScheduler->addSubgraph(0, {}, {chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP});
    // Subgraph 1
    graphScheduler->addSubgraph(1, { chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP }, { chan_m1_getData_DGSF_DAE, lse_ld_m2_DGSF_DAE });
    // Subgraph 2
    graphScheduler->addSubgraph(2, { chan_m1_getData_DGSF_DAE, lse_ld_m2_DGSF_DAE }, {});

    //GraphScheduler* graphScheduler = new GraphScheduler();
    //// Subgraph 0
    //graphScheduler->addSubgraph(0, {}, { chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP });
    //// Subgraph 1
    //graphScheduler->addSubgraph(1, { chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP }, { });

    // Set subgraphId
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            entry.chanPtr->subgraphId = 100;  // Initial a fake id
        }
    }

    lc_jj->loopVar->subgraphId = 0;
    chan_jj_lc->subgraphId = 0;
    lc_kk->loopVar->subgraphId = 0;
    chan_kk_lc->subgraphId = 0;
    chan_jj_relay_loop_kk->subgraphId = 0;
    lc_i->loopVar->subgraphId = 0;
    chan_i_lc->subgraphId = 0;
    chan_kk_relay_loop_i->subgraphId = 0;
    chan_i_row->subgraphId = 0;
    chan_jj_relay_loop_i->subgraphId = 0;
    lc_k->loopVar->subgraphId = 0;
    chan_m1_addr->subgraphId = 0;
    chan_m1_addr_delay->subgraphId = 0;
    lse_ld_m1->subgraphId = 0;
    chan_k_row->subgraphId = 0;
    chan_k_row_delay->subgraphId = 0;
    chan_jj_relay_loop_k->subgraphId = 0;
    chan_i_row_relay_loop_k->subgraphId = 0;

    lc_j->loopVar->subgraphId = 1;
    chan_m2_addr->subgraphId = 1;
    chan_partialSum_addr->subgraphId = 1;
    chan_m2_addr_delay->subgraphId = 1;
    chan_m1_getData_DGSF_DAE_temp->subgraphId = 1;
    lse_ld_m2->subgraphId = 1;

    chan_mul->subgraphId = 2;
    //chan_partialSum_addr_delay->subgraphId = 2;
    chan_mul_delay->subgraphId = 2;
    lse_ld_partialSum->subgraphId = 2;
    chan_partialSum->subgraphId = 2;
    lse_st_partialSum->subgraphId = 2;


    //// Debug_yin
    //GraphScheduler* graphScheduler = new GraphScheduler();
    //// Subgraph 0
    //graphScheduler->addSubgraph(0, {}, { chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP });
    //// Subgraph 1
    //graphScheduler->addSubgraph(1, { chan_k_lc_DGSF_LOOP, chan_jj_relay_loop_k_DGSF_LOOP, chan_i_row_relay_loop_k_DGSF_LOOP, chan_k_row_DGSF_LOOP, chan_m1_getData_DGSF_LOOP }, {});


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


    //*** Simulate
    // Initiation
    registry->init();  // Update registry and initial all the module in registry
    registry->pathBalance();
    profiler->init();
    watchdog.addCheckPointChan({ lc_jj_getEnd, lc_kk_getEnd, lc_i_getEnd, lc_k_getEnd, lc_j_getEnd });
    graphScheduler->schedulerInit();  // Initial graph scheduler

    begin->get({ 1 });
    uint64_t iter = 0;

    //int jj = 0;
    //int kk = 0;
    //int i = 0;
    //int k = 0;
    //int j = 0;

    vector<int> res;  // Result
    vector<int> temp; // temp_result

    uint64_t graphId = 0; // For debug
    vector<pair<uint64_t, uint64_t>> subgraphRecord;  // For debug

    uint64_t max_iter = /*53248;*/6888290;
    uint64_t segment = max_iter / 100;
    uint64_t percent = 0;
    uint64_t clk = 0;

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
        clk = DFSim::ClkDomain::getInstance()->getClk();
        debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
        debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

        //** Loop Lc_jj
        lc_jj->var = lc_jj->mux->mux(lc_jj->var, 0, lc_jj->sel);
        lc_jj->mux->muxUpdate(lc_jj->sel);
        lc_jj->mux->outChan->value = lc_jj->var;
        lc_jj->loopVar->get();
        lc_jj->loopVar->value = lc_jj->loopVar->assign(lc_jj->mux->outChan);  // After get(), must update chan's value
        lc_jj->var = lc_jj->loopVar->value + block_size;
        lc_jj->lcUpdate(lc_jj->var < matrix_height);

        // Clear begin
        begin->valid = 0;

        // loop interface: var jj
        chan_jj_lc->get();
        chan_jj_lc->value = chan_jj_lc->assign(lc_jj->loopVar);

        //** Loop Lc_kk
        lc_kk->var = lc_kk->mux->mux(lc_kk->var, 0, lc_kk->sel);
        lc_kk->mux->muxUpdate(lc_kk->sel);
        lc_kk->mux->outChan->value = lc_kk->var;
        lc_kk->loopVar->get();
        lc_kk->loopVar->value = lc_kk->loopVar->assign(lc_kk->mux->outChan);
        lc_kk->var = lc_kk->loopVar->assign(lc_kk->mux->outChan) + block_size;
        lc_kk->lcUpdate(lc_kk->var < matrix_height);
        //// Debug_yin_04.15
        //auto& _data2 = lc_kk->loopVar->channel.front();
        //if (_data2.last && _data2.lastOuter && lc_kk->loopVar->valid)
        //{
        //    std::cout << "Catch the last signal -> lc_kk->loopVar @ DataCnt = " << lc_kk->loopVar->chanDataCnt << std::endl;
        //}
        //auto dataCnt = lc_kk->loopVar->chanDataCnt;
        //if (dataCnt == 3)
        //{
        //    //std::cout << "Iter" << iter << std::endl;
        //    //system("pause");
        //}

        // loop interface: var kk
        chan_kk_lc->get();
        chan_kk_lc->value = chan_kk_lc->assign(lc_kk->loopVar);

        chan_jj_relay_loop_kk->get();
        chan_jj_relay_loop_kk->value = chan_jj_relay_loop_kk->assign(chan_jj_lc);
        // Debug_yin_04.15
        //auto& _data1 = chan_jj_relay_loop_kk->channel.front();
        //if (_data1.last && _data1.lastOuter && chan_jj_relay_loop_kk->valid)
        //{
        //    std::cout << "Catch the last signal -> chan_jj_relay_loop_kk" << std::endl;
        //}

        //** Loop Lc_i
        lc_i->var = lc_i->mux->mux(lc_i->var, 0, lc_i->sel);
        lc_i->mux->muxUpdate(lc_i->sel);
        lc_i->mux->outChan->value = lc_i->var;
        lc_i->loopVar->get();
        lc_i->loopVar->value = lc_i->loopVar->assign(lc_i->mux->outChan);
        lc_i->var = lc_i->loopVar->assign(lc_i->mux->outChan) + 1;
        lc_i->lcUpdate(lc_i->var < matrix_height);

        // loop interface: var i
        chan_i_lc->get();
        chan_i_lc->value = chan_i_lc->assign(lc_i->loopVar);

        chan_i_row->get();
        chan_i_row->value = chan_i_row->assign(lc_i->loopVar) * matrix_height;

        chan_kk_relay_loop_i->get();
        chan_kk_relay_loop_i->value = chan_kk_relay_loop_i->assign(chan_kk_lc);

        chan_jj_relay_loop_i->get();
        chan_jj_relay_loop_i->value = chan_jj_relay_loop_i->assign(chan_jj_relay_loop_kk);
        //// Debug_yin_04.15
        //auto& _data = chan_jj_relay_loop_i->channel.front();
        //if (_data.last && _data.lastOuter && chan_jj_relay_loop_i->valid)
        //{
        //    std::cout << "Catch the last signal -> chan_jj_relay_loop_i" << std::endl;
        //}

        //** Loop Lc_k
        lc_k->var = lc_k->mux->mux(lc_k->var, 0, lc_k->sel);
        lc_k->mux->muxUpdate(lc_k->sel);
        lc_k->mux->outChan->value = lc_k->var;
        lc_k->loopVar->get();
        lc_k->loopVar->value = lc_k->loopVar->assign(lc_k->mux->outChan);
        lc_k->var = lc_k->loopVar->assign(lc_k->mux->outChan) + 1;
        lc_k->lcUpdate(lc_k->var < block_size);

        // loop interface: var k
        chan_k_lc_DGSF_LOOP->get();
        chan_k_lc_DGSF_LOOP->value = chan_k_lc_DGSF_LOOP->assign(lc_k->loopVar);

        chan_k_row->get();
        chan_k_row->value = (chan_k_row->assign(lc_k->loopVar) + chan_k_row->assign(chan_kk_relay_loop_i)) * matrix_width;

        chan_k_row_delay->get();
        chan_k_row_delay->value = chan_k_row_delay->assign(chan_k_row);

        chan_k_row_DGSF_LOOP->get();
        chan_k_row_DGSF_LOOP->value = chan_k_row_DGSF_LOOP->assign(chan_k_row_delay);

        chan_m1_addr->get();
        chan_m1_addr->value = chan_m1_addr->assign(chan_i_row) + chan_m1_addr->assign(lc_k->loopVar) + chan_m1_addr->assign(chan_kk_relay_loop_i) + m1_BaseAddr;

        chan_m1_addr_delay->get();
        chan_m1_addr_delay->value = chan_m1_addr_delay->assign(chan_m1_addr);

        lse_ld_m1->get();
        lse_ld_m1->value = lse_ld_m1->assign() - m1_BaseAddr;

        chan_m1_getData_DGSF_LOOP->get();
        uint64_t m1_rowId = chan_m1_getData_DGSF_LOOP->assign(lse_ld_m1) / matrix_width;
        uint64_t m1_colId = chan_m1_getData_DGSF_LOOP->assign(lse_ld_m1) % matrix_width;
        int m1_data = m1[m1_rowId][m1_colId];
        chan_m1_getData_DGSF_LOOP->value = m1_data;

        chan_jj_relay_loop_k->get();
        chan_jj_relay_loop_k->value = chan_jj_relay_loop_k->assign(chan_jj_relay_loop_i);

        chan_jj_relay_loop_k_DGSF_LOOP->get();
        chan_jj_relay_loop_k_DGSF_LOOP->value = chan_jj_relay_loop_k_DGSF_LOOP->assign(chan_jj_relay_loop_k);

        chan_i_row_relay_loop_k->get();
        chan_i_row_relay_loop_k->value = chan_i_row_relay_loop_k->assign(chan_i_row);

        chan_i_row_relay_loop_k_DGSF_LOOP->get();
        chan_i_row_relay_loop_k_DGSF_LOOP->value = chan_i_row_relay_loop_k_DGSF_LOOP->assign(chan_i_row_relay_loop_k);

        //** Loop Lc_j
        lc_j->var = lc_j->mux->mux(lc_j->var, 0, lc_j->sel);
        lc_j->mux->muxUpdate(lc_j->sel);
        lc_j->mux->outChan->value = lc_j->var;
        lc_j->loopVar->get();
        lc_j->loopVar->value = lc_j->loopVar->assign(lc_j->mux->outChan);
        lc_j->var = lc_j->loopVar->assign(lc_j->mux->outChan) + 1;
        lc_j->lcUpdate(lc_j->var < block_size);

        // loop interface: var j
        chan_m2_addr->get();
        chan_m2_addr->value = chan_m2_addr->assign(chan_k_row_DGSF_LOOP) + chan_m2_addr->assign(lc_j->loopVar) + chan_m2_addr->assign(chan_jj_relay_loop_k_DGSF_LOOP) + m2_BaseAddr;

        chan_m2_addr_delay->get();
        chan_m2_addr_delay->value = chan_m2_addr_delay->assign(chan_m2_addr);

        lse_ld_m2->get();
        lse_ld_m2->value = lse_ld_m2->assign() - m2_BaseAddr;

        lse_ld_m2_DGSF_DAE->get();
        lse_ld_m2_DGSF_DAE->value = lse_ld_m2_DGSF_DAE->assign(lse_ld_m2);

        chan_m1_getData_DGSF_DAE_temp->get();
        chan_m1_getData_DGSF_DAE_temp->value = chan_m1_getData_DGSF_DAE_temp->assign(chan_m1_getData_DGSF_LOOP);

        chan_m1_getData_DGSF_DAE->get();
        chan_m1_getData_DGSF_DAE->value = chan_m1_getData_DGSF_DAE->assign(chan_m1_getData_DGSF_DAE_temp);

        chan_mul->get();
        uint64_t m2_addr_ack = chan_mul->assign(lse_ld_m2_DGSF_DAE);
        uint64_t m2_rowId = m2_addr_ack / matrix_width;
        uint64_t m2_colId = m2_addr_ack % matrix_width;
        int m2_data = m2[m2_rowId][m2_colId];
        chan_mul->value = chan_mul->assign(chan_m1_getData_DGSF_DAE) * m2_data;

        chan_mul_delay->get();
        chan_mul_delay->value = chan_mul_delay->assign(chan_mul);

        chan_partialSum_addr->get();
        chan_partialSum_addr->value = chan_partialSum_addr->assign(chan_i_row_relay_loop_k_DGSF_LOOP) + chan_partialSum_addr->assign(lc_j->loopVar) + chan_partialSum_addr->assign(chan_jj_relay_loop_k_DGSF_LOOP) + partialSum_BaseAddr;

        chan_partialSum_addr_delay->get();
        chan_partialSum_addr_delay->value = chan_partialSum_addr_delay->assign(chan_partialSum_addr);

        lse_ld_partialSum->get();
        lse_ld_partialSum->value = lse_ld_partialSum->assign() - partialSum_BaseAddr;

        chan_partialSum->get();
        uint64_t partialSum_addr_ack = chan_partialSum->assign(lse_ld_partialSum);
        uint64_t partialSum_rowId = partialSum_addr_ack / matrix_width;
        uint64_t partialSum_colId = partialSum_addr_ack % matrix_width;
        int partialSum_data = m2[partialSum_rowId][partialSum_colId];
        chan_partialSum->value = chan_partialSum->assign(chan_mul_delay) + partialSum_data;

        temp = lse_st_partialSum->get();

        if (temp[0])
        {
            res.push_back(chan_partialSum->value);
        }

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** GraphScheduler update
        graphScheduler->graphUpdate();

        //** Profiler update
        profiler->updateBufferMaxDataNum();
        profiler->updateChanUtilization(graphScheduler->currSubgraphId);

        if (graphId != graphScheduler->currSubgraphId)
        {
            graphId = graphScheduler->currSubgraphId;
            subgraphRecord.push_back(std::make_pair(graphId, clk));
        }

        //// Debug_yin
        //if (graphId != graphScheduler->currSubgraphId)
        //{
        //    debug->debug_mode = Debug_mode::Print_brief;
        //    debug->getFile() << "graphId: " << graphId << "\tclk: " << clk << std::endl;
        //    graphId = graphScheduler->currSubgraphId;
        //}

        end->get();

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_brief;
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (/*53248 > iter && iter > 51248*/ iter >= 0)
        {
            //std::cout << "table_size:" << graphScheduler->subgraphTable.size() << std::endl;

            debug->getFile() << std::endl;
            debug->getFile() << "Loop index jj: " << chan_jj_relay_loop_k->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index kk: " << chan_kk_relay_loop_i->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;
            debug->getFile() << "Loop index k: " << chan_k_lc_DGSF_LOOP->value << std::endl;
            debug->getFile() << "Loop index j: " << lc_j->loopVar->value << std::endl;

            debug->vecPrint("Result", res, 15);

            debug->chanPrint("begin", begin);

            // loop jj
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  jj  *****************" << std::endl;
            debug->chanPrint("lc_jj->loopVar", lc_jj->loopVar);
            debug->chanPrint("chan_jj_lc", chan_jj_lc);
            // loop kk
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  kk  *****************" << std::endl;
            debug->chanPrint("lc_kk->loopVar", lc_kk->loopVar);
            debug->chanPrint("chan_kk_lc", chan_kk_lc);
            debug->chanPrint("chan_jj_relay_loop_kk", chan_jj_relay_loop_kk);
            // loop i
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  i  *****************" << std::endl;
            debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
            debug->chanPrint("chan_i_lc", chan_i_lc);
            debug->chanPrint("chan_i_row", chan_i_row);
            debug->chanPrint("chan_jj_relay_loop_i", chan_jj_relay_loop_i);
            debug->chanPrint("chan_kk_relay_loop_i", chan_kk_relay_loop_i);
            // loop k
            debug->getFile() << std::endl;
            debug->getFile() << "Current subgraphId: " << graphScheduler->currSubgraphId  << "\t" << " Clk:" << clk << std::endl;
            debug->getFile() << "*****************  k  *****************" << std::endl;
            debug->chanPrint("lc_k->loopVar", lc_k->loopVar);

            debug->chanPrint("chan_k_lc_DGSF_LOOP", chan_k_lc_DGSF_LOOP);
            debug->chanPrint("chan_m1_addr", chan_m1_addr);
            debug->chanPrint("chan_m1_addr_delay", chan_m1_addr_delay);
            debug->chanPrint("chan_k_row", chan_k_row);
            debug->chanPrint("chan_k_row_delay", chan_k_row_delay);
            debug->lsePrint("lse_ld_m1", lse_ld_m1);
            debug->chanPrint("chan_jj_relay_loop_k", chan_jj_relay_loop_k);
            debug->chanPrint("chan_i_row_relay_loop_k", chan_i_row_relay_loop_k);
            debug->getFile() << "Current subgraphId: " << graphScheduler->currSubgraphId << "\t" << " Clk:" << clk << std::endl;

            debug->chanPrint("chan_k_row_DGSF_LOOP", chan_k_row_DGSF_LOOP);
            debug->chanPrint("chan_m1_getData_DGSF_LOOP", chan_m1_getData_DGSF_LOOP);
            debug->chanPrint("chan_jj_relay_loop_k_DGSF_LOOP", chan_jj_relay_loop_k_DGSF_LOOP);
            debug->chanPrint("chan_i_row_relay_loop_k_DGSF_LOOP", chan_i_row_relay_loop_k_DGSF_LOOP);
            // loop j
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  j  *****************" << std::endl;
            debug->chanPrint("lc_j->loopVar", lc_j->loopVar);
            debug->chanPrint("chan_m2_addr", chan_m2_addr);
            debug->chanPrint("chan_m2_addr_delay", chan_m2_addr_delay);
            debug->lsePrint("lse_ld_m2", lse_ld_m2);
            debug->getFile() << "Current subgraphId: " << graphScheduler->currSubgraphId << "\t" << " Clk:" << clk << std::endl;

            auto subgraphStatus = graphScheduler->getSubgraphStatus();
            debug->getFile() << "Subgraph isOver: ";
            for (auto& status : subgraphStatus)
            {
                debug->getFile() << status << " ";
            }
            debug->getFile() << std::endl;

            debug->chanPrint("lse_ld_m2_DGSF_DAE", lse_ld_m2_DGSF_DAE);
            debug->chanPrint("chan_m1_getData_DGSF_DAE_temp", chan_m1_getData_DGSF_DAE_temp);
            debug->chanPrint("chan_m1_getData_DGSF_DAE", chan_m1_getData_DGSF_DAE);
            debug->chanPrint("chan_mul", chan_mul);
            debug->chanPrint("chan_mul_delay", chan_mul_delay);
            debug->chanPrint("chan_partialSum_addr", chan_partialSum_addr);
            debug->chanPrint("chan_partialSum_addr_delay", chan_partialSum_addr_delay);
            debug->lsePrint("lse_ld_partialSum", lse_ld_partialSum);
            debug->chanPrint("chan_partialSum", chan_partialSum);
            debug->lsePrint("lse_st_partialSum", lse_st_partialSum);

            debug->getFile() << std::endl;
            debug->getFile() << "*****************  End signal  *****************" << std::endl;

            debug->chanPrint("lc_jj->getEnd", lc_jj->getEnd);
            debug->getFile() << "lc_jj loopEnd: " << lc_jj->loopEnd << std::endl;
            //debug->chanPrint("lc_jj->sendEnd", lc_jj->sendEnd);

            debug->chanPrint("lc_kk->getEnd", lc_kk->getEnd);
            debug->getFile() << "lc_kk loopEnd: " << lc_kk->loopEnd << std::endl;
            //debug->chanPrint("lc_kk->sendEnd", lc_kk->sendEnd);

            debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
            debug->getFile() << "lc_i loopEnd: " << lc_i->loopEnd << std::endl;
            //debug->chanPrint("lc_i->sendEnd", lc_i->sendEnd);

            debug->chanPrint("lc_k->getEnd", lc_k->getEnd);
            debug->getFile() << "lc_k loopEnd: " << lc_k->loopEnd << std::endl;
            //debug->chanPrint("lc_k->sendEnd", lc_k->sendEnd);

            debug->chanPrint("lc_j->getEnd", lc_j->getEnd);
            debug->getFile() << "lc_j loopEnd: " << lc_j->loopEnd << std::endl;
            //debug->chanPrint("lc_j->sendEnd", lc_j->sendEnd);

            debug->chanPrint("end", end);

            // Print MemorySystem
            //debug->memSysPrint(memSys);
        }

        if (!end->channel.empty())
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

    debug->debug_mode = Debug_mode::Print_detail;

    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Profiling" << std::endl;
    debug->getFile() << "*******************************" << std::endl;

    debug->getFile() << "******* Channel Utilization *********" << std::endl;

    uint64_t chanNum = 0;
    uint64_t avgWeight = 0;
    float chanUtilAvg = 0;
    uint64_t chanActiveNumTotal = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->subgraphId != 100)
        {
            uint64_t activeNum = entry.chanPtr->activeCnt;
            float utilization = std::min(static_cast<float>(activeNum) / static_cast<float>((entry.chanPtr->speedup * entry.chanPtr->activeClkCnt)) * 100, float(100));
            //debug->getFile() << "ChanName: " << entry.chanPtr->moduleName << "\t" << std::fixed << utilization << std::setprecision(2) << "%" << std::endl;

            // TODO: Exclude channel in "Nop"
            if (entry.chanPtr->moduleName != "Chan_begin"
                && entry.chanPtr->moduleName != "Chan_end"
                //&& (entry.chanPtr->masterName == "None" || entry.chanPtr->isLoopVar)
                && (entry.chanPtr->keepMode != 1 && entry.chanPtr->drainMode != 1))
            {
                debug->getFile() << "ChanName: " << entry.chanPtr->moduleName << "\t" << std::fixed << utilization << std::setprecision(2) << "%" 
                    << "\t" << entry.chanPtr->subgraphId 
                    << "\t" << entry.chanPtr->activeCnt
                    << "\t" << entry.chanPtr->activeClkCnt << std::endl;
                ++chanNum;
                avgWeight += entry.chanPtr->activeClkCnt;
                chanUtilAvg += utilization * entry.chanPtr->activeClkCnt;

                chanActiveNumTotal += activeNum;
            }
        }
    }

    debug->getFile() << std::endl;
    debug->getFile() << "Avg channel utilization: " << std::fixed << chanUtilAvg / (/*chanNum * */avgWeight) << std::setprecision(2) << "%" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << "******* ALU/Reg Access Times *********" << std::endl;
    debug->getFile() << "Total ALU Active Times: " << chanActiveNumTotal << std::endl;
    debug->getFile() << "Total Reg Access Times: " << chanActiveNumTotal * 3 << std::endl;

    profiler->printBufferMaxDataNum("chan_i_lc", chan_i_lc);
    profiler->printBufferMaxDataNum("chan_i_row", chan_i_row);
    profiler->printBufferMaxDataNum("chan_jj_relay_loop_i", chan_jj_relay_loop_i);
    profiler->printBufferMaxDataNum("chan_kk_relay_loop_i", chan_kk_relay_loop_i);
    // loop k
    profiler->printBufferMaxDataNum("chan_k_lc_DGSF_LOOP", chan_k_lc_DGSF_LOOP);
    //profiler->printBufferMaxDataNum("chan_m1_addr", chan_m1_addr);
    //profiler->printBufferMaxDataNum("chan_m1_addr_delay", chan_m1_addr_delay);
    //profiler->printBufferMaxDataNum("chan_k_row", chan_k_row);
    //profiler->printBufferMaxDataNum("chan_k_row_delay", chan_k_row_delay);
    profiler->printBufferMaxDataNum("chan_k_row_DGSF_LOOP", chan_k_row_DGSF_LOOP);
    //debug->lsePrint("lse_ld_m1", lse_ld_m1);
    profiler->printBufferMaxDataNum("chan_m1_getData_DGSF_LOOP", chan_m1_getData_DGSF_LOOP);
    //profiler->printBufferMaxDataNum("chan_jj_relay_loop_k", chan_jj_relay_loop_k);
    profiler->printBufferMaxDataNum("chan_jj_relay_loop_k_DGSF_LOOP", chan_jj_relay_loop_k_DGSF_LOOP);
    //profiler->printBufferMaxDataNum("chan_i_row_relay_loop_k", chan_i_row_relay_loop_k);
    profiler->printBufferMaxDataNum("chan_i_row_relay_loop_k_DGSF_LOOP", chan_i_row_relay_loop_k_DGSF_LOOP);
    // loop j
    profiler->printBufferMaxDataNum("chan_m2_addr", chan_m2_addr);
    profiler->printBufferMaxDataNum("chan_m2_addr_delay", chan_m2_addr_delay);
    profiler->printBufferMaxDataNum("lse_ld_m2", lse_ld_m2);
    profiler->printBufferMaxDataNum("lse_ld_m2_DGSF_DAE", lse_ld_m2_DGSF_DAE);
    profiler->printBufferMaxDataNum("chan_m1_getData_DGSF_DAE_temp", chan_m1_getData_DGSF_DAE_temp);
    profiler->printBufferMaxDataNum("chan_m1_getData_DGSF_DAE", chan_m1_getData_DGSF_DAE);
    profiler->printBufferMaxDataNum("chan_mul", chan_mul);
    profiler->printBufferMaxDataNum("chan_mul_delay", chan_mul_delay);
    profiler->printBufferMaxDataNum("chan_partialSum_addr", chan_partialSum_addr);
    profiler->printBufferMaxDataNum("chan_partialSum_addr_delay", chan_partialSum_addr_delay);
    profiler->printBufferMaxDataNum("lse_ld_partialSum", lse_ld_partialSum);
    profiler->printBufferMaxDataNum("chan_partialSum", chan_partialSum);
    profiler->printBufferMaxDataNum("lse_st_partialSum", lse_st_partialSum);


    //*** Record subgraph switch
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Subgraph switch record: " << std::endl;
    debug->getFile() << std::endl;
    uint64_t deltaTime = 0;
    for (size_t i = 0; i < subgraphRecord.size(); ++i)
    {
        auto item = subgraphRecord[i];
        if (i < subgraphRecord.size() - 1)
        {
            deltaTime = subgraphRecord[i + 1].second - subgraphRecord[i].second;
        }
        else
        {
            deltaTime = 0;
        }
        debug->getFile() << "Cnt: " << i+1 << "\tId: " << item.first << "\t@clk: " << item.second << "\t\tDelta: " << deltaTime << std::endl;
    }


    //*** Print Lse access 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Lse profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printLseProfiling("lse_ld_m1", lse_ld_m1);
    profiler->printLseProfiling("lse_ld_m2", lse_ld_m2);
    profiler->printLseProfiling("lse_ld_partialSum", lse_ld_partialSum);
    profiler->printLseProfiling("lse_st_partialSum", lse_st_partialSum);

    //*** Print cache 
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Cache miss rate: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printCacheMissRate();

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Total Cycle: " << clk << std::endl;
    debug->getFile() << "Total run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;


    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
    delete graphScheduler;
    delete profiler;
}
