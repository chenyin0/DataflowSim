#include "./Lud.h"

using namespace DFSimTest;

void LudTest::lud_DGSF(Debug* debug)
{
    // Generate benchmark data
    generateData();


    //******  Define module  ******//
    //*** Declare registry
    Registry* registry = new Registry();

    //*** Declare memory
    MemSystem* memSys = new MemSystem();


    ////*** Declare Lse
    //Lse* lse_ld_aii = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[i][i]
    //Lse* lse_ld_aij = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[i][j]
    //Lse* lse_ld_aik = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[i][k]
    //Lse* lse_ld_akj = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[k][j]
    //Lse* lse_st_aij = new Lse(LSE_QUEUE_SIZE, 0, true, memSys, 1);  // Store a[i][j]

    //Lse* lse_ld_aji = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[j][i]
    //Lse* lse_ld_ajk = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // load a[j][k]
    //Lse* lse_ld_aki = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load a[k][i]
    //Lse* lse_st_aji = new Lse(LSE_QUEUE_SIZE, 0, true, memSys, 1);  // Store a[j][i]


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

    // loop i
    ChanBase* chan_j1_fifo_DGSF = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Store each row of upper matrix(U); Use ChanBase to replace ChanDGSF

    ChanBase* chan_j1_fifo_keepMode = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Send the element of upper matrix(U) to inner loop j1 (reuse time = matrix_size - i)
    chan_j1_fifo_keepMode->keepMode = 1;

    ChanBase* chan_j2_fifo_DGSF = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Store each row of lower matrix(L); Use ChanBase to replace ChanDGSF

    ChanBase* chan_j2_fifo_keepMode = new ChanBase(BASE_INPUT_BUFF_SIZE, 0, 1);  // Send the element of lower matrix(L) to inner loop j2 (reuse time = matrix_size - (i+1))
    chan_j2_fifo_keepMode->keepMode = 1;

    // loop j1
    ChanBase* chan_i_relay_loop_j1 = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_i_relay_loop_j1->keepMode = 1;

    ChanBase* chan_j1_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);  // Emulate operations in loop j1

    ChanBase* chan_j1_fifo_delay = new ChanBase(2 * BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, ADD + MUL, DGSF_outer_loop_speedup);

    ChanBase* chan_k1_fifo_keepMode = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_k1_fifo_keepMode->keepMode = 1;

    ChanBase* chan_sum_loop_j1_drainMode = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_sum_loop_j1_drainMode->drainMode = 1;

    ChanBase* chan_j1_store_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);

    // loop j2
    ChanBase* chan_i_relay_loop_j2 = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_i_relay_loop_j2->keepMode = 1;

    ChanBase* chan_j2_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);  // Emulate operations in loop j2

    ChanBase* chan_j2_fifo_delay = new ChanBase(2 * BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, ADD + MUL, DGSF_outer_loop_speedup);

    ChanBase* chan_k2_fifo_keepMode = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_k2_fifo_keepMode->keepMode = 1;

    ChanBase* chan_sum_loop_j2_drainMode = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);
    chan_sum_loop_j2_drainMode->drainMode = 1;

    ChanBase* chan_j2_store_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_outer_loop_speedup, 0, DGSF_outer_loop_speedup);

    // loop k1
    ChanBase* chan_k1_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_inner_loop_speedup, 0, DGSF_inner_loop_speedup);
    ChanBase* chan_k1_fifo_delay = new ChanBase(4 * BASE_INPUT_BUFF_SIZE * DGSF_inner_loop_speedup, ADD + 2 * MUL + SUB, DGSF_inner_loop_speedup);

    // loop k2
    ChanBase* chan_k2_fifo = new ChanBase(BASE_INPUT_BUFF_SIZE * DGSF_inner_loop_speedup, 0, DGSF_inner_loop_speedup);
    ChanBase* chan_k2_fifo_delay = new ChanBase(4 * BASE_INPUT_BUFF_SIZE * DGSF_inner_loop_speedup, ADD + 2 * MUL + SUB, DGSF_inner_loop_speedup);


    //*** Define interconnect
    // Begin & end connect to the outer most loop
    begin->addDownstream({ lc_i_mux_falseChan });
    end->addUpstream({ lc_i->sendEnd });

    // Loop index channel
    chan_i_lc->addUpstream({ lc_i->loopVar });
    chan_i_lc->addDownstream({ lc_j1->loopVar, lc_j2->loopVar });

    chan_j1_lc->addUpstream({ lc_j1->loopVar });
    chan_j1_lc->addDownstream({ lc_k1->loopVar });

    chan_j2_lc->addUpstream({ lc_j2->loopVar });
    chan_j2_lc->addDownstream({ lc_k2->loopVar });

    // loop i
    chan_j1_fifo_DGSF->addUpstream({ lc_i->loopVar });
    chan_j1_fifo_DGSF->addDownstream({ chan_j1_fifo_keepMode });

    chan_j1_fifo_keepMode->addUpstream({ chan_j1_fifo_DGSF });
    chan_j1_fifo_keepMode->addDownstream({ chan_j1_fifo });

    chan_j2_fifo_DGSF->addUpstream({ lc_i->loopVar });
    chan_j2_fifo_DGSF->addDownstream({ chan_j2_fifo_keepMode });

    chan_j2_fifo_keepMode->addUpstream({ chan_j2_fifo_DGSF });
    chan_j2_fifo_keepMode->addDownstream({ chan_j2_fifo });

    // loop j1
    chan_i_relay_loop_j1->addUpstream({ lc_j1->loopVar, chan_i_lc });
    chan_i_relay_loop_j1->addDownstream({ chan_k1_fifo });

    chan_j1_fifo->addUpstream({ lc_j1->loopVar, chan_j1_fifo_keepMode });
    chan_j1_fifo->addDownstream({ chan_j1_fifo_delay });

    chan_j1_fifo_delay->addUpstream({ chan_j1_fifo });
    chan_j1_fifo_delay->addDownstream({ chan_k1_fifo_keepMode });

    chan_k1_fifo_keepMode->addUpstream({ chan_j1_fifo_delay });
    chan_k1_fifo_keepMode->addDownstream({ chan_k1_fifo });

    chan_sum_loop_j1_drainMode->addUpstream({ chan_k1_fifo_delay });
    chan_sum_loop_j1_drainMode->addDownstream({ chan_j1_store_fifo });

    chan_j1_store_fifo->addUpstream({ lc_j1->loopVar, chan_sum_loop_j1_drainMode });
    chan_j1_store_fifo->noDownstream = 1;

    // loop j2
    chan_i_relay_loop_j2->addUpstream({ lc_j2->loopVar, chan_i_lc });
    chan_i_relay_loop_j2->addDownstream({ chan_k2_fifo });

    chan_j2_fifo->addUpstream({ lc_j2->loopVar, chan_j2_fifo_keepMode });
    chan_j2_fifo->addDownstream({ chan_j2_fifo_delay });

    chan_j2_fifo_delay->addUpstream({ chan_j2_fifo });
    chan_j2_fifo_delay->addDownstream({ chan_k2_fifo_keepMode });

    chan_k2_fifo_keepMode->addUpstream({ chan_j2_fifo_delay });
    chan_k2_fifo_keepMode->addDownstream({ chan_k2_fifo });

    chan_sum_loop_j2_drainMode->addUpstream({ chan_k2_fifo_delay });
    chan_sum_loop_j2_drainMode->addDownstream({ chan_j2_store_fifo });

    chan_j2_store_fifo->addUpstream({ lc_j2->loopVar, chan_sum_loop_j2_drainMode });
    chan_j2_store_fifo->noDownstream = 1;

    // loop k1
    chan_k1_fifo->addUpstream({ lc_k1->loopVar, chan_k1_fifo_keepMode, chan_i_relay_loop_j1 });
    chan_k1_fifo->addDownstream({ chan_k1_fifo_delay });

    chan_k1_fifo_delay->addUpstream({ chan_k1_fifo });
    chan_k1_fifo_delay->addDownstream({ chan_sum_loop_j1_drainMode });

    // loop k2
    chan_k2_fifo->addUpstream({ lc_k2->loopVar, chan_k2_fifo_keepMode, chan_i_relay_loop_j2 });
    chan_k2_fifo->addDownstream({ chan_k2_fifo_delay });

    chan_k2_fifo_delay->addUpstream({ chan_k2_fifo });
    chan_k2_fifo_delay->addDownstream({ chan_sum_loop_j2_drainMode });


    //*** LC addPort : getAct, sendAct, getEnd, sendEnd
    lc_i->addPort({ }, { chan_i_lc, chan_j1_fifo_DGSF, chan_j2_fifo_DGSF }, { lc_j1->sendEnd }, { end });
    lc_i->addDependence({ begin }, {});  // No loop dependence

    lc_j1->addPort({ chan_i_lc }, { chan_j1_lc, chan_j1_fifo, chan_i_relay_loop_j1, chan_j1_store_fifo }, { chan_j1_store_fifo }, { lc_i->getEnd });
    lc_j1->addDependence({}, {});  // No loop dependence

    lc_j2->addPort({ chan_i_lc }, { chan_j2_lc, chan_j2_fifo, chan_i_relay_loop_j2, chan_j2_store_fifo }, { chan_j2_store_fifo }, { /*lc_i->getEnd*/ });
    lc_j2->addDependence({}, {});  // No loop dependence

    lc_k1->addPort({ chan_j1_lc }, { chan_k1_fifo }, { chan_k1_fifo_delay }, { /*lc_j1->getEnd*/ });
    lc_k1->addDependence({}, {});  // No loop dependence

    lc_k2->addPort({ chan_j2_lc }, { chan_k2_fifo }, { chan_k2_fifo_delay }, { /*lc_j2->getEnd*/ });
    lc_k2->addDependence({}, {});  // No loop dependence

    lc_j2->sendEnd->noDownstream = 1;
    lc_k1->sendEnd->noDownstream = 1;
    lc_k2->sendEnd->noDownstream = 1;

    //*** Simulate
    // Initiation
    registry->tableInit();  // Update registry and initial all the module in registry

    begin->get({ 1 });
    uint iter = 0;

    int i = 1;
    int j1 = 0;
    int j2 = 0;
    int k1 = 0;
    int k2 = 0;

    lc_j1->loopVar->enable = 1;
    lc_j2->loopVar->enable = 0;

    //vector<int> res;  // Result
    //vector<int> temp; // temp_result

    uint max_iter = 5000000;
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

        chan_j1_fifo_DGSF->get();
        chan_j1_fifo_DGSF->value = chan_j1_fifo_DGSF->assign(lc_i->loopVar);

        chan_j1_fifo_keepMode->get();
        chan_j1_fifo_keepMode->value = chan_j1_fifo_keepMode->assign(chan_j1_fifo_DGSF);

        chan_j2_fifo_DGSF->get();
        chan_j2_fifo_DGSF->value = chan_j2_fifo_DGSF->assign(lc_i->loopVar);

        chan_j2_fifo_keepMode->get();
        chan_j2_fifo_keepMode->value = chan_j2_fifo_keepMode->assign(chan_j2_fifo_DGSF);

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

        if (lc_j1->loopVar->valid && lc_j1->loopVar->channel.front().last)
        {
            lc_j1->loopVar->enable = 0;
            lc_j2->loopVar->enable = 1;
        }

        // loop interface: var j1
        chan_j1_lc->get();
        chan_j1_lc->value = chan_j1_lc->assign(lc_j1->loopVar);

        chan_j1_fifo->get();
        chan_j1_fifo->value = chan_j1_fifo->assign(chan_j1_fifo_keepMode);

        chan_j1_fifo_delay->get();
        chan_j1_fifo_delay->value = chan_j1_fifo_delay->assign(chan_j1_fifo);

        chan_k1_fifo_keepMode->get();
        chan_k1_fifo_keepMode->value = chan_k1_fifo_keepMode->assign(chan_j1_fifo_delay);

        chan_i_relay_loop_j1->get();
        chan_i_relay_loop_j1->value = chan_i_relay_loop_j1->assign(lc_j1->loopVar);

        //** Loop Lc_k1
        k1 = lc_k1->mux->mux(k1, 0, lc_k1->sel);
        lc_k1->mux->muxUpdate(lc_k1->sel);
        lc_k1->mux->outChan->value = k1;
        lc_k1->loopVar->get();
        lc_k1->loopVar->value = lc_k1->loopVar->assign(lc_k1->mux->outChan);
        k1 = lc_k1->loopVar->assign(lc_k1->mux->outChan) + 1;
        lc_k1->lcUpdate(k1 < chan_i_relay_loop_j1->value);

        // loop interface: var k1
        chan_k1_fifo->get();
        uint aik_addr = chan_k1_fifo->assign(chan_i_relay_loop_j1) * matrix_size + chan_k1_fifo->assign(lc_k1->loopVar);
        uint akj_addr = chan_k1_fifo->assign(lc_k1->loopVar) * matrix_size + chan_k1_fifo->assign(chan_k1_fifo_keepMode);
        chan_k1_fifo->value -= matrix[aik_addr] * matrix[akj_addr];

        chan_k1_fifo_delay->get();
        chan_k1_fifo_delay->value = chan_k1_fifo_delay->assign(chan_k1_fifo);

        //** loop Lc_j1 tail
        chan_sum_loop_j1_drainMode->get();
        chan_sum_loop_j1_drainMode->value = chan_sum_loop_j1_drainMode->assign(chan_k1_fifo_delay);

        chan_j1_store_fifo->get();
        chan_j1_store_fifo->value = chan_j1_store_fifo->assign(chan_sum_loop_j1_drainMode);

        //if (lse_st_aij->valid && lse_st_aij->channel.front().last)
        //{
        //    lc_j2->loopVar->enable = 1;
        //}

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

        if (lc_j2->loopVar->valid && lc_j2->loopVar->channel.front().last)
        {
            lc_j2->loopVar->enable = 0;
            lc_j1->loopVar->enable = 1;
        }

        // loop interface: var j2
        chan_j2_lc->get();
        chan_j2_lc->value = chan_j2_lc->assign(lc_j2->loopVar);

        chan_j2_fifo->get();
        chan_j2_fifo->value = chan_j2_fifo->assign(chan_j2_fifo_keepMode);

        chan_j2_fifo_delay->get();
        chan_j2_fifo_delay->value = chan_j2_fifo_delay->assign(chan_j2_fifo);

        chan_k2_fifo_keepMode->get();
        chan_k2_fifo_keepMode->value = chan_k2_fifo_keepMode->assign(chan_j2_fifo_delay);

        chan_i_relay_loop_j2->get();
        chan_i_relay_loop_j2->value = chan_i_relay_loop_j2->assign(lc_j2->loopVar);

        //** Loop Lc_k2
        k2 = lc_k2->mux->mux(k2, 0, lc_k2->sel);
        lc_k2->mux->muxUpdate(lc_k2->sel);
        lc_k2->mux->outChan->value = k2;
        lc_k2->loopVar->get();
        lc_k2->loopVar->value = lc_k2->loopVar->assign(lc_k2->mux->outChan);
        k2 = lc_k2->loopVar->assign(lc_k2->mux->outChan) + 1;
        lc_k2->lcUpdate(k2 < chan_i_relay_loop_j2->value);

        // loop interface: var k2
        chan_k2_fifo->get();
        uint ajk_addr = chan_k2_fifo->assign(chan_k2_fifo_keepMode) * matrix_size + chan_k2_fifo->assign(lc_k2->loopVar);
        uint aki_addr = chan_k2_fifo->assign(lc_k2->loopVar) * matrix_size + chan_k2_fifo->assign(chan_i_relay_loop_j2);
        chan_k1_fifo->value -= matrix[ajk_addr] * matrix[aki_addr];

        chan_k2_fifo_delay->get();
        chan_k2_fifo_delay->value = chan_k2_fifo_delay->assign(chan_k2_fifo);

        //** loop Lc_j2 tail
        chan_sum_loop_j2_drainMode->get();
        chan_sum_loop_j2_drainMode->value = chan_sum_loop_j2_drainMode->assign(chan_k2_fifo_delay);

        chan_j2_store_fifo->get();
        chan_j2_store_fifo->value = chan_j2_store_fifo->assign(chan_sum_loop_j2_drainMode);

        //if (lse_st_aji->valid && lse_st_aji->channel.front().last)
        //{
        //    lc_j1->loopVar->enable = 1;
        //}


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
            debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index j1: " << chan_j1_lc->value << std::endl;  // Inner most relay channel
            debug->getFile() << "Loop index j2: " << chan_j2_lc->value << std::endl;
            debug->getFile() << "Loop index k1: " << lc_k1->loopVar->value << std::endl;
            debug->getFile() << "Loop index k2: " << lc_k2->loopVar->value << std::endl;

            //debug->vecPrint("Result", res, 15);

            debug->chanPrint("begin", begin);

            // loop i
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  i  *****************" << std::endl;
            debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
            debug->chanPrint("chan_i_lc", chan_i_lc);
            debug->chanPrint("chan_j1_fifo_DGSF", chan_j1_fifo_DGSF);
            debug->chanPrint("chan_j1_fifo_keepMode", chan_j1_fifo_keepMode);
            debug->chanPrint("chan_j2_fifo_DGSF", chan_j2_fifo_DGSF);
            debug->chanPrint("chan_j2_fifo_keepMode", chan_j2_fifo_keepMode);
            //for (auto& i : chan_i_lc->lastTagQueue)
            //{
            //    debug->getFile() << i.first << ": " << i.second.size() << std::endl;
            //}
            // loop j1
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  j1  *****************" << std::endl;
            debug->chanPrint("lc_j1->loopVar", lc_j1->loopVar);
            debug->chanPrint("chan_j1_lc", chan_j1_lc);
            debug->chanPrint("chan_i_relay_loop_j1", chan_i_relay_loop_j1);
            debug->chanPrint("chan_j1_fifo", chan_j1_fifo);
            debug->chanPrint("chan_j1_fifo_delay", chan_j1_fifo_delay);
            debug->chanPrint("chan_k1_fifo_keepMode", chan_k1_fifo_keepMode);
            debug->chanPrint("chan_sum_loop_j1_drainMode", chan_sum_loop_j1_drainMode);
            debug->chanPrint("chan_j1_store_fifo", chan_j1_store_fifo);
            // loop k1
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  k1  *****************" << std::endl;
            debug->chanPrint("lc_k1->loopVar", lc_k1->loopVar);
            debug->chanPrint("chan_k1_fifo", chan_k1_fifo);
            debug->chanPrint("chan_k1_fifo_delay", chan_k1_fifo_delay);
            // loop j2
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  j2  *****************" << std::endl;
            //debug->chanPrint("lc_j2->mux->trueChan", lc_j2->mux->trueChan);
            //debug->chanPrint("lc_j2->mux->falseChan", lc_j2->mux->falseChan);
            //debug->chanPrint("lc_j2->mux->outChan", lc_j2->mux->outChan);
            debug->chanPrint("lc_j2->loopVar", lc_j2->loopVar);
            debug->chanPrint("chan_j2_lc", chan_j2_lc);
            debug->chanPrint("chan_i_relay_loop_j2", chan_i_relay_loop_j2);
            debug->chanPrint("chan_j2_fifo", chan_j2_fifo);
            debug->chanPrint("chan_j2_fifo_delay", chan_j2_fifo_delay);
            debug->chanPrint("chan_k2_fifo_keepMode", chan_k2_fifo_keepMode);
            debug->chanPrint("chan_sum_loop_j2_drainMode", chan_sum_loop_j2_drainMode);
            debug->chanPrint("chan_j2_store_fifo", chan_j2_store_fifo);
            // loop k2
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  k2  *****************" << std::endl;
            debug->chanPrint("lc_k2->loopVar", lc_k2->loopVar);
            debug->chanPrint("chan_k2_fifo", chan_k2_fifo);
            debug->chanPrint("chan_k2_fifo_delay", chan_k2_fifo_delay);

            debug->getFile() << std::endl;
            debug->getFile() << "*****************  End signal  *****************" << std::endl;

            debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
            debug->getFile() << "lc_i loopEnd: " << lc_i->loopEnd << std::endl;
            //debug->chanPrint("lc_jj->sendEnd", lc_jj->sendEnd);

            debug->chanPrint("lc_j1->getEnd", lc_j1->getEnd);
            debug->getFile() << "lc_j1 loopEnd: " << lc_j1->loopEnd << std::endl;
            //debug->chanPrint("lc_kk->sendEnd", lc_kk->sendEnd);

            debug->chanPrint("lc_j2->getEnd", lc_j2->getEnd);
            debug->getFile() << "lc_j2 loopEnd: " << lc_j2->loopEnd << std::endl;
            //debug->chanPrint("lc_i->sendEnd", lc_i->sendEnd);

            debug->chanPrint("lc_k1->getEnd", lc_k1->getEnd);
            debug->getFile() << "lc_k1 loopEnd: " << lc_k1->loopEnd << std::endl;
            //debug->chanPrint("lc_k->sendEnd", lc_k->sendEnd);

            debug->chanPrint("lc_k2->getEnd", lc_k2->getEnd);
            debug->getFile() << "lc_k2 loopEnd: " << lc_k2->loopEnd << std::endl;
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

    delete registry;  // All the Module pointers have been deleted when destruct registry
    delete memSys;
}