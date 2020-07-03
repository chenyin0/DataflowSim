#include "./Bfs.h"

using namespace DFSimTest;

/*

1. Compare cache OoO vs. cache in order
2. Test SGMF mode

*/

void BfsTest::bfs_SGMF(Debug* debug)
{
    // Generate benchmark data
    generateData();


    //******  Define module  ******//
    //*** Declare registry
    Registry* registry = new Registry();

    //*** Declare memory
    MemSystem* memSys = new MemSystem();

    //*** Declare Lse
    Lse* lse_ld_node = new Lse(SGMF_INPUT_BUFF_SIZE, 0, false, memSys);  // Load node
    Lse* lse_ld_edge = new Lse(SGMF_INPUT_BUFF_SIZE, 0, false, memSys);  // Load edge
    Lse* lse_ld_level = new Lse(SGMF_INPUT_BUFF_SIZE, 0, false, memSys);  // Load level


    //*** Declare Lc
    // Loop i
    ChanSGMF* lc_i_loopVar = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanBase* lc_i_getEnd = new ChanBase(2, 0);
    ChanBase* lc_i_sendEnd = new ChanBase(2, 0);

    ChanSGMF* lc_i_mux_trueChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* lc_i_mux_falseChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* lc_i_mux_outChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);

    ChanBase* chan_i_lc = new ChanBase(16, 0);  // KeepMode should be ChanBase
    //ChanSGMF* chan_i_lc = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    chan_i_lc->keepMode = 1;

    MuxSGMF* lc_i_mux = new MuxSGMF(lc_i_mux_trueChan, lc_i_mux_falseChan, lc_i_mux_outChan);
    lc_i_mux->addPort({ lc_i_loopVar }, { }, { lc_i_loopVar });
    LcSGMF* lc_i = new LcSGMF(lc_i_loopVar, lc_i_getEnd, lc_i_sendEnd, lc_i_mux);
    // Set outer-most loop
    lc_i->isOuterMostLoop = 1;

    // Loop j
    ChanSGMF* lc_j_loopVar = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    //ChanBase* lc_j_loopVar = new ChanBase(2, 0);
    ChanBase* lc_j_getEnd = new ChanBase(2, 0);
    ChanBase* lc_j_sendEnd = new ChanBase(2, 0);

    ChanSGMF* lc_j_mux_trueChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* lc_j_mux_falseChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* lc_j_mux_outChan = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);

    //ChanSGMF* chan_j_lc = new ChanSGMF(2, 0);
    //chan_j_lc->keepMode = 1;

    MuxSGMF* lc_j_mux = new MuxSGMF(lc_j_mux_trueChan, lc_j_mux_falseChan, lc_j_mux_outChan);
    lc_j_mux->addPort({ lc_j_loopVar }, { }, { lc_j_loopVar });
    LcSGMF* lc_j = new LcSGMF(lc_j_loopVar, lc_j_getEnd, lc_j_sendEnd, lc_j_mux);
    

    //*** Declare channel
    ChanBase* begin = new ChanBase(1, 0);
    begin->noUpstream = 1;
    ChanBase* end = new ChanBase(1, 0);
    end->noDownstream = 1;

    // Loop i
    ChanBase* chan_queue = new ChanBase(200, 0);  // chan_queue is a fifo
    
    //ChanBase* chan_node = new ChanBase(16, 0);
    ChanSGMF* chan_node = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    chan_node->keepMode = 1;

    // Loop j
    ChanSGMF* chan_edge_addr = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* chan_level_addr = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);
    ChanSGMF* chan_check_mark = new ChanSGMF(SGMF_INPUT_BUFF_SIZE, 0);


    //*** Define interconnect
    // Begin & end connect to the outer most loop
    begin->addDownstream({ lc_i_mux_falseChan });
    end->addUpstream({ lc_j->sendEnd });

    // Loop index channel
    chan_i_lc->addUpstream({ lc_i->loopVar });
    chan_i_lc->addDownstream({ lc_j->loopVar });

    // Loop i
    chan_queue->addUpstream({ lc_i->loopVar, chan_check_mark });
    chan_queue->addDownstream({ lse_ld_node });

    lse_ld_node->addUpstream({ chan_queue });
    lse_ld_node->addDownstream({ chan_node });

    chan_node->addUpstream({ lse_ld_node });
    chan_node->addDownstream({ /*lc_j->mux->falseChan*/lc_j->loopVar/*, lc_j->mux->falseChan*/ });

    // Loop j
    chan_edge_addr->addUpstream({ lc_j->loopVar });
    chan_edge_addr->addDownstream({ lse_ld_edge });

    lse_ld_edge->addUpstream({ chan_edge_addr });
    lse_ld_edge->addDownstream({ chan_level_addr });

    chan_level_addr->addUpstream({ lse_ld_edge });
    chan_level_addr->addDownstream({ lse_ld_level });

    lse_ld_level->addUpstream({ chan_level_addr });
    lse_ld_level->addDownstream({ chan_check_mark });

    chan_check_mark->addUpstream({ lse_ld_level });
    chan_check_mark->addDownstream({ chan_queue });


    //*** LC addPort : getAct, sendAct, getEnd, sendEnd
    lc_i->addPort({ }, { chan_i_lc, chan_queue }, { lc_j->sendEnd }, { end });
    lc_i->addDependence({ begin }, {});  // No loop dependence

    lc_j->addPort({ chan_i_lc, chan_node }, { chan_edge_addr }, { chan_check_mark }, { lc_i->getEnd });
    lc_j->addDependence({ chan_node }, {});  // Loop initial from chan_node (New node)


    //*** Simulate
    // Initiation
    registry->tableInit();  // Update registry and initial all the module in registry

    begin->get({ 1 });
    uint iter = 0;

    int i = 0;
    int j = 0;

    // Push initial data
    Data data;
    data.valid = 1;
    data.value = initialNode;
    uint queueBufferId = chan_queue->getChanId(chan_check_mark);
    chan_queue->chanBuffer[queueBufferId].push_back(data);

    uint nodeCnt = 0;

    uint max_iter = 50000;
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
        lc_i->lcUpdate(i < nodeNum);

        begin->valid = 0;

        // loop interface: var i
        chan_i_lc->get();
        chan_i_lc->value = chan_i_lc->assign(lc_i->loopVar);

        chan_queue->get();
        chan_queue->value = chan_queue->assign(chan_check_mark) + nodeBaseAddr;  // Index -> addr

        lse_ld_node->get();
        lse_ld_node->value = lse_ld_node->assign();

        chan_node->get();
        chan_node->value = chan_node->assign(lse_ld_node) - nodeBaseAddr;  // Addr -> index

        //** Loop Lc_j
        uint edgeBegin = nodes[chan_node->value].begin;
        uint edgeEnd = nodes[chan_node->value].end;

        j = lc_j->mux->mux(j, edgeBegin, lc_j->sel);
        //std::cout << j << std::endl;
        lc_j->mux->muxUpdate(lc_j->sel);
        lc_j->mux->outChan->value = j;
        lc_j->loopVar->get();
        lc_j->loopVar->value = lc_j->loopVar->assign(lc_j->mux->outChan);
        j = lc_j->loopVar->assign(lc_j->mux->outChan) + 1;
        lc_j->lcUpdate(j <= edgeEnd);

        //if (lc_j->loopVar->valid && lc_j->loopVar->channel.front().last)
        //{
        //    chan_node->popChannel(1, 1);
        //    chan_node->valid = 0;
        //}

        chan_edge_addr->get();
        chan_edge_addr->value = chan_edge_addr->assign(lc_j->loopVar) + edgeBaseAddr;

        lse_ld_edge->get();
        lse_ld_edge->value = lse_ld_edge->assign() - edgeBaseAddr;  // Addr -> index

        chan_level_addr->get();
        uint edgeIndex = chan_level_addr->assign(lse_ld_edge);
        chan_level_addr->value = edges[edgeIndex].dst + levelBaseAddr;

        lse_ld_level->get();
        lse_ld_level->value = lse_ld_level->assign() - levelBaseAddr;

        chan_check_mark->get();
        uint levelIndex = chan_check_mark->assign(lse_ld_level);

        if (level[levelIndex] == -1)
        {
            chan_check_mark->value = levelIndex;
            level[levelIndex] = 0;
            ++nodeCnt;
            std::cout << nodeCnt << "\t" << levelIndex << std::endl;
            std::cout << nodes[levelIndex].begin << "\t" << nodes[levelIndex].end << std::endl;
        }
        else
        {
            // If current node has been visited, just pop it out rather than push it to the chan_queue
            if (chan_check_mark->valid)
            {
                chan_check_mark->popChannel(1, 1);  // Compel pop
                chan_check_mark->valid = 0;
            }
        }

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //end->get();

        //std::cout << nodeCnt << std::endl;

        // If chan_queue is empty after each chird node has been visited, break!
        if (nodeCnt >= 213 && chan_queue->chanBuffer[queueBufferId].empty() && lse_ld_level->chanBuffer[0].empty())
        {
            Data endSignal;
            end->channel.push_back(endSignal);  // Break, compel end!
            end->get();
        }

        debug->getFile() << "nodeCnt: " << nodeCnt << std::endl;

        //** Print log
        // Set debug mode
        //debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (iter >= 0)
        {
            //debug->getFile() << std::endl;
            //debug->getFile() << "Loop index jj: " << jj_ << std::endl;
            //debug->getFile() << "Loop index kk: " << kk_ << std::endl;
            //debug->getFile() << "Loop index i: " << i_ << std::endl;
            //debug->getFile() << "Loop index k: " << k_ << std::endl;
            //debug->getFile() << "Loop index j: " << j_ << std::endl;

            debug->getFile() << std::endl;
            debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;
            debug->getFile() << "Loop index j: " << lc_j->loopVar->value << std::endl;

            //debug->vecPrint("Result", res, 15);

            //debug->chanPrint("begin", begin);

            // loop i
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  i  *****************" << std::endl;
            debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
            debug->chanPrint("chan_i_lc", chan_i_lc);
            debug->chanPrint("chan_queue", chan_queue);
            debug->lsePrint("lse_ld_node", lse_ld_node);
            debug->chanPrint("chan_node", chan_node);
            // loop j
            debug->getFile() << std::endl;
            debug->getFile() << "*****************  j  *****************" << std::endl;
            debug->getFile() << "edgeBegin: " << edgeBegin << "\t edgeEnd: " << edgeEnd << std::endl;
            debug->chanPrint("lc-j->mux->outChan", lc_j->mux->outChan);
            debug->chanPrint("lc_j->loopVar", lc_j->loopVar);
            debug->chanPrint("chan_edge_addr", chan_edge_addr);
            debug->lsePrint("lse_ld_edge", lse_ld_edge);
            debug->chanPrint("chan_level_addr", chan_level_addr);
            debug->lsePrint("lse_ld_level", lse_ld_level);
            debug->chanPrint("chan_check_mark", chan_check_mark);

            debug->getFile() << std::endl;
            debug->getFile() << "*****************  End signal  *****************" << std::endl;
            debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
            debug->getFile() << "lc_i loopEnd: " << lc_i->loopEnd << std::endl;

            debug->chanPrint("lc_j->getEnd", lc_j->getEnd);
            debug->getFile() << "lc_j loopEnd: " << lc_j->loopEnd << std::endl;

            debug->chanPrint("end", end);

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