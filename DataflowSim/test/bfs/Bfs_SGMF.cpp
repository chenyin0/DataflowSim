#include "./Bfs.h"

using namespace DFSimTest;

/*

1. Compare cache OoO vs. cache in order
2. Test SGMF mode

*/

void BfsTest::bfs_sgmf(Debug* debug)
{
    // Generate benchmark data
    generateData();


    //******  Define module  ******//
    //*** Declare registry
    Registry* registry = new Registry();

    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare Lse
    Lse* lse_ld_node = new Lse(16, 0, false, memSys, 5);  // Load node
    Lse* lse_ld_edge = new Lse(16, 0, false, memSys, 5);  // Load edge
    Lse* lse_ld_level = new Lse(16, 0, false, memSys, 5);  // Load level


    //*** Declare Lc
    // Loop i
    ChanSGMF* lc_i_loopVar = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_i_getEnd = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_i_sendEnd = new ChanSGMF(2, 0, 1);

    ChanSGMF* lc_i_mux_trueChan = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_i_mux_falseChan = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_i_mux_outChan = new ChanSGMF(2, 0, 1);

    ChanSGMF* chan_i_lc = new ChanSGMF(2, 0, 1);
    chan_i_lc->keepMode = 1;

    MuxSGMF* lc_i_mux = new MuxSGMF(lc_i_mux_trueChan, lc_i_mux_falseChan, lc_i_mux_outChan);
    lc_i_mux->addPort({ lc_i_loopVar }, { }, { lc_i_loopVar });
    //Mux* lc_i_mux = new Mux({ lc_i_loopVar }, { }, { lc_i_loopVar });
    LcSGMF* lc_i = new LcSGMF(lc_i_loopVar, lc_i_getEnd, lc_i_sendEnd, lc_i_mux);

    // Loop j
    ChanSGMF* lc_j_loopVar = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_j_getEnd = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_j_sendEnd = new ChanSGMF(2, 0, 1);

    ChanSGMF* lc_j_mux_trueChan = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_j_mux_falseChan = new ChanSGMF(2, 0, 1);
    ChanSGMF* lc_j_mux_outChan = new ChanSGMF(2, 0, 1);

    ChanSGMF* chan_j_lc = new ChanSGMF(2, 0, 1);
    chan_j_lc->keepMode = 1;

    MuxSGMF* lc_j_mux = new MuxSGMF(lc_j_mux_trueChan, lc_j_mux_falseChan, lc_j_mux_outChan);
    lc_j_mux->addPort({ lc_j_loopVar }, { }, { lc_j_loopVar });
    //Mux* lc_j_mux = new Mux({ lc_j_loopVar }, { }, { lc_j_loopVar });
    LcSGMF* lc_j = new LcSGMF(lc_j_loopVar, lc_j_getEnd, lc_j_sendEnd, lc_j_mux);
    
}