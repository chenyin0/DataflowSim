#include "./Aes.h"

using namespace DFSimTest;

void AesTest::aes_Base(Debug* debug)
{
    // Generate benchmark data
    // Not use real data


    //******  Define module  ******//
    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare registry
    Registry* registry = new Registry(memSys);

    ////*** Declare Lse
    //Lse* lse_ld_ctx = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load ctx
    //lse_ld_ctx->noLatencyMode = 1;
    //Lse* lse_ld_k = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load k
    //lse_ld_ctx->noLatencyMode = 1;
    //Lse* lse_ld_buf = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load buf
    //lse_ld_ctx->noLatencyMode = 1;


    //*** Declare Lc
    // Loop i
    ChanBase* lc_i_loopVar = new ChanBase(2, 0, 1);
    ChanBase* lc_i_getEnd = new ChanBase(2, 0, 1);
    ChanBase* lc_i_sendEnd = new ChanBase(2, 0, 1);

    ChanBase* lc_i_mux_trueChan = new ChanBase(2, 0, 1);
    ChanBase* lc_i_mux_falseChan = new ChanBase(2, 0, 1);
    ChanBase* lc_i_mux_outChan = new ChanBase(2, 0, 1);

    //// Inner most loop not need to declare the loop index channel below
    //ChanBase* chan_i_lc = new ChanBase(2, 0, 1);
    //chan_i_lc->keepMode = 1;

    Mux* lc_i_mux = new Mux(lc_i_mux_trueChan, lc_i_mux_falseChan, lc_i_mux_outChan);
    lc_i_mux->addPort({ lc_i_loopVar }, { }, { lc_i_loopVar });
    Lc* lc_i = new Lc(lc_i_loopVar, lc_i_getEnd, lc_i_sendEnd, lc_i_mux);
    // Set outer-most loop
    lc_i->isOuterMostLoop = 1;


    //*** Declare channel
    ChanBase* begin = new ChanBase(1, 0);
    begin->noUpstream = 1;
    ChanBase* end = new ChanBase(1, 0);
    end->noDownstream = 1;

    uint64_t BASE_INPUT_BUFF_SIZE = Global::base_input_buffer_size;
    // loop i
    ChanBase* chan_aes_subByte = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, Base_speedup);
    ChanBase* chan_aes_shiftRows = new ChanBase(2 * BASE_INPUT_BUFF_SIZE, 2, Base_speedup);
    ChanBase* chan_aes_mixColumns = new ChanBase(12 * BASE_INPUT_BUFF_SIZE, 12, Base_speedup);

    ChanBase* chan_cond = new ChanBase(16 * BASE_INPUT_BUFF_SIZE, 1, Base_speedup);
    chan_cond->isCond = 1;

    // True path
    ChanBase* chan_truePath_aes_addRoundKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 4, Base_speedup);
    chan_truePath_aes_addRoundKey->branchMode = 1;
    chan_truePath_aes_addRoundKey->channelCond = 1;

    // False path
    ChanBase* chan_falsePath_aes_expandEncKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 8, Base_speedup);
    chan_falsePath_aes_expandEncKey->branchMode = 1;
    chan_falsePath_aes_expandEncKey->channelCond = 0;

    ChanBase* chan_falsePath_aes_addRoundKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 4, Base_speedup);
    chan_falsePath_aes_addRoundKey->branchMode = 1;
    chan_falsePath_aes_addRoundKey->channelCond = 0;

    ChanBase* chan_branch_merge = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, Base_speedup);


    //*** Define interconnect
    // Begin & end connect to the outer most loop
    begin->addDownstream({ lc_i_mux_falseChan });
    end->addUpstream({ lc_i->sendEnd });

    // loop i
    chan_aes_subByte->addUpstream({ lc_i->loopVar });
    chan_aes_subByte->addDownstream({ chan_aes_shiftRows });

    chan_aes_shiftRows->addUpstream({ chan_aes_subByte });
    chan_aes_shiftRows->addDownstream({ chan_aes_mixColumns });

    chan_aes_mixColumns->addUpstream({ chan_aes_shiftRows });
    chan_aes_mixColumns->addDownstream({ chan_cond });

    chan_cond->addUpstream({ lc_i->loopVar, chan_aes_mixColumns });
    chan_cond->addDownstream({ chan_branch_merge, chan_truePath_aes_addRoundKey, chan_falsePath_aes_expandEncKey });

    chan_truePath_aes_addRoundKey->addUpstream({ chan_cond });
    chan_truePath_aes_addRoundKey->addDownstream({ chan_branch_merge });

    chan_falsePath_aes_expandEncKey->addUpstream({ chan_cond });
    chan_falsePath_aes_expandEncKey->addDownstream({ chan_falsePath_aes_addRoundKey });

    chan_falsePath_aes_addRoundKey->addUpstream({ chan_falsePath_aes_expandEncKey });
    chan_falsePath_aes_addRoundKey->addDownstream({ chan_branch_merge });

    chan_branch_merge->addUpstream({ chan_cond, chan_truePath_aes_addRoundKey, chan_falsePath_aes_addRoundKey });
    chan_branch_merge->noDownstream = 1;


    //*** LC addPort : getAct, sendAct, getEnd, sendEnd
    lc_i->addPort({ }, { chan_cond, chan_aes_subByte }, { chan_branch_merge }, { end });
    lc_i->addDependence({ begin }, {});  // No loop dependence


    //*** Simulate
    // Initiation
    registry->init();  // Update registry and initial all the module in registry

    begin->get({ 1 });
    uint64_t iter = 0;

    int i = 1;

    uint64_t max_iter = 500000;
    uint64_t segment = max_iter / 100;
    uint64_t percent = 0;
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

        //** Loop Lc_jj
        i = lc_i->mux->mux(i, 0, lc_i->sel);
        lc_i->mux->muxUpdate(lc_i->sel);
        lc_i->mux->outChan->value = i;
        lc_i->loopVar->get();
        lc_i->loopVar->value = lc_i->loopVar->assign(lc_i->mux->outChan);  // After get(), must update chan's value
        i = lc_i->loopVar->value + 1;
        lc_i->lcUpdate(i < 14 * segment_size);

        // Clear begin
        begin->valid = 0;

        chan_aes_subByte->get();
        chan_aes_subByte->value = chan_aes_subByte->assign(lc_i->loopVar);

        chan_aes_shiftRows->get();
        chan_aes_shiftRows->value = chan_aes_shiftRows->assign(chan_aes_subByte);

        chan_aes_mixColumns->get();
        chan_aes_mixColumns->value = chan_aes_mixColumns->assign(chan_aes_shiftRows);

        chan_cond->get();
        chan_cond->value = (chan_cond->assign(lc_i->loopVar) & 1) ? 1 : 0;
        if (chan_cond->valid)
        {
            chan_cond->channel.front().cond = chan_cond->value;  // Update data cond before send data out
        }

        chan_truePath_aes_addRoundKey->get();
        chan_truePath_aes_addRoundKey->value = chan_truePath_aes_addRoundKey->assign(chan_cond);

        chan_falsePath_aes_expandEncKey->get();
        chan_falsePath_aes_expandEncKey->value = chan_falsePath_aes_expandEncKey->assign(chan_cond);

        chan_falsePath_aes_addRoundKey->get();
        chan_falsePath_aes_addRoundKey->value = chan_falsePath_aes_addRoundKey->assign(chan_falsePath_aes_expandEncKey);

        chan_branch_merge->get();
        uint64_t cond = chan_branch_merge->assign(chan_cond);
        uint64_t trueData = chan_branch_merge->assign(chan_truePath_aes_addRoundKey);
        uint64_t falseData = chan_branch_merge->assign(chan_falsePath_aes_addRoundKey);
        chan_branch_merge->value = cond ? trueData : falseData;


        //** MemorySystem update
        memSys->MemSystemUpdate();

        end->get();

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_brief;
        //debug->debug_mode = Debug_mode::Turn_off;

        if (iter > 0)
        {
            debug->getFile() << std::endl;
            debug->getFile() << "Loop index i: " << lc_i->loopVar->value << std::endl;  // Inner most relay channel
            debug->chanPrint("begin", begin);

            // loop i
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  i  *****************" << std::endl;
            debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
            debug->chanPrint("chan_aes_subByte", chan_aes_subByte);
            debug->chanPrint("chan_aes_shiftRows", chan_aes_shiftRows);
            debug->chanPrint("chan_aes_mixColumns", chan_aes_mixColumns);
            debug->chanPrint("chan_cond", chan_cond);

            debug->chanPrint("chan_truePath_aes_addRoundKey", chan_truePath_aes_addRoundKey);

            debug->chanPrint("chan_falsePath_aes_expandEncKey", chan_falsePath_aes_expandEncKey);
            debug->chanPrint("chan_falsePath_aes_addRoundKey", chan_falsePath_aes_addRoundKey);
            debug->chanPrint("chan_branch_merge", chan_branch_merge);

            debug->getFile() << std::endl;
            debug->getFile() << "*****************  End signal  *****************" << std::endl;

            debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
            debug->getFile() << "lc_i loopEnd: " << lc_i->loopEnd << std::endl;

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