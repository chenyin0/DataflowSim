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
        _output_file << "getLast: " << !channel->getLast.empty();
        _output_file << "  ";
        _output_file << "en: " << channel->enable; /*<< std::endl;*/
        //if (channel->isLoopVar)
        //{
        //    _output_file << "  ";
        //    _output_file << "prodLast: " << !channel->produceLast.empty();
        //}
        _output_file << std::endl;

        // Print each chanBuffer
        for (size_t i = 0; i < channel->chanBuffer.size(); ++i)
        {
            _output_file << "chBuf_" << i << "(bp=" << channel->bp[i] << ")(cnt=" << channel->chanBufferDataCnt[i] << "): ";
            for (auto& data : channel->chanBuffer[i])
            {
                if (data.valid)
                {
                    _output_file << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
#ifdef DGSF
                    _output_file << ":" << "g" << data.graphSwitch;
#endif
                    _output_file << " ";
                }
            }
            _output_file << std::endl;
        }

        _output_file << "chan(cnt=" << channel->chanDataCnt << "): ";
        for (auto i : channel->channel)
        {
            _output_file << "d" << channel->value << ":" << "c" << i.cycle << ":" << "l" << i.last;
#ifdef DGSF
            _output_file << ":" << "g" << i.graphSwitch;
#endif
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
        _output_file << "getLast: " << !channel->getLast.empty();
        _output_file << "  ";
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
        for (auto data : channel->matchQueue)
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
        for (auto data : channel->channel)
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
        _output_file << "getLast: " << !_lse->getLast.empty();
        _output_file << "  ";
        _output_file << "en: " << _lse->enable << std::endl;

        // Print each chanBuffer
        for (size_t i = 0; i < _lse->chanBuffer.size(); ++i)
        {
            _output_file << "chanBuffer_" << i << "(bp=" << _lse->bp[i] << "):(cnt=" << _lse->chanBufferDataCnt[i] << "): ";
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
                _output_file << std::setw(5) << req.first.addr;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "isWt:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(5) << req.first.isWrite;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "inflg:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(5) << req.first.inflight;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "rdy:";
        for (auto& req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(5) << req.first.ready;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "hasPush:";
        for (auto req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(5) << req.first.hasPushChan;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "OrderId:";
        for (auto req : _lse->reqQueue)
        {
            if (req.first.valid)
            {
                _output_file << std::setw(5) << req.first.cnt;
            }
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
        _output_file << "MemSys reqQueue:" << std::endl;
        _output_file << std::setw(12) << "addr:";
        for (auto& req : _memSys->getReqQueue())
        {
            if (req.valid)
            {
                _output_file << std::setw(5) << req.addr;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "isWt:";
        for (auto& req : _memSys->getReqQueue())
        {
            if (req.valid)
            {
                _output_file << std::setw(5) << req.isWrite;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "inflg:";
        for (auto& req : _memSys->getReqQueue())
        {
            if (req.valid)
            {
                _output_file << std::setw(5) << req.inflight;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "rdy:";
        for (auto& req : _memSys->getReqQueue())
        {
            if (req.valid)
            {
                _output_file << std::setw(5) << req.ready;
            }
        }

        _output_file << std::endl;
        _output_file << std::setw(12) << "orderId:";
        for (auto& req : _memSys->getReqQueue())
        {
            if (req.valid)
            {
                _output_file << std::setw(5) << req.cnt;
            }
        }

        //_output_file << std::endl;
        //_output_file << std::setw(12) << "hasPush:";
        //for (auto& req : _memSys->getReqQueue())
        //{
        //    if (req.valid)
        //    {
        //        _output_file << std::setw(5) << req.hasPushChan;
        //    }
        //}
        _output_file << std::endl;

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
                    _output_file << std::setw(5) << req.first.addr;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "isWt:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(5) << req.first.isWrite;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "inflg:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(5) << req.first.inflight;
                }
            }

            _output_file << std::endl;
            _output_file << std::setw(12) << "rdy:";
            for (auto& req : _memSys->spm->getReqQueue())
            {
                if (req.first.valid)
                {
                    _output_file << std::setw(5) << req.first.ready;
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

            const vector<vector<ReqQueueBank>> reqQueue = _memSys->cache->getReqQueue();

            for (size_t level = 0; level < reqQueue.size(); ++level)
            {
                _output_file << std::setw(8) << "Cache_L" << level + 1 << "_reqQueue:" << std::endl;

                for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)
                {
                    _output_file << std::setw(8) << "Bank_" << bankId << "_L" << level + 1 << ":" << std::endl;

                    // Print each req
                    _output_file << std::setw(12) << "addr:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(5) << req.first.addr;
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
                                _output_file << std::setw(5) << "1";
                            }
                            else
                            {
                                _output_file << std::setw(5) << "0";
                            }
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "inflg:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(5) << req.first.inflight;
                        }
                    }

                    _output_file << std::endl;
                    _output_file << std::setw(12) << "rdy:";
                    for (auto& req : reqQueue[level][bankId])
                    {
                        if (req.first.valid)
                        {
                            _output_file << std::setw(5) << req.first.ready;
                        }
                    }

                    //_output_file << std::endl;
                    //_output_file << std::setw(12) << "OrderId:";
                    //for (auto& req : reqQueue[level][bankId])
                    //{
                    //    if (req.first.valid)
                    //    {
                    //        _output_file << std::setw(5) << req.first.cnt;
                    //    }
                    //}

                    _output_file << std::endl;
                }

                _output_file << std::endl;
            }
        }

        _output_file << std::endl;
    }
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