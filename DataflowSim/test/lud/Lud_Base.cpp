#include "./Lud.h"

using namespace DFSimTest;

void LudTest::lud_Base(Debug* debug)
{
    //******  Define module  ******//
    //*** Declare registry
    Registry* registry = new Registry();

    //*** Declare memory
    MemSystem* memSys = new MemSystem();


    //*** Declare Lse
    Lse* lse_ld_aii = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[i][i]
    Lse* lse_ld_aij = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[i][j]
    Lse* lse_ld_aik = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[i][k]
    Lse* lse_ld_akj = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[k][j]
    Lse* lse_st_aij = new Lse(LSE_QUEUE_SIZE, 0, true, memSys, 1);  // Store a[i][j]

    Lse* lse_ld_aji = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[j][i]
    Lse* lse_ld_ajk = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[j][k]
    Lse* lse_ld_aki = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[k][i]
    Lse* lse_st_aji = new Lse(LSE_QUEUE_SIZE, 0, true, memSys, 1);  // Store a[j][i]


    //*** Declare Lc
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
    // Set outer-most loop
    lc_i->isOuterMostLoop = 1;

    // Loop j1
    ChanBase* lc_j1_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_j1_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_j1_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_j1_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j1_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j1_mux_outChan = new ChanBase(2, 0, 1);

    ChanBase* chan_j1_lc = new ChanBase(2, 0, 1);
    chan_j1_lc->keepMode = 1;

    Mux* lc_j1_mux = new Mux(lc_j1_mux_trueChan, lc_j1_mux_falseChan, lc_j1_mux_outChan);
    lc_j1_mux->addPort({ lc_j1_loopVar }, { }, { lc_j1_loopVar });
    Lc* lc_j1 = new Lc(lc_j1_loopVar, lc_j1_getEnd, lc_j1_sendEnd, lc_j1_mux);

    // Loop j2
    ChanBase* lc_j2_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_j2_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_j2_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_j2_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j2_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_j2_mux_outChan = new ChanBase(2, 0, 1);

    ChanBase* chan_j2_lc = new ChanBase(2, 0, 1);
    chan_j2_lc->keepMode = 1;

    Mux* lc_j2_mux = new Mux(lc_j2_mux_trueChan, lc_j2_mux_falseChan, lc_j2_mux_outChan);
    lc_j2_mux->addPort({ lc_j2_loopVar }, { }, { lc_j2_loopVar });
    Lc* lc_j2 = new Lc(lc_j2_loopVar, lc_j2_getEnd, lc_j2_sendEnd, lc_j2_mux);

    // Loop k1
    ChanBase* lc_k1_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_k1_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_k1_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_k1_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k1_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k1_mux_outChan = new ChanBase(2, 0, 1);

    //ChanBase* chan_k1_lc = new ChanBase(2, 0, 1);
    //chan_k1_lc->keepMode = 1;

    Mux* lc_k1_mux = new Mux(lc_k1_mux_trueChan, lc_k1_mux_falseChan, lc_k1_mux_outChan);
    lc_k1_mux->addPort({ lc_k1_loopVar }, { }, { lc_k1_loopVar });
    Lc* lc_k1 = new Lc(lc_k1_loopVar, lc_k1_getEnd, lc_k1_sendEnd, lc_k1_mux);

    // Loop k2
    ChanBase* lc_k2_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_k2_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_k2_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_k2_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k2_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_k2_mux_outChan = new ChanBase(2, 0, 1);

    //ChanBase* chan_k2_lc = new ChanBase(2, 0, 1);
    //chan_k2_lc->keepMode = 1;

    Mux* lc_k2_mux = new Mux(lc_k2_mux_trueChan, lc_k2_mux_falseChan, lc_k2_mux_outChan);
    lc_k2_mux->addPort({ lc_k2_loopVar }, { }, { lc_k2_loopVar });
    Lc* lc_k2 = new Lc(lc_k2_loopVar, lc_k2_getEnd, lc_k2_sendEnd, lc_k2_mux);


    //*** Declare channel
    ChanBase* begin = new ChanBase(1, 0);
    begin->noUpstream = 1;
    ChanBase* end = new ChanBase(1, 0);
    end->noDownstream = 1;

    // loop j1
    ChanBase* chan_i_relay_loop_j1 = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Relay channel in loop j1 for chan_i_lc
    chan_i_relay_loop_j1->keepMode = 1;

    ChanBase* chan_aij_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);

    ChanBase* chan_sum_loop_j1 = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);
    chan_sum_loop_j1->keepMode = 1;

    // loop j2
    ChanBase* chan_i_relay_loop_j2 = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Relay channel in loop j2 for chan_i_lc
    chan_i_relay_loop_j2->keepMode = 1;

    ChanBase* chan_aji_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);

    ChanBase* chan_sum_loop_j2 = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);
    chan_sum_loop_j2->keepMode = 1;

    ChanBase* chan_aii_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);

    ChanBase* chan_sum_div = new ChanBase(BASE_INPUT_BUFF_SIZE, DIV, 1);

    // loop k1
    ChanBase* chan_aik_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);
    ChanBase* chan_akj_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);
    ChanBase* chan_sum_loop_k1 = new ChanBase(BASE_INPUT_BUFF_SIZE, MUL + SUB, 1);

    // loop k2
    ChanBase* chan_ajk_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);
    ChanBase* chan_aki_addr = new ChanBase(BASE_INPUT_BUFF_SIZE, ADD + MUL, 1);
    ChanBase* chan_sum_loop_k2 = new ChanBase(BASE_INPUT_BUFF_SIZE, MUL + SUB, 1);


    //*** Define interconnect
    // Begin & end connect to the outer most loop
    begin->addDownstream({ lc_i_mux_falseChan });
    end->addUpstream({ lc_i->sendEnd });

    // Loop index channel
    chan_i_lc->addUpstream({ lc_i->loopVar });
    chan_i_lc->addDownstream({ lc_j1->loopVar, lc_j2->loopVar, chan_i_relay_loop_j1, chan_i_relay_loop_j2, chan_aij_addr, chan_aji_addr, chan_aii_addr });

    chan_j1_lc->addUpstream({ lc_j1->loopVar });
    chan_j1_lc->addDownstream({ lc_k1->loopVar, chan_akj_addr });

    chan_j2_lc->addUpstream({ lc_j2->loopVar });
    chan_j2_lc->addDownstream({ lc_k2->loopVar, chan_ajk_addr });

    // loop j1
    chan_i_relay_loop_j1->addUpstream({ lc_j1->loopVar, chan_i_lc });
    chan_i_relay_loop_j1->addDownstream({ chan_aik_addr });

    chan_aij_addr->addUpstream({ lc_j1->loopVar, chan_i_lc });
    chan_aij_addr->addDownstream({ lse_ld_aij, lse_st_aij });

    lse_ld_aij->addUpstream({ chan_aij_addr });
    lse_ld_aij->addDownstream({ chan_sum_loop_j1 });

    chan_sum_loop_j1->addUpstream({ lse_ld_aij });
    chan_sum_loop_j1->addDownstream({ chan_sum_loop_k1 });

    lse_st_aij->addUpstream({ chan_aij_addr, chan_sum_loop_k1 });
    lse_st_aij->noDownstream = 1;

    // loop j2
    chan_i_relay_loop_j2->addUpstream({ lc_j2->loopVar, chan_i_lc });
    chan_i_relay_loop_j2->addDownstream({ chan_aki_addr });

    chan_aii_addr ->addUpstream({ chan_i_lc });
    chan_aii_addr->addDownstream({ lse_ld_aii });

    lse_ld_aii->addUpstream({ chan_aii_addr });
    lse_ld_aii->addDownstream({ chan_sum_div });

    chan_aji_addr->addUpstream({ lc_j2->loopVar, chan_i_lc });
    chan_aji_addr->addDownstream({ lse_ld_aji, lse_st_aji });

    lse_ld_aji->addUpstream({ chan_aji_addr });
    lse_ld_aji->addDownstream({ chan_sum_loop_j2 });

    chan_sum_loop_j2->addUpstream({ lse_ld_aji });
    chan_sum_loop_j2->addDownstream({ chan_sum_loop_k2 });

    chan_sum_div->addUpstream({ chan_sum_loop_k2, lse_ld_aii });
    chan_sum_div->addDownstream({ lse_st_aji });

    lse_st_aji->addUpstream({ chan_aji_addr, chan_sum_div });
    lse_st_aji->noDownstream = 1;

    // loop k1
    chan_akj_addr->addUpstream({ lc_k1->loopVar, chan_j1_lc });
    chan_akj_addr->addDownstream({ lse_ld_akj });

    lse_ld_akj->addUpstream({ chan_akj_addr });
    lse_ld_akj->addDownstream({ chan_sum_loop_k1 });

    chan_aik_addr->addUpstream({ lc_k1->loopVar, chan_i_relay_loop_j1 });
    chan_aik_addr->addDownstream({ lse_ld_aik });

    lse_ld_aik->addUpstream({ chan_aik_addr });
    lse_ld_aik->addDownstream({ chan_sum_loop_k1 });

    chan_sum_loop_k1->addUpstream({ lse_ld_aik, lse_ld_akj, chan_sum_loop_j1 });
    chan_sum_loop_k1->addDownstream({ lse_st_aij });

    // loop k2
    chan_ajk_addr->addUpstream({ lc_k2->loopVar, chan_j2_lc });
    chan_ajk_addr->addDownstream({ lse_ld_ajk });

    lse_ld_ajk->addUpstream({ chan_ajk_addr });
    lse_ld_ajk->addDownstream({ chan_sum_loop_k2 });

    chan_aki_addr->addUpstream({ lc_k2->loopVar, chan_i_relay_loop_j2 });
    chan_aki_addr->addDownstream({ lse_ld_aki });

    lse_ld_aki->addUpstream({ chan_aki_addr });
    lse_ld_aki->addDownstream({ chan_sum_loop_k2 });

    chan_sum_loop_k2->addUpstream({ lse_ld_ajk, lse_ld_aki, chan_sum_loop_j2 });
    chan_sum_loop_k2->addDownstream({ lse_st_aij });


    //*** LC addPort : getAct, sendAct, getEnd, sendEnd
    lc_i->addPort({ }, { chan_i_lc }, { lc_j2->sendEnd }, { end });
    lc_i->addDependence({ begin }, {});  // No loop dependence

    lc_j1->addPort({ chan_i_lc }, { chan_j1_lc, chan_i_relay_loop_j1, chan_aij_addr }, { lse_st_aij }, { /*lc_i->getEnd*/ });
    lc_j1->addDependence({}, {});  // No loop dependence

    lc_j2->addPort({ chan_i_lc }, { chan_j2_lc, chan_i_relay_loop_j2, chan_aji_addr }, { lse_st_aji }, { lc_i->getEnd });
    lc_j2->addDependence({}, {});  // No loop dependence

    lc_k1->addPort({ chan_j1_lc }, { chan_akj_addr, chan_aik_addr }, { chan_sum_loop_k1 }, { /*lc_j1->getEnd*/ });
    lc_k1->addDependence({}, {});  // No loop dependence

    lc_k2->addPort({ chan_j2_lc }, { chan_ajk_addr, chan_aki_addr }, { chan_sum_loop_k2 }, { /*lc_j2->getEnd*/ });
    lc_k2->addDependence({}, {});  // No loop dependence


    //*** Simulate
    // Initiation
    registry->tableInit();  // Update registry and initial all the module in registry

    begin->get({ 1 });
    uint iter = 0;

    int i = 0;
    int j1 = 0;
    int j2 = 0;
    int k1 = 0;
    int k2 = 0;

    //vector<int> res;  // Result
    //vector<int> temp; // temp_result

    uint max_iter = 500000;
    uint segment = max_iter / 100;
    uint percent = 0;
    // Execute
    while (iter < max_iter)
    {
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

        //** Loop Lc_i
        i = lc_i->mux->mux(i, 0, lc_i->sel);
        lc_i->mux->muxUpdate(lc_i->sel);
        lc_i->mux->outChan->value = i;
        lc_i->loopVar->get();
        lc_i->loopVar->value = lc_i->loopVar->assign(lc_i->mux->outChan);  // After get(), must update chan's value
        i = lc_i->loopVar->value + 1;
        lc_i->lcUpdate(i < matrix_size);

        // Clear begin
        begin->valid = 0;

        // loop interface: var i
        chan_i_lc->get();
        chan_i_lc->value = chan_i_lc->assign(lc_i->loopVar);

        //** Loop Lc_j1
        if (lc_j1->mux->sel == 0)
        {
            j1 = chan_i_lc->value;  // Inductive loop variable
        }
        j1 = lc_j1->mux->mux(j1, 0, lc_j1->sel);
        lc_j1->mux->muxUpdate(lc_j1->sel);
        lc_j1->mux->outChan->value = j1;
        lc_j1->loopVar->get();
        lc_j1->loopVar->value = lc_j1->loopVar->assign(lc_j1->mux->outChan);
        j1 = lc_j1->loopVar->assign(lc_j1->mux->outChan) + 1;
        lc_j1->lcUpdate(j1 < matrix_size);

        // loop interface: var j1
        chan_j1_lc->get();
        chan_j1_lc->value = chan_j1_lc->assign(lc_j1->loopVar);

        chan_i_relay_loop_j1->get();
        chan_i_relay_loop_j1->value = chan_i_relay_loop_j1->assign(chan_i_lc);

        chan_aij_addr->get();
        chan_aij_addr->value = chan_aij_addr->assign(chan_i_lc) * matrix_size + chan_aij_addr->assign(lc_j1->loopVar);

        lse_ld_aij->get();
        lse_ld_aij->value = lse_ld_aij->assign();

        chan_sum_loop_j1->get();
        chan_sum_loop_j1->value = matrix[chan_sum_loop_j1->assign(lse_ld_aij)];

        //** Loop Lc_k1
        k1 = lc_k1->mux->mux(k1, 0, lc_k1->sel);
        lc_k1->mux->muxUpdate(lc_k1->sel);
        lc_k1->mux->outChan->value = k1;
        lc_k1->loopVar->get();
        lc_k1->loopVar->value = lc_k1->loopVar->assign(lc_k1->mux->outChan);
        k1 = lc_k1->loopVar->assign(lc_k1->mux->outChan) + 1;
        lc_k1->lcUpdate(k1 < chan_i_lc->value);

        // loop interface: var k1
        chan_akj_addr->get();
        chan_akj_addr->value = chan_akj_addr->assign(lc_k1->loopVar) * matrix_size + chan_akj_addr->assign(chan_j1_lc);

        chan_aik_addr->get();
        chan_aik_addr->value = chan_aik_addr->assign(chan_i_relay_loop_j1) * matrix_size + chan_aik_addr->assign(lc_k1->loopVar);

        lse_ld_akj->get();
        lse_ld_akj->value = lse_ld_akj->assign();

        lse_ld_aik->get();
        lse_ld_aik->value = lse_ld_aik->assign();

        chan_sum_loop_k1->get();
        chan_sum_loop_k1->value -= matrix[chan_sum_loop_k1->assign(lse_ld_akj)] * matrix[chan_sum_loop_k1->assign(lse_ld_aik)];

        //** loop Lc_j1 tail
        lse_st_aij->get();
        lse_st_aij->value = lse_st_aij->assign();
        if (!lse_st_aji->chanBuffer[0].empty() && !lse_st_aji->chanBuffer[1].empty())
        {
            int addr_aji = lse_st_aji->chanBuffer[0].front().value;
            int data_aji = lse_st_aji->chanBuffer[1].front().value;
            matrix[addr_aji] = data_aji;
        }

        //** Loop Lc_j2
        if (lc_j2->mux->sel == 0)
        {
            j2 = chan_i_lc->value + 1;  // Inductive loop variable
        }
        j2 = lc_j2->mux->mux(j2, 0, lc_j2->sel);
        lc_j2->mux->muxUpdate(lc_j2->sel);
        lc_j2->mux->outChan->value = j2;
        lc_j2->loopVar->get();
        lc_j2->loopVar->value = lc_j2->loopVar->assign(lc_j2->mux->outChan);
        j2 = lc_j2->loopVar->assign(lc_j2->mux->outChan) + 1;
        lc_j2->lcUpdate(j2 < matrix_size);

        // loop interface: var j2
        chan_j2_lc->get();
        chan_j2_lc->value = chan_j2_lc->assign(lc_j2->loopVar);

        chan_i_relay_loop_j2->get();
        chan_i_relay_loop_j2->value = chan_i_relay_loop_j2->assign(chan_i_lc);

        chan_aji_addr->get();
        chan_aji_addr->value = chan_aji_addr->assign(lc_j2->loopVar) * matrix_size + chan_aij_addr->assign(chan_i_lc);

        lse_ld_aji->get();
        lse_ld_aji->value = lse_ld_aji->assign();

        chan_sum_loop_j2->get();
        chan_sum_loop_j2->value = matrix[chan_sum_loop_j2->assign(lse_ld_aji)];

        chan_aii_addr->get();
        chan_aii_addr->value = chan_aii_addr->assign(chan_i_lc);

        lse_ld_aii->get();
        lse_ld_aii->value = lse_ld_aii->assign();

        //** Loop Lc_k2
        k2 = lc_k2->mux->mux(k2, 0, lc_k2->sel);
        lc_k2->mux->muxUpdate(lc_k2->sel);
        lc_k2->mux->outChan->value = k2;
        lc_k2->loopVar->get();
        lc_k2->loopVar->value = lc_k2->loopVar->assign(lc_k2->mux->outChan);
        k2 = lc_k2->loopVar->assign(lc_k2->mux->outChan) + 1;
        lc_k2->lcUpdate(k2 < chan_i_lc->value);

        // loop interface: var k2
        chan_ajk_addr->get();
        chan_ajk_addr->value = chan_ajk_addr->assign(chan_j2_lc) * matrix_size + chan_ajk_addr->assign(lc_k2->loopVar);

        chan_aki_addr->get();
        chan_aki_addr->value = chan_aki_addr->assign(lc_k2->loopVar) * matrix_size + chan_aki_addr->assign(chan_i_relay_loop_j2);

        lse_ld_ajk->get();
        lse_ld_ajk->value = lse_ld_ajk->assign();

        lse_ld_aki->get();
        lse_ld_aki->value = lse_ld_aki->assign();

        chan_sum_loop_k2->get();
        chan_sum_loop_k2->value -= matrix[chan_sum_loop_k2->assign(lse_ld_ajk)] * matrix[chan_sum_loop_k2->assign(lse_ld_aki)];

        chan_sum_div->get();
        chan_sum_div->value = chan_sum_div->assign(chan_sum_loop_k2) / chan_sum_div->assign(lse_ld_aii);

        //** loop Lc_j2 tail
        lse_st_aij->get();
        lse_st_aij->value = lse_st_aij->assign();
        if (!lse_st_aji->chanBuffer[0].empty() && !lse_st_aji->chanBuffer[1].empty())
        {
            int addr_aji = lse_st_aji->chanBuffer[0].front().value;
            int data_aji = lse_st_aji->chanBuffer[1].front().value;
            matrix[addr_aji] = data_aji;
        }


        //** MemorySystem update
        memSys->MemSystemUpdate();

        end->get();

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_brief;
        debug->debug_mode = Debug_mode::Turn_off;

        if (iter > 0)
        {
            debug->getFile() << std::endl;
            debug->getFile() << "Loop index jj: " << chan_jj_relay_loop_k->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index kk: " << chan_kk_relay_loop_i->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;
            debug->getFile() << "Loop index k: " << chan_k_lc->value << std::endl;
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
            debug->getFile() << "*****************  k  *****************" << std::endl;
            debug->chanPrint("lc_k->loopVar", lc_k->loopVar);
            debug->chanPrint("chan_k_lc", chan_k_lc);
            debug->chanPrint("chan_m1_addr", chan_m1_addr);
            debug->chanPrint("chan_k_row", chan_k_row);
            debug->chanPrint("chan_i_row_relay_loop_k", chan_i_row_relay_loop_k);
            debug->lsePrint("lse_ld_m1", lse_ld_m1);
            debug->chanPrint("chan_m1_getData", chan_m1_getData);
            debug->chanPrint("chan_jj_relay_loop_k", chan_jj_relay_loop_k);
            // loop j
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  j  *****************" << std::endl;
            debug->chanPrint("lc_j->loopVar", lc_j->loopVar);
            debug->chanPrint("chan_m2_addr", chan_m2_addr);
            debug->chanPrint("chan_m2_addr_delay", chan_m2_addr_delay);
            debug->lsePrint("lse_ld_m2", lse_ld_m2);
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
            debug->memSysPrint(memSys);
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

    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
}