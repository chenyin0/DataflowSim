#include "./Aes.h"

using namespace DFSimTest;

void AesTest::aes_Base(Debug* debug)
{
    // Generate benchmark data
    // Not use real data


    //******  Define module  ******//
    //*** Declare registry
    Registry* registry = new Registry();

    //*** Declare memory
    MemSystem* memSys = new MemSystem();


    //*** Declare Lse
    Lse* lse_ld_ctx = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load ctx
    lse_ld_ctx->noLatencyMode = 1;
    Lse* lse_ld_k = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load k
    lse_ld_ctx->noLatencyMode = 1;
    Lse* lse_ld_buf = new Lse(LSE_QUEUE_SIZE, 0, false, memSys, 1);  // Load buf
    lse_ld_ctx->noLatencyMode = 1;


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

    // loop i
    ChanBase* chan_aes_subByte = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 16, 1);
    ChanBase* chan_aes_shiftRows = new ChanBase(2 * BASE_INPUT_BUFF_SIZE, 2, 1);
    ChanBase* chan_aes_mixColumns = new ChanBase(6 * BASE_INPUT_BUFF_SIZE, 24, 1);

    ChanBase* chan_cond = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, 1);
    chan_cond->isCond = 1;

    // True path
    ChanBase* chan_truePath_aes_addRoundKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, 1);
    chan_truePath_aes_addRoundKey->branchMode = 1;
    chan_truePath_aes_addRoundKey->channelCond = 1;

    // False path
    ChanBase* chan_falsePath_aes_expandEncKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 4, 1);
    chan_falsePath_aes_expandEncKey->branchMode = 1;
    chan_falsePath_aes_expandEncKey->channelCond = 0;

    ChanBase* chan_falsePath_aes_addRoundKey = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, 1);
    chan_falsePath_aes_addRoundKey->branchMode = 1;
    chan_falsePath_aes_addRoundKey->channelCond = 0;

    ChanBase* chan_branch_merge = new ChanBase(1 * BASE_INPUT_BUFF_SIZE, 1, 1);
}