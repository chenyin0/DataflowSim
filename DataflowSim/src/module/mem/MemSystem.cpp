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
    //vector<Lse*> lseVec( _lse->speedup, _lse );
    //lseReqTable.insert(lseReqTable.end(), lseVec.begin(), lseVec.end());
    lseReqTable.insert(lseReqTable.end(), _lse);

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

void MemSystem::resetBankRecorder(uint entryId)
{
    bankRecorder[entryId].valid = 0;
    bankRecorder[entryId].hasRegisteredCoalescer = 0;
    bankRecorder[entryId].reqQueue.clear();

    //for (auto& entry : bankRecorder)
    //{
    //    entry.valid = 0;
    //    entry.hasRegisteredCoalescer = 0;
    //    entry.reqQueue.clear();
    //    //entry.rdPtr = 0;
    //    //entry.hasSent2Mem = 0;
    //}
}

void MemSystem::getLseReq()
{
    bool ptrUpdateLock = 0;
    uint ptr = reqQueueWritePtr;  // Record inital ptr

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
                else  // If corresponding bank is occupied in this round, check whether can be coaleseced with the perior one
                {
                    // 1) A write req not send to coalescer and 2) Any req can't coalesce with a write req
                    if (!req.second.isWrite && !entry.reqQueue.front().isWrite)
                    {
                        if (coalescerFreeEntryNum > 0)
                        {
                            uint addrTag = getAddrTag(req.second.addr);
                            if (addrTag == entry.addrTag)  // Check whether is coalesceable
                            {
                                // If the first time coalescing, occupy a coalscer entry
                                if (!entry.hasRegisteredCoalescer)
                                {
                                    // Assign a new coalescer entry, never coalesce to an existent one
                                    coalescerFreeEntryNum--;
                                    entry.hasRegisteredCoalescer = 1;
                                }

                                if (entry.reqQueue.size() < MEMSYS_COALESCER_SIZY_PER_ENTRY)
                                {
                                    req.second.coalesced = 1;
                                    entry.reqQueue.push_back(req.second);
                                    sendSuccess = 1;
                                }
                            }
                        }
                    }
                }
            }

            if (sendSuccess)
            {
                lseRegistry[req.second.lseId]->setInflight(req.second);
            }
            else
            {
                _lse->memReqBlockCnt++;
            }
        }
    }
    reqQueueWritePtr = (++reqQueueWritePtr) % lseReqTable.size();

    // For Lse parallelism/speedup mode
    if (ClkDomain::checkClkAdd())
    {
        // Send to coalescer
        for (auto& entry : bankRecorder)
        {
            if (entry.valid && entry.hasRegisteredCoalescer)
            {
                //deque<MemReq> _reqQueue;
                //for (auto iter = entry.reqQueue.begin() + entry.rdPtr; iter != entry.reqQueue.end(); ++iter)
                //{
                //    _reqQueue.emplace_back(*iter);
                //}
                coalescer.send2Coalescer(entry.addrTag, entry.reqQueue);
            }
        }

        // Send to memSystem's reqQueue
        for (size_t entryId = 0; entryId < bankRecorder.size(); ++entryId)
        {
            auto& entry = bankRecorder[entryId];
            if (entry.valid)
            {
                //// Only send the req to memSys once
                //if (!entry.hasSent2Mem && entry.rdPtr == 0)
                //{
                //    //auto& req = entry.reqQueue.front();
                //    //req.coalesced = entry.hasRegisteredCoalescer ? 1 : 0;
                //    //if (addTransaction(req))  // Use the front request to represent all the coalesced address
                //    //{
                //    //    entry.hasSent2Mem = 1;
                //    //}
                //}

                auto& req = entry.reqQueue.front();
                req.coalesced = entry.hasRegisteredCoalescer ? 1 : 0;
                if (addTransaction(req))  // Use the front request to represent all the coalesced address
                {
                    //entry.hasSent2Mem = 1;
                    resetBankRecorder(entryId);
                }
            }
        }

        coalescerFreeEntryNum = MEMSYS_COALESCER_ENTRY_NUM - coalescer.getCoalescerOccupiedEntryNum();  // Update coalescer free entry number

        //// Set Lse request inflight
        //for (auto& entry : bankRecorder)
        //{
        //    if (entry.valid && entry.hasSent2Mem)
        //    {
        //        for (auto& req : entry.reqQueue)
        //        {
        //            lseRegistry[req.lseId]->setInflight(req);
        //        }
        //        //for (auto iter = entry.reqQueue.begin() + entry.rdPtr; iter != entry.reqQueue.end(); ++iter)
        //        //{
        //        //    auto& req = *iter;
        //        //    lseRegistry[req.lseId]->setInflight(req);
        //        //}
        //    }
        //}

        //// Update entry rdPtr
        //for (auto& entry : bankRecorder)
        //{
        //    if (entry.valid)
        //    {
        //        entry.rdPtr = entry.reqQueue.size();
        //    }
        //}
    }

    //// Send to coalescer
    //for (auto& entry : bankRecorder)
    //{
    //    if (entry.valid && entry.hasRegisteredCoalescer)
    //    {
    //        deque<MemReq> _reqQueue;
    //        for (auto iter = entry.reqQueue.begin() + entry.rdPtr; iter != entry.reqQueue.end(); ++iter)
    //        {
    //            _reqQueue.emplace_back(*iter);
    //        }
    //        coalescer.send2Coalescer(entry.addrTag, _reqQueue);
    //    }
    //}

    //// Send to memSystem's reqQueue
    //for (auto& entry : bankRecorder)
    //{
    //    if (entry.valid)
    //    {
    //        // Only send the req to memSys once
    //        if (!entry.hasSent2Mem && entry.rdPtr == 0)
    //        {
    //            auto& req = entry.reqQueue.front();
    //            //req.coalesced = entry.hasRegisteredCoalescer ? 1 : 0;
    //            if (addTransaction(req))  // Use the front request to represent all the coalesced address
    //            {
    //                entry.hasSent2Mem = 1;
    //            }
    //        }
    //    }
    //}

    //// Pop Lse request
    //for (auto& entry : bankRecorder)
    //{
    //    if (entry.valid && entry.hasSent2Mem)
    //    {
    //        //for (auto& req : entry.reqQueue)
    //        //{
    //        //    lseRegistry[req.lseId]->sendReq2Mem();
    //        //}
    //        for (auto iter = entry.reqQueue.begin() + entry.rdPtr; iter != entry.reqQueue.end(); ++iter)
    //        {
    //            auto& req = *iter;
    //            lseRegistry[req.lseId]->setInflight(req);
    //        }
    //    }
    //}

    //// Update entry rdPtr
    //for (auto& entry : bankRecorder)
    //{
    //    if (entry.valid)
    //    {
    //        entry.rdPtr = entry.reqQueue.size();
    //    }
    //}
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

    // Debug_yin_04.03
    uint clk_d = ClkDomain::getClk();

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
                        iter = coalescerQueue.erase(iter);
                    }
                    else
                    {
                        iter++;
                    }
                }

                if (coalescerQueue.empty())
                {
                    coalescer.popCoalescerEntry(coalescerEntryId);
                    ackQueue[ptr].pop_front();
                }
            }
            else
            {
                if (!lseRecorder[req.lseId])  // If not Lse contention, pop ackQueue
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

void MemSystem::send2Spm()
{
    //if (spm != nullptr)
    //{
    //    for (size_t i = 0; i < reqQueue.size(); ++i)
    //    {
    //        if (!reqQueue[i].empty())
    //        {
    //            MemReq& req = reqQueue[i].front();

    //            int ackQueueEntry = getAckQueueEntry(ackQueue[i]);
    //            if (ackQueueEntry != -1)  // If find a empty entry of ackQueue
    //            {
    //                req.memSysAckQueueBankEntryId = ackQueueEntry;
    //                if (spm->addTransaction(req))  // Send req to SPM
    //                {
    //                    ackQueue[i][ackQueueEntry] = req;
    //                    reqQueue[i].pop_front();
    //                }
    //            }
    //        }
    //    }
    //}
    //else
    //{
    //    Debug::throwError("Not define SPM!", __FILE__, __LINE__);
    //}
}

void MemSystem::getFromSpm()
{
    /*if (spm != nullptr)
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
    }*/
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

                if (cache->addTransaction(req))  // Send req to cache
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
                if (ack.valid)
                {
                    ackQueue[queueId].emplace_back(ack);
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
    // MemSys -> Lse
    sendBack2Lse();

    // MemSys update in a reverse sequence to avoid data stall 
    if (ClkDomain::getInstance()->checkClkAdd())  // MemorySystem update only when clk updated
    {
        // Send back ack from on-chip memory to memSys
        if (SPM_ENABLE)
        {
            getFromSpm();  // Get callback from SPM
        }

        if (CACHE_ENABLE)
        {
            getFromCache();  // Get callback from Cache
        }

        // DRAM & memory bus
        returnReqAck();
        getReqAckFromMemoryDataBus(memDataBus->MemoryDataBusUpdate());
        mem->update();

        // Send req to on-chip memory
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
    }

    // Get req from Lse to memSys
    getLseReq();
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

const vector<deque<MemReq>>& MemSystem::getAckQueue() const
{
    return ackQueue;
}

const Coalescer& MemSystem::getCoalescer() const
{
    return coalescer;
}
#endif
