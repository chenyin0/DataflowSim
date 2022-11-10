#include "Debug.h"

using namespace DFSim;

Debug::Debug(string fileName)
{
    _output_file.open(fileName);

    if (_output_file.is_open())
    {
        std::cout << "file open" << std::endl;
        _output_file << DFSim::ClkDomain::getInstance()->getCurrentSystemTime() << std::endl;
    }
}

Debug::~Debug()
{
    _output_file.close();
}

void Debug::chanPrint(const string name, const Channel* channel)
{
    if (debug_mode == Debug_mode::Print_detail)
    {
        _output_file << std::endl;
        _output_file << "== " << name << " valid: " << channel->valid;
        _output_file << "  ";
        //_output_file << "bp: " << channel->bp/* << std::endl*/;
        //_output_file << "  ";
        /*_output_file << "getLast: " << !channel->getLast.empty();
        _output_file << "  ";*/
        _output_file << "en: " << channel->enable; /*<< std::endl;*/
        _output_file << "  ";
        _output_file << "currId: " << channel->getCurrId();
        _output_file << "  ";
        _output_file << "speedup: " << channel->speedup;
        _output_file << "  ";
        _output_file << "graphId: " << channel->subgraphId;
        /*if (channel->chanType == ChanType::Chan_DGSF)
        {*/
            _output_file << "  ";
            _output_file << "currGraphId: " << GraphScheduler::currSubgraphId;
        //}
//#ifdef DGSF
        /*_output_file << "  ";
        _output_file << "getLast: " << channel->getTheLastData.front();*/
//#endif
        //if (channel->isLoopVar)
        //{
        //    _output_file << "  ";
        //    _output_file << "prodLast: " << !channel->produceLast.empty();
        //}
        _output_file << std::endl;

        // Print each chanBuffer
        for (size_t i = 0; i < channel->chanBuffer.size(); ++i)
        {
            _output_file << "chBuf_" << i << "(bp=" << channel->bp[i] << ")";
            _output_file << "(cnt=" << channel->chanBufferDataCnt[i] << ",";
            _output_file << "num=" << channel->chanBuffer[i].size() << ",";
            _output_file << "theLast=" << channel->getTheLastData[i] << "): ";
            for (auto& data : channel->chanBuffer[i])
            {
                if (data.valid)
                {
                    _output_file << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
                    _output_file << ":" << "lo" << data.lastOuter;
#ifdef DGSF
                    /*_output_file << ":" << "g" << data.graphSwitch;*/      
#endif
                    if (channel->branchMode || 
                        (channel->isCond && channel->chanType == ChanType::Chan_DGSF) || 
                        channel->chanType == ChanType::Chan_PartialMux)
                    {
                        _output_file << ":" << "b" << data.cond;
                    }
                    _output_file << " ";
                }
            }
            _output_file << std::endl;
        }

        _output_file << "chan(cnt=" << channel->chanDataCnt << "): ";
        for (auto& i : channel->channel)
        {
            _output_file << "d" << channel->value << ":" << "c" << i.cycle << ":" << "l" << i.last;
            _output_file << ":" << "lo" << i.lastOuter;
#ifdef DGSF
            //_output_file << ":" << "g" << i.graphSwitch;
#endif
            if (channel->branchMode || 
                channel->isCond ||
                channel->chanType == ChanType::Chan_PartialMux)
            {
                _output_file << ":" << "b" << i.cond;
            }
            _output_file << " ";
        }
        _output_file << std::endl;
    }
}

void Debug::chanPrint(const string name, const ChanSGMF* channel)
{
    if (debug_mode == Debug_mode::Print_detail)
    {
        _output_file << std::endl;
        _output_file << "== " << name << " valid: " << channel->valid;
        _output_file << "  ";
        //_output_file << "bp: " << channel->bp/* << std::endl*/;
        //_output_file << "  ";
        /*_output_file << "getLast: " << !channel->getLast.empty();
        _output_file << "  ";*/
        _output_file << "en: " << channel->enable << std::endl;

        // Print each chanBuffer
        for (size_t i = 0; i < channel->chanBuffer.size(); ++i)
        {
            _output_file << "chBuf_" << i << "(bp=" << channel->bp[i] << "): ";
            for (auto& data : channel->chanBuffer[i])
            {
                if (data.valid)
                {
                    _output_file << "t" << data.tag << ":" << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
                    _output_file << " ";
                }
            }
            _output_file << std::endl;
        }

        //// Print chanBundle
        //for (size_t i = 0; i < channel->chanBundle.size(); ++i)
        //{
        //    auto chan = channel->chanBundle[i];

        //    if (!chan.empty())
        //    {
        //        _output_file << "Chan" << i << ":" << std::endl;
        //        _output_file << "\t";
        //    }

        //    for (auto data : chan)
        //    {
        //        if (data.valid)
        //        {
        //            _output_file <</* "v" << data.valid << ":" <<*/ "t" << data.tag << ":";
        //            _output_file << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
        //            _output_file << " ";
        //        }
        //    }

        //    if (!chan.empty() && i != channel->chanBundle.size() - 1)
        //    {
        //        _output_file << std::endl;
        //    }
        //}

        // Print SGMF matchQ
        _output_file << std::endl;
        _output_file << "matchQ:" << std::endl;
        _output_file << "\t";
        for (auto& data : channel->matchQueue)
        {
            if (data.valid)
            {
                _output_file << "t" << data.tag << ":";
                _output_file <</* "d" << data.value << ":" <<*/ "c" << data.cycle << ":" << "l" << data.last;
                _output_file << " ";
            }
        }

        // Print SGMF channel
        _output_file << std::endl;
        _output_file << "chan(cnt=" << channel->chanDataCnt << "):" << std::endl;
        _output_file << "\t";
        for (auto& data : channel->channel)
        {
            _output_file << "t" << data.tag << ":";
            _output_file << "d" << channel->value << ":" << "c" << data.cycle << ":" << "l" << data.last;
            _output_file << " ";
        }
        _output_file << std::endl;
    }
}

void Debug::lsePrint(const string _name, const Lse* _lse)
{
    if (debug_mode == Debug_mode::Print_detail)
    {
        _output_file << std::endl;
        _output_file << "== " << _name << " valid: " << _lse->valid;
        _output_file << "  ";
        //_output_file << "bp: " << _lse->bp/* << std::endl*/;
        //_output_file << "  ";
        /*_output_file << "getLast: " << !_lse->getLast.empty();
        _output_file << "  ";*/
        _output_file << "en: " << _lse->enable;
        _output_file << "  ";
        _output_file << "currReqId: " << _lse->getCurrReqId() << std::endl;

        // Print each chanBuffer
        for (size_t i = 0; i < _lse->chanBuffer.size(); ++i)
        {
            _output_file << "chanBuffer_" << i;
            _output_file << "(bp=" << _lse->bp[i] << ")";
            _output_file << "(cnt=" << _lse->chanBufferDataCnt[i] << ",";
            _output_file << "theLast:" << _lse->getTheLastData[i] << "): ";

            for (auto& data : _lse->chanBuffer[i])
            {
                if (data.valid)
                {
                    _output_file << "t" << data.tag << ":" << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
                    _output_file << " ";
                }
            }
            _output_file << std::endl;
        }

        // Print Lse reqQueue
        _output_file << "reqQueue:" << std::endl;
        _output_file << std::setw(12) << "addr:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.addr;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "isWt:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.isWrite;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "inflg:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.inflight;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "coalesed:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.coalesced;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "rdy:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.ready;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "hasPush:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.hasPushChan;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "OrderId:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(MemSys_SetWidth) << req.first.cnt;
            }
        }

        // Print Lse suspendReqQueue
        _output_file << std::endl;
        _output_file << "suspendReq: ";
        for (auto& suspendReq : _lse->suspendReqVec)
        {
            auto& req = suspendReq.second;
            _output_file << "v " << suspendReq.first << ":"
                << "addr " << req.addr << ":"
                << "isWt " << req.isWrite << ":"
                << "orderId " << req.cnt;
            _output_file << " ";
        }

        // Print Lse channel
        _output_file << std::endl;
        _output_file << "chan(cnt=" << _lse->chanDataCnt << "): " << std::endl;
        _output_file << "\t";
        for (auto& i : _lse->channel)
        {
            _output_file << "d" << i.value << ":" << "c" << i.cycle << ":" << "l" << i.last;
#ifdef DGSF
            _output_file << ":" << "g" << i.graphSwitch;
#endif
            _output_file << " ";
        }

        _output_file << std::endl;
    }
}

void Debug::memSysPrint(const MemSystem* _memSys)
{
    if (debug_mode == Debug_mode::Print_detail)
    {
        _output_file << std::endl;
        _output_file << "== " << "MemorySystem" << std::endl;

        // Print memSys reqQueue
        auto& reqQueue = _memSys->getReqQueue();
        auto& ackQueue = _memSys->getAckQueue();
        for (size_t bankId = 0; bankId < reqQueue.size(); ++bankId)
        {
            _output_file << "Bank_" << bankId << std::endl;

            // Print reqQueue
            _output_file << std::setw(8) << "reqQueue:" << std::endl;
            _output_file << std::setw(12) << "addr:";
            for (auto& req : reqQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.addr;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "isWt:";
            for (auto& req : reqQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.isWrite;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "inflg:";
            for (auto& req : reqQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.inflight;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "rdy:";
            for (auto& req : reqQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.ready;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "orderId:";
            for (auto& req : reqQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.cnt;
                }
            }
            _output_file << std::endl;


            // Print ackQueue
            _output_file << std::setw(8) << "ackQueue:" << std::endl;
            _output_file << std::setw(12) << "addr:";
            for (auto& req : ackQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.addr;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "isWt:";
            for (auto& req : ackQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.isWrite;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "inflg:";
            for (auto& req : ackQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.inflight;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "rdy:";
            for (auto& req : ackQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.ready;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "orderId:";
            for (auto& req : ackQueue[bankId])
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.cnt;
                }
            }
            _output_file << std::endl;
        }

        // Coaleser
        _output_file << std::endl;
        _output_file << "MemSys Coaleser:" << std::endl;
        const auto& coalescerTable = (*_memSys).getCoalescer().getCoalescerTable();

        for (size_t entryId = 0; entryId < coalescerTable.size(); ++entryId)
        {
            auto& entry = coalescerTable[entryId];
            _output_file << std::setw(8) << "Entry_" << entryId << ":"
                << " valid:" << entry.valid
                << " block_addr:" << entry.blockAddr
                << " rdy:" << entry.ready << std::endl;
                //<< " outstanding:" << entry.outstanding << std::endl;

            auto& reqQueue = entry.coalescerQueue;
            // Print each EntryItem
            _output_file << std::setw(12) << "addr:";
            for (auto& req : reqQueue)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.addr;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "isWt:";
            for (auto& req : reqQueue)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.isWrite;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "inflg:";
            for (auto& req : reqQueue)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.inflight;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "rdy:";
            for (auto& req : reqQueue)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.ready;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "orderId:";
            for (auto& req : reqQueue)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.cnt;
                }
            }

            _output_file << std::endl;
        }

        // SPM
        if (_memSys->spm != nullptr)
        {
            // Print SPM reqQueue
            _output_file << std::endl;
            _output_file << "SPM reqQueue:" << std::endl;

            _output_file << std::setw(12) << "addr:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.first.addr;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "isWt:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.first.isWrite;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "inflg:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.first.inflight;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "rdy:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.first.ready;
                }
            }
        }
        _output_file << std::endl;

        // Cache
        if (_memSys->cache != nullptr)
        {
            // Print cache reqQueue
            _output_file << std::endl;
            _output_file << "Cache reqQueue:" << std::endl;

            const vector<vector<ReqQueueBank>>& reqQueue = _memSys->cache->getReqQueue();

            for (size_t level = 0; level < reqQueue.size(); ++level)
            {
                _output_file << std::setw(8) << "Cache_L" << level + 1 << "_reqQueue:" << std::endl;

                for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)
                {
                    _output_file << std::setw(8) << "Bank_" << bankId << "_L" << level + 1 << ":";
                    _output_file << " total_size: " << _memSys->cache->getReqQueueSizePerBank()[level];
                    _output_file << " curr_size: " << reqQueue[level][bankId].size() << std::endl;

                    // Print each req
                    _output_file << std::setw(12) << "addr:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.first.addr;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "isWt:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            if (req.first.cacheOp == Cache_operation::WRITE)
                            {
                                _output_file << std::setw(MemSys_SetWidth) << "1";
                            }
                            else
                            {
                                _output_file << std::setw(MemSys_SetWidth) << "0";
                            }
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "inflg:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.first.inflight;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "rdy:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.first.ready;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "OrderId:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.first.cnt;
                        }
                    }

                    _output_file << std::endl;
                }

                _output_file << std::endl;
            }

            // Print cache ackQueue
            _output_file << std::endl;
            _output_file << "Cache ackQueue:" << std::endl;

            const vector<vector<deque<CacheReq>>>& ackQueue = _memSys->cache->getAckQueue();

            for (size_t level = 0; level < ackQueue.size(); ++level)
            {
                _output_file << std::setw(8) << "Cache_L" << level + 1 << "_ackQueue:" << std::endl;

                for (size_t bankId = 0; bankId < ackQueue[level].size(); ++bankId)
                {
                    _output_file << std::setw(8) << "Bank_" << bankId << "_L" << level + 1 << ":";
                    _output_file << " total_size: " << _memSys->cache->getAckQueueSizePerBank()[level];
                    _output_file << " curr_size: " << ackQueue[level][bankId].size() << std::endl;

                    // Print each req
                    _output_file << std::setw(12) << "addr:";
                    for (auto& req : ackQueue[level][bankId])
                    {
                        if (req.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.addr;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "isWt:";
                    for (auto& req : ackQueue[level][bankId])
                    {
                        if (req.valid)
                        {
                            if (req.cacheOp == Cache_operation::WRITE)
                            {
                                _output_file << std::setw(MemSys_SetWidth) << "1";
                            }
                            else
                            {
                                _output_file << std::setw(MemSys_SetWidth) << "0";
                            }
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "inflg:";
                    for (auto& req : ackQueue[level][bankId])
                    {
                        if (req.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.inflight;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "rdy:";
                    for (auto& req : ackQueue[level][bankId])
                    {
                        if (req.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.ready;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "OrderId:";
                    for (auto& req : ackQueue[level][bankId])
                    {
                        if (req.valid)
                        {
                            _output_file << std::setw(MemSys_SetWidth) << req.cnt;
                        }
                    }

                    _output_file << std::endl;
                }

                _output_file << std::endl;
            }

            //// Print cache MSHR
            //_output_file << std::endl;
            //_output_file << "Cache MSHR:" << std::endl;

            //const vector<Mshr>& mshr = _memSys->cache->getMshr();

            //for (size_t level = 0; level < mshr.size(); ++level)
            //{
            //    _output_file << std::setw(8) << "Mshr_L" << level + 1 << ":" << std::endl;

            //    for (size_t entryId = 0; entryId < mshr[level].getMshrTable().size(); ++entryId)
            //    {
            //        auto& entry = mshr[level].getMshrTable()[entryId];
            //        _output_file << std::setw(8) << "Entry_" << entryId << ":"
            //            << " valid:" << entry.valid
            //            << " block_addr:" << entry.blockAddr
            //            << " rdy:" << entry.ready
            //            << " outstanding:" << entry.outstanding << std::endl;

            //        auto& reqQueue = entry.mshrQueue;
            //        // Print each EntryItem
            //        _output_file << std::setw(12) << "addr:";
            //        for (auto& req : reqQueue)
            //        {
            //            if (req.valid)
            //            {
            //                _output_file << std::setw(MemSys_SetWidth) << req.addr;
            //            }
            //        }

            //        _output_file << std::endl;
            //        _output_file << std::setw(12) << "isWt:";
            //        for (auto& req : reqQueue)
            //        {
            //            if (req.valid)
            //            {
            //                if (req.cacheOp == Cache_operation::WRITE)
            //                {
            //                    _output_file << std::setw(MemSys_SetWidth) << "1";
            //                }
            //                else
            //                {
            //                    _output_file << std::setw(MemSys_SetWidth) << "0";
            //                }
            //            }
            //        }

            //        _output_file << std::endl;
            //        _output_file << std::setw(12) << "inflg:";
            //        for (auto& req : reqQueue)
            //        {
            //            if (req.valid)
            //            {
            //                _output_file << std::setw(MemSys_SetWidth) << req.inflight;
            //            }
            //        }

            //        _output_file << std::endl;
            //        _output_file << std::setw(12) << "rdy:";
            //        for (auto& req : reqQueue)
            //        {
            //            if (req.valid)
            //            {
            //                _output_file << std::setw(MemSys_SetWidth) << req.ready;
            //            }
            //        }

            //        _output_file << std::endl;
            //        _output_file << std::setw(12) << "orderId:";
            //        for (auto& req : reqQueue)
            //        {
            //            if (req.valid)
            //            {
            //                _output_file << std::setw(MemSys_SetWidth) << req.cnt;
            //            }
            //        }

            //        _output_file << std::endl;
            //    }

            //    _output_file << std::endl;
            //}

            // Print cache reqQueue2Mem
            auto req2Mem = _memSys->cache->getReqQueue2Mem();
            _output_file << "reqQueue2Mem:";
            _output_file << " Total_size: " << REQ_QUEUE_TO_MEM_SIZE;
            _output_file << " Curr_size: " << req2Mem.size() << std::endl;
            for (auto& req : req2Mem)
            {
                if (req.valid)
                {
                    _output_file << std::setw(MemSys_SetWidth) << req.addr;
                }
            }
        }

        _output_file << std::endl;
    }
}

void Debug::printRegistry(const Registry* _registry)
{
    _output_file << std::endl;
    _output_file << "Registry Entry: " << std::endl;
    for (auto& entry : _registry->getRegistryTable())
    {
        _output_file << std::endl;
        _output_file << entry.moduleName;
        if (entry.chanPtr != nullptr)
        {
            _output_file << "\n\t" << "Upstream: ";
            for (auto& upstream : entry.chanPtr->upstream)
            {
                _output_file << upstream->moduleName << "\t";
            }

            _output_file << "\n\t" << "Downstream: ";
            for (auto& downstream : entry.chanPtr->downstream)
            {
                _output_file << downstream->moduleName << "\t";
            }
        }
        _output_file << std::endl;
    }
}

void Debug::printSimNodes(ChanGraph& _chanGraph)
{
    vector<string> simNodes = _chanGraph.bfsTraverseNodes();

    _output_file << std::endl;
    _output_file << "Sim Nodes: " << std::endl;
    for (auto& node : simNodes)
    {
        _output_file << std::endl;
        _output_file << node << "\t" << dynamic_cast<Chan_Node*>(_chanGraph.getNode(node))->node_op << std::endl;
        _output_file << "Upstream: ";

        for (auto& preNode : _chanGraph.getNode(node)->pre_nodes_data)
        {
            _output_file << preNode << "\t";
        }

        //_output_file << "\n\t" << "Downstream: ";
        //for (auto& downstream : entry.chanPtr->downstream)
        //{
        //    _output_file << downstream->moduleName << "\t";
        //}

        _output_file << std::endl;
    }
}

void Debug::printSimInfo(const vector<Channel*>& _chans, const vector<Lc*> _lc)
{
    // Print chan
    for (auto& chanPtr : _chans)
    {
        if (chanPtr->chanType == ChanType::Chan_Lse)
        {
            lsePrint(chanPtr->moduleName, dynamic_cast<Lse*>(chanPtr));
        }
        else
        {
            if (chanPtr->isLoopVar)
            {
                getFile() << std::endl;
                getFile() << "************ Lc: " << chanPtr->masterName << " ***********" << std::endl;
                chanPrint(chanPtr->moduleName, chanPtr);
            }
            else
            {
                chanPrint(chanPtr->moduleName, chanPtr);
            }
        }
    }

    // Print End signal
    getFile() << std::endl;
    getFile() << "*****************  End signal  *****************" << std::endl;
    for (auto& lcPtr : _lc)
    {
        chanPrint(lcPtr->getEnd->moduleName, lcPtr->getEnd);
        getFile() << lcPtr->moduleName << " loopEnd: " << lcPtr->loopEnd << std::endl;
    }
}

void Debug::printGraphScheduler(const GraphScheduler* _graphScheduler)
{
    getFile() << std::endl;
    getFile() << "*****************  Subgraph status *****************" << std::endl;
    getFile() << "Current SubgraphId: " << _graphScheduler->currSubgraphId << std::endl;
    getFile() << std::endl;
    auto& subgraphStatus = _graphScheduler->getSubgraphStatus();
    for (size_t i = 0; i < subgraphStatus.size(); ++i)
    {
        getFile() << "SubgraphId: " << i << "\tisOver: " << subgraphStatus[i] << std::endl;

        getFile() << "Producer: ";
        for (auto& producer : _graphScheduler->subgraphTable.at(i).first)
        {
            getFile() << producer->moduleName << " ";
        }
        getFile() << std::endl;

        getFile() << "Consumer: ";
        for (auto& consumer : _graphScheduler->subgraphTable.at(i).second)
        {
            getFile() << consumer->moduleName << " ";
        }
        getFile() << std::endl;
        getFile() << std::endl;
    }
    getFile() <<"graphTimeoutCnt: " << _graphScheduler->subgraphTimeout <<std::endl;
}

void Debug::throwError(const string errorDescrip, const string fileName, const uint lineNum)
{
    uint clk = DFSim::ClkDomain::getInstance()->getClk();
    std::cout << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    std::cout << "Error: " << errorDescrip << std::endl;
    std::cout << "Clock: " << clk << std::endl;
    std::cout << "Line: " << lineNum << std::endl;
    std::cout << "File name: " << fileName << std::endl;
    DEBUG_ASSERT(false);
}