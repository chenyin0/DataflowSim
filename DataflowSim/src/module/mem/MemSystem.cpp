#include "./MemSystem.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"

using namespace DFSim;

MemSystem::MemSystem()
{
    reqQueue.resize(MEMSYS_QUEUE_BANK_NUM);
    //for (auto& queue : reqQueue)
    //{
    //    queue.resize(MEMSYS_REQ_QUEUE_SIZE_PER_BANK);
    //}

    ackQueue.resize(MEMSYS_QUEUE_BANK_NUM);
    //for (auto& queue : ackQueue)
    //{
    //    queue.resize(MEMSYS_ACK_QUEUE_SIZE_PER_BANK);
    //}

    if (SPM_ENABLE)
    {
        spm = new Spm();
        bankRecorder.resize(SPM_BANK_NUM);
    }
    
    if (CACHE_ENABLE)
    {
        cache = new Cache();
        bankRecorder.resize(CACHE_BANK_NUM_L1);
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
    bool addSuccess = 0;
    uint bankId = getBankId(_req.addr);
    if (reqQueue[bankId].size() < MEMSYS_REQ_QUEUE_SIZE_PER_BANK)
    {
        reqQueue[bankId].push_back(_req);
        return true;
    }
    else
    {
        Debug::throwError("Try to push req into a full reqQueue", __FILE__, __LINE__);
    }

    return false;
}

uint MemSystem::getBankId(uint _addr)
{
    if (CACHE_ENABLE)
    {
        return cache->getCacheBank(_addr, 0);
    }

    // Should not execute here
    Debug::throwError("Not define which bank", __FILE__, __LINE__);
}

uint MemSystem::getAddrTag(uint _addr)
{
    if (CACHE_ENABLE)
    {
        return cache->getCacheBlockId(_addr, 0);
    }

    // Should not execute here
    Debug::throwError("Not define address tag for coalescing", __FILE__, __LINE__);
}

void MemSystem::resetBankRecorder()
{
    for (auto& entry : bankRecorder)
    {
        entry.valid = 0;
        entry.hasRegisteredCoalescer = 0;
        entry.reqQueue.clear();
    }
}

void MemSystem::getLseReq()
{
    //for (size_t i = 0; i < std::min(reqQueue.size(), lseReqTable.size()); ++i)
    //{
    //    Lse* _lse = lseReqTable[reqQueueWritePtr];
    //    if (_lse->sendReq2Mem())
    //    {
    //        reqQueueWritePtr = (++reqQueueWritePtr) % lseReqTable.size();
    //    }
    //    else
    //    {
    //        break;
    //    }
    //}

    //vector<pair<bool, uint>> bankOccupied(reqQueue.size());  // <bool, uint> = <valid, reqRecorder index>
    //bankOccupied.assign(bankOccupied.size(), make_pair(0, 0));  // Clear bankOccupied, clear flag and address
    //vector<deque<pair<uint, Lse*>>> reqRecorder;  // Record each req in one round

    bool ptrUpdateLock = 0;
    uint ptr = reqQueueWritePtr;  // Record inital ptr
    resetBankRecorder();
    uint coalescerFreeEntryNum = MEMSYS_COALESCER_ENTRY_NUM - coalescer.getCoalescerOccupiedEntryNum();

    // Generate all the valid req in this round
    for (size_t i = 0; i < lseReqTable.size(); ++i)
    {
        ptr = (ptr + i) % lseReqTable.size();
        Lse* _lse = lseReqTable[ptr];
        auto req = _lse->peekReqQueue();
        if (req.first)
        {
            bool sendSuccess = 0;
            uint bankId = getBankId(req.second.addr);
            if (reqQueue[bankId].size() < MEMSYS_REQ_QUEUE_SIZE_PER_BANK)
            {
                auto& entry = bankRecorder[bankId];
                if (!entry.valid)  // If this entry has not been visited in this round
                {
                    entry.valid = 1;
                    entry.addrTag = getAddrTag(req.second.addr);
                    entry.reqQueue.push_back(req.second);
                    sendSuccess = 1;
                }
                else
                {
                    if (coalescerFreeEntryNum > 0)
                    {
                        if (entry.addrTag == getAddrTag(req.second.addr))  // Check whether is coalesceable
                        {
                            // If the first time coalescing, occupy a coalscer entry
                            if (!entry.hasRegisteredCoalescer)
                            {
                                coalescerFreeEntryNum--;
                            }
                            entry.hasRegisteredCoalescer = 1;

                            if (entry.reqQueue.size() < MEMSYS_COALESCER_SIZY_PER_ENTRY)
                            {
                                entry.reqQueue.push_back(req.second);
                                sendSuccess = 1;
                            }
                        }
                    }
                }
            }

            if (!sendSuccess)
            {
                _lse->memReqBlockCnt++;
            }
        }
        else
        {
            if (!ptrUpdateLock)
            {
                reqQueueWritePtr = ptr;
                ptrUpdateLock = 1;  // Only update ptr once in each round
            }
        }
    }

    // Pop Lse request
    for (auto& entry : bankRecorder)
    {
        if (entry.valid)
        {
            for (auto& req : entry.reqQueue)
            {
                lseRegistry[req.lseId]->sendReq2Mem();
            }
        }
    }

    // Send to coalescer
    for (auto& entry : bankRecorder)
    {
        if (entry.valid && entry.hasRegisteredCoalescer)
        {
            for (auto& req : entry.reqQueue)
            {
                coalescer.send2Coalescer(entry.addrTag, entry.reqQueue);
            }
        }
    }

    // Send to reqQueue
    for (auto& entry : bankRecorder)
    {
        if (entry.valid)
        {
            addTransaction(entry.reqQueue.front());  // Use the front request to represent all the coalesced address
        }
    }
}

//void MemSystem::writeAck2Lse(MemReq _req)
//{
//    lseRegistry[_req.lseId]->ackCallback(_req);
//}

void MemSystem::sendBack2Lse()
{
    // Add a LSE recorder in each cycle
    // Round-robin each ackQueue
    // Erase non-conflict req in coalescing table
    // Only when corresponding coalescing table entry has empty, pop the ackQueue

    deque<bool> lseRecorder(lseRegistry.size());
    uint ptr = ackQueueReadPtr;

    for (size_t i = 0; i < ackQueue.size(); ++i)
    {
        ptr = (ptr + i) % ackQueue.size();
        if (!ackQueue[ptr].empty())
        {
            auto& req = ackQueue[ptr].front();
            if (req.coalesced)
            {
                uint addrTag = getAddrTag(req.addr);
                uint coalescerEntryId = coalescer.searchCoalescer(addrTag);
                auto& coalescerQueue = coalescer.coalescerTable[coalescerEntryId].coalescerQueue;
                for (auto iter = coalescerQueue.begin(); iter != coalescerQueue.end();)
                {
                    auto& lseId = iter->lseId;
                    if (!lseRecorder[lseId])
                    {
                        lseRecorder[lseId] = 1;
                        lseRegistry[lseId]->ackCallback(*iter);
                        coalescerQueue.erase(iter);

                        if (coalescerQueue.empty())
                        {
                            coalescer.popCoalescerEntry(coalescerEntryId);
                            ackQueue[ptr].pop_front();
                        }
                    }
                    else
                    {
                        iter++;
                    }
                }
            }
            else
            {
                if (!lseRecorder[req.lseId])  // If not Lse content, pop ackQueue
                {
                    lseRecorder[req.lseId] = 1;
                    lseRegistry[req.lseId]->ackCallback(req);  // Write back ack to Lse
                    ackQueue[ptr].pop_front();
                }
            }
        }
    }


    //for (auto& queue : ackQueue)
    //{
    //    for (auto& req : queue)
    //    {
    //        if (req.valid && req.ready)
    //        {
    //            /*lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.ready = 1;
    //            lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.inflight = 0;*/
    //            lseRegistry[req.lseId]->ackCallback(req);
    //            req.valid = 0;  // Clear req
    //        }
    //    }
    //}
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

                //int ackQueueEntry = getAckQueueEntry(ackQueue[i]);
                //if (ackQueueEntry != -1)  // If find a empty entry of ackQueue
                //{
                //    req.memSysAckQueueBankEntryId = ackQueueEntry;
                //    !!! if (cache->addTransaction(req))  // Send req to cache
                //    {
                //        ackQueue[i][ackQueueEntry] = req;
                //        reqQueue[i].pop_front();
                //    }
                //}

                !!!if (cache->addTransaction(req))  // Send req to cache
                {
                    reqQueue[i].pop_front();
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
    //if (cache != nullptr)
    //{
    //    vector<MemReq> _req;
    //    _req = cache->callBack();

    //    for (auto& req : _req)
    //    {
    //        ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].ready = 1;
    //        ackQueue[req.memSysAckQueueBankId][req.memSysAckQueueBankEntryId].inflight = 0;
    //        //reqQueue[req.memSysAckQueueBankId].cnt = req.cnt;  // Send back cache request order
    //    }
    //}
    //else
    //{
    //    Debug::throwError("Not define Cache!", __FILE__, __LINE__);
    //}

    if (cache != nullptr)
    {
        for (size_t queueId = 0; queueId < ackQueue.size(); ++queueId)
        {
            if (ackQueue[queueId].size() < MEMSYS_ACK_QUEUE_SIZE_PER_BANK)
            {
                auto ack = cache->callBack(queueId);
                if (ack.first)
                {
                    ackQueue[queueId].emplace_back(ack.second);
                }
            }
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
