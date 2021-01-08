#include "./MemSystem.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"

using namespace DFSim;

MemSystem::MemSystem()
{
    reqQueue.resize(MEMSYS_QUEUE_BANK_NUM);
    for (auto& queue : reqQueue)
    {
        queue.resize(MEMSYS_REQ_QUEUE_SIZE_PER_BANK);
    }

    ackQueue.resize(MEMSYS_QUEUE_BANK_NUM);
    for (auto& queue : ackQueue)
    {
        queue.resize(MEMSYS_ACK_QUEUE_SIZE_PER_BANK);
    }

    if (SPM_ENABLE)
    {
        spm = new Spm();
    }
    
    if (CACHE_ENABLE)
    {
        cache = new Cache();
    }

    memDataBus = new MemoryDataBus();

    mem = new DRAMSim::MultiChannelMemorySystem("../DRAMSim2/ini/DDR3_micron_16M_8B_x8_sg15.ini", "../DRAMSim2/ini/system.ini", ".", "example_app", 16384);
    TransactionCompleteCB* read_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_read_complete);
    TransactionCompleteCB* write_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_write_complete);
    //TransactionCompleteCB* read_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_read_complete);
    //TransactionCompleteCB* write_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_write_complete);
    mem->RegisterCallbacks(read_cb, write_cb, power_callback);
}

MemSystem::~MemSystem()
{
    delete mem;
    delete spm;
    delete cache;
    delete memDataBus;

    //for (auto& lse : lseRegistry)
    //{
    //    delete lse;
    //}

}

uint MemSystem::registerLse(Lse* _lse)
{
    lseRegistry.push_back(_lse);
    uint lseId = lseRegistry.size() - 1;  // Index in LseRegistry

    // Add lseReqTable
    vector<Lse*> lseVec( _lse->speedup, _lse );
    lseReqTable.insert(lseReqTable.end(), lseVec.begin(), lseVec.end());

    return lseId;
}

//bool MemSystem::checkReqQueueIsFull()
//{
//    if (reqQueue.size() < MEMSYS_REQ_QUEUE_SIZE)
//    {
//        return false;
//    }
//    else
//    {
//        return true;
//    }
//}

uint MemSystem::addrBias(uint _addr) 
{
    if (DATA_PRECISION % 8 != 0)
    {
        Debug::throwError("DATA_PRECISION is not in multiples of byte!", __FILE__, __LINE__);
    }
    else
    {
        return _addr << (uint)log2(DATA_PRECISION / 8);
    }
}

bool MemSystem::addTransaction(MemReq _req)
{
    //if (reqQueue.size() < MEMSYS_REQ_QUEUE_SIZE)
    //{
    //    reqQueue.push_back(_req);
    //    return true;
    //}
    //else
    //{
    //    return false;
    //}

    bool addSuccess = 0;
    for (size_t i = 0; i < reqQueue.size(); ++i)
    {
        if (reqQueue[i].size() < MEMSYS_REQ_QUEUE_SIZE_PER_BANK)
        {
            _req.memSysAckQueueBankId = i;  // Record the entry of reqQueue in memSystem
            _req.addr = addrBias(_req.addr);
            reqQueue[i].push_back(_req);
            addSuccess = 1;
            break;
        }
    }

    return addSuccess;
}

void MemSystem::getLseReq()
{
    for (size_t i = 0; i < std::min(reqQueue.size(), lseReqTable.size()); ++i)
    {
        Lse* _lse = lseReqTable[reqQueueWritePtr];
        if (_lse->sendReq2Mem())
        {
            reqQueueWritePtr = (++reqQueueWritePtr) % lseReqTable.size();
        }
        else
        {
            break;
        }
    }
}

void MemSystem::sendBack2Lse()
{
    for (auto& queue : ackQueue)
    {
        for (auto& req : queue)
        {
            if (req.valid && req.ready)
            {
                /*lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.ready = 1;
                lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.inflight = 0;*/
                lseRegistry[req.lseId]->ackCallback(req);
                req.valid = 0;  // Clear req
            }
        }
    }
}

void MemSystem::send2Spm()
{
    if (spm != nullptr)
    {
        for (size_t i = 0; i < reqQueue.size(); ++i)
        {
            if (!reqQueue[i].empty())
            {
                MemReq& req = reqQueue[i].front();

                int ackQueueEntry = getAckQueueEntry(ackQueue[i]);
                if (ackQueueEntry != -1)  // If find a empty entry of ackQueue
                {
                    req.memSysAckQueueBankEntryId = ackQueueEntry;
                    if (spm->addTransaction(req))  // Send req to SPM
                    {
                        ackQueue[i][ackQueueEntry] = req;
                        reqQueue[i].pop_front();
                    }
                }
            }
        }
    }
    else
    {
        Debug::throwError("Not define SPM!", __FILE__, __LINE__);
    }
}

void MemSystem::getFromSpm()
{
    if (spm != nullptr)
    {
        vector<MemReq> _req = spm->callBack();
        for (auto& req : _req)
        {
            ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].ready = 1;
            ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].inflight = 0;
        }
    }
    else
    {
        Debug::throwError("Not define SPM!", __FILE__, __LINE__);
    }
}

int MemSystem::getAckQueueEntry(vector<MemReq> _queue)
{
    for (size_t i = 0; i < _queue.size(); ++i)
    {
        if (!_queue[i].valid)
        {
            return i;
        }
    }

    return -1;
}

void MemSystem::send2Cache()
{
    if (cache != nullptr)
    {
        for (size_t i = 0; i < reqQueue.size(); ++i)
        {
            if (!reqQueue[i].empty())
            {
                MemReq& req = reqQueue[i].front();

                int ackQueueEntry = getAckQueueEntry(ackQueue[i]);
                if (ackQueueEntry != -1)  // If find a empty entry of ackQueue
                {
                    req.memSysAckQueueBankEntryId = ackQueueEntry;
                    !!! if (cache->addTransaction(req))  // Send req to cache
                    {
                        ackQueue[i][ackQueueEntry] = req;
                        reqQueue[i].pop_front();
                    }
                }
            }
        }
    }
    else
    {
        Debug::throwError("Not define Cache!", __FILE__, __LINE__);
    }
}

void MemSystem::getFromCache()
{
    if (cache != nullptr)
    {
        vector<MemReq> _req;
        _req = cache->callBack();

        for (auto& req : _req)
        {
            ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].ready = 1;
            ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].inflight = 0;
            //reqQueue[req.memSysAckQueueBankId].cnt = req.cnt;  // Send back cache request order
        }
    }
    else
    {
        Debug::throwError("Not define Cache!", __FILE__, __LINE__);
    }
}

void MemSystem::getReqAckFromMemoryDataBus(vector<MemReq> _reqAcks)
{
    for (auto& reqAck : _reqAcks)
    {
        reqAckStack.push_back(reqAck);
    }
}

void MemSystem::returnReqAck()
{
    if (SPM_ENABLE)
    {
        // Send reqAck back to SPM
        for (auto& reqAck : reqAckStack)
        {
            spm->mem_req_complete(reqAck);
        }
    }

    if (CACHE_ENABLE)
    {
        // Send reqAck back to Cache
        for (auto& reqAck : reqAckStack)
        {
            cache->mem_req_complete(reqAck);
        }
    }

    reqAckStack.clear();
}

void MemSystem::MemSystemUpdate()
{
    if (ClkDomain::getInstance()->checkClkAdd())  // MemorySystem update only when clk updated
    {
        getLseReq();

        if (SPM_ENABLE)
        {
            send2Spm();  // Send req to SPM
            spm->spmUpdate();
            spm->sendReq2Mem(mem);
        }

        if (CACHE_ENABLE)
        {
            send2Cache();
            cache->cacheUpdate();
            cache->sendReq2Mem(mem);
        }

        mem->update();
        getReqAckFromMemoryDataBus(memDataBus->MemoryDataBusUpdate());

        returnReqAck();

        if (SPM_ENABLE)
        {
            getFromSpm();  // Get callback from SPM
        }

        if (CACHE_ENABLE)
        {
            getFromCache();  // Get callback from Cache
        }

        sendBack2Lse();
    }
}


#ifdef DEBUG_MODE
// For Debug
const vector<Lse*>& MemSystem::getLseRegistry() const
{
    return lseRegistry;
}

const vector<deque<MemReq>>& MemSystem::getReqQueue() const
{
    return reqQueue;
}
#endif
