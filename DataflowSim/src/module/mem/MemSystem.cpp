#include "./MemSystem.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"
// #include "../../util/util.hpp"  // Debug_yin_04.12

using namespace DFSim;

MemSystem::MemSystem()
{
    reqQueue.resize(Global::memSystem_queue_bank_num);
    ackQueue.resize(Global::memSystem_queue_bank_num);

    if (Global::spm_enable)
    {
        spm = new Spm();
        bankRecorder.resize(Global::spm_bank_num);
    }

    if (Global::cache_enable)
    {
        cache = new Cache();
        bankRecorder.resize(Global::cache_bank_num_L1);
    }

    memDataBus = new MemoryDataBus();

    //// DRAMSim2
    // mem = new DRAMSim::MultiChannelMemorySystem("../DRAMSim2/ini/DDR3_micron_16M_8B_x8_sg15.ini", "../DRAMSim2/ini/system.ini", ".", "example_app", 16384);
    // TransactionCompleteCB* read_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_read_complete);
    // TransactionCompleteCB* write_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_write_complete);
    ////TransactionCompleteCB* read_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_read_complete);
    ////TransactionCompleteCB* write_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_write_complete);
    // mem->RegisterCallbacks(read_cb, write_cb, power_callback);
    // mem->setCPUClockSpeed(FREQ)

    // DRAMSim3
    /*dramsim3::Config config("configs/HBM1_4Gb_x128.ini", ".");*/
#ifdef WINDOWS
    const std::string &config_file = "../DRAMSim3/configs/HBM1_4Gb_x128.ini";
#endif
#ifdef LINUX
    const std::string &config_file = "./DataflowSim/lib/dramsim3/configs/HBM1_4Gb_x128.ini";
#endif
    std::function<void(uint64_t)> read_callback = std::bind(&MemoryDataBus::mem_read_complete, memDataBus, std::placeholders::_1);
    std::function<void(uint64_t)> write_callback = std::bind(&MemoryDataBus::mem_write_complete, memDataBus, std::placeholders::_1);
    mem = dramsim3::GetMemorySystem(config_file, ".", read_callback, write_callback);

    auto tCK_ = mem->GetTCK();
    dramsimClkFreqHz = (uint64_t)(1.0 / (tCK_ * 1e-9));
    hostClkFreqHz = Global::freq;
}

MemSystem::~MemSystem()
{
    delete mem;
    delete spm;
    delete cache;
    delete memDataBus;
}

uint64_t MemSystem::registerLse(Lse *_lse)
{
    lseRegistry.push_back(_lse);
    uint64_t lseId = lseRegistry.size() - 1; // Index in LseRegistry

    // Add lseReqTable
    // vector<Lse*> lseVec( _lse->speedup, _lse );
    // lseReqTable.insert(lseReqTable.end(), lseVec.begin(), lseVec.end());
    lseReqTable.insert(lseReqTable.end(), _lse);

    return lseId;
}

uint64_t MemSystem::addrBias(uint64_t _addr)
{
    if (Global::data_precision % 8 != 0)
    {
        Debug::throwError("DATA_PRECISION is not in multiples of byte!", __FILE__, __LINE__);
    }
    else
    {
        return _addr << (uint64_t)log2(Global::data_precision / 8);
    }
}

bool MemSystem::addTransaction(MemReq _req)
{
    uint64_t bankId = getBankId(_req.addr);
    if (reqQueue[bankId].size() < Global::memSystem_req_queue_size_per_bank)
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

uint64_t MemSystem::getBankId(uint64_t _addr)
{
    if (Global::cache_enable)
    {
        return cache->getCacheBank(_addr, 0);
    }

    // Should not execute here
    Debug::throwError("Not define which bank", __FILE__, __LINE__);
}

uint64_t MemSystem::getAddrTag(uint64_t _addr)
{
    if (Global::cache_enable)
    {
        return cache->getCacheBlockId(_addr, 0);
    }

    // Should not execute here
    Debug::throwError("Not define address tag for coalescing", __FILE__, __LINE__);
}

void MemSystem::resetBankRecorder(uint64_t entryId)
{
    bankRecorder[entryId].valid = 0;
    bankRecorder[entryId].hasRegisteredCoalescer = 0;
    bankRecorder[entryId].reqQueue.clear();
}

void MemSystem::getLseReq()
{
    bool ptrUpdateLock = 0;
    // Debug_yin_04.12
    uint64_t ptr = reqQueueWritePtr; // Record inital ptr
    // uint64_t ptr = Util::uRandom(0, lseReqTable.size() - 1);

    // Generate all the valid req in this round
    for (size_t i = 0; i < lseReqTable.size(); ++i)
    {
        ptr = (ptr + i) % lseReqTable.size();
        Lse *_lse = lseReqTable[ptr];
        auto req = _lse->peekReqQueue();
        if (req.first)
        {
            bool sendSuccess = 0;
            uint64_t bankId = getBankId(req.second.addr);
            if (reqQueue[bankId].size() < Global::memSystem_req_queue_size_per_bank)
            {
                auto &entry = bankRecorder[bankId];
                if (!entry.valid) // If this entry has not been visited in this round
                {
                    entry.valid = 1;
                    entry.addrTag = getAddrTag(req.second.addr);
                    entry.reqQueue.push_back(req.second);
                    sendSuccess = 1;
                }
                else // If corresponding bank is occupied in this round, check whether can be coaleseced with the perior one
                {
                    // 1) A write req not send to coalescer and 2) Any req can't coalesce with a write req
                    if (!req.second.isWrite && !entry.reqQueue.front().isWrite)
                    {
                        if (coalescerFreeEntryNum > 0)
                        {
                            uint64_t addrTag = getAddrTag(req.second.addr);
                            if (addrTag == entry.addrTag) // Check whether is coalesceable
                            {
                                // If the first time coalescing, occupy a coalscer entry
                                if (!entry.hasRegisteredCoalescer)
                                {
                                    // Assign a new coalescer entry, never coalesce to an existent one
                                    coalescerFreeEntryNum--;
                                    entry.hasRegisteredCoalescer = 1;
                                }

                                if (entry.reqQueue.size() < Global::memSys_coalescer_size_per_entry)
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
                //_lse->setInflight(req.second);
            }
#ifdef DEBUG_MODE
            else
            {
                _lse->memReqBlockCnt++;
            }
#endif
        }
    }
    reqQueueWritePtr = (++reqQueueWritePtr) % lseReqTable.size();

    // For Lse parallelism/speedup mode
    if (ClkDomain::checkClkAdd())
    {
        // Send to coalescer
        for (auto &entry : bankRecorder)
        {
            if (entry.valid && entry.hasRegisteredCoalescer)
            {
                coalescer.send2Coalescer(entry.addrTag, entry.reqQueue);
            }
        }

        // Send to memSystem's reqQueue
        for (size_t entryId = 0; entryId < bankRecorder.size(); ++entryId)
        {
            auto &entry = bankRecorder[entryId];
            if (entry.valid)
            {
                auto &req = entry.reqQueue.front();
                req.coalesced = entry.hasRegisteredCoalescer ? 1 : 0;
                if (addTransaction(req)) // Use the front request to represent all the coalesced address
                {
                    // entry.hasSent2Mem = 1;
                    resetBankRecorder(entryId);
                }
            }
        }

        coalescerFreeEntryNum = Global::memSys_coalescer_entry_num - coalescer.getCoalescerOccupiedEntryNum(); // Update coalescer free entry number
    }
}

void MemSystem::sendBack2Lse()
{
    // Add a LSE recorder in each cycle
    // Round-robin each ackQueue
    // Erase non-conflict req in coalescing table
    // Only when corresponding coalescing table entry has empty, pop the ackQueue

    deque<bool> lseRecorder(lseRegistry.size());
    uint64_t ptr = ackQueueReadPtr;

    for (size_t i = 0; i < ackQueue.size(); ++i)
    {
        ptr = (ptr + i) % ackQueue.size();
        if (!ackQueue[ptr].empty())
        {
            auto &req = ackQueue[ptr].front();
            if (req.coalesced)
            {
                uint64_t addrTag = getAddrTag(req.addr);
                uint64_t coalescerEntryId = coalescer.searchCoalescer(addrTag);
                auto &coalescerQueue = coalescer.coalescerTable[coalescerEntryId].coalescerQueue;
                for (auto iter = coalescerQueue.begin(); iter != coalescerQueue.end();)
                {
                    auto &lseId = iter->lseId;
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
                if (!lseRecorder[req.lseId]) // If not Lse contention, pop ackQueue
                {
                    lseRecorder[req.lseId] = 1;
                    lseRegistry[req.lseId]->ackCallback(req); // Write back ack to Lse
                    ackQueue[ptr].pop_front();
                }
            }
        }
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

void MemSystem::send2Spm()
{
    // if (spm != nullptr)
    //{
    //     for (size_t i = 0; i < reqQueue.size(); ++i)
    //     {
    //         if (!reqQueue[i].empty())
    //         {
    //             MemReq& req = reqQueue[i].front();

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
    // else
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
        //  Debug_yin_22.06.10 annotate the code below
        // if (CACHE_ALL_HIT)
        //{
        //    for (size_t queueId = 0; queueId < ackQueue.size(); ++queueId)
        //    {
        //        if (ackQueue[queueId].size() < MEMSYS_ACK_QUEUE_SIZE_PER_BANK)
        //        {
        //            auto ack = cache->callBack(queueId);
        //            if (ack.valid)
        //            {
        //                ackQueue[queueId].emplace_back(ack);
        //            }
        //        }
        //    }
        //}
        // else
        //{
        //    for (size_t i = 0; i < reqQueue.size(); ++i)
        //    {
        //        if (!reqQueue[i].empty())
        //        {
        //            MemReq& req = reqQueue[i].front();
        //            if (cache->addTransaction(req))  // Send req to cache
        //            {
        //                reqQueue[i].pop_front();
        //            }
        //        }
        //    }
        //}

        for (size_t i = 0; i < reqQueue.size(); ++i)
        {
            if (!reqQueue[i].empty())
            {
                MemReq &req = reqQueue[i].front();
                if (Global::cache_all_hit)
                {
                    if (ackQueue[i].size() < Global::memSystem_ack_queue_size_per_bank)
                    {
                        ackQueue[i].emplace_back(req);
                        reqQueue[i].pop_front();
                        cache->updateCacheAccessCnt();
                    }
                }
                else
                {
                    if (cache->addTransaction(req)) // Send req to cache
                    {
                        reqQueue[i].pop_front();
                        cache->updateCacheAccessCnt();
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
        for (size_t queueId = 0; queueId < ackQueue.size(); ++queueId)
        {
            if (ackQueue[queueId].size() < Global::memSystem_ack_queue_size_per_bank)
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
    for (auto &reqAck : _reqAcks)
    {
        reqAckStack.push_back(reqAck);
    }
}

void MemSystem::returnReqAck()
{
    if (Global::spm_enable)
    {
        // Send reqAck back to SPM
        for (auto &reqAck : reqAckStack)
        {
            spm->mem_req_complete(reqAck);
        }
    }

    if (Global::cache_enable)
    {
        // Send reqAck back to Cache
        for (auto &reqAck : reqAckStack)
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
    if (ClkDomain::getInstance()->checkClkAdd()) // MemorySystem update only when clk updated
    {
        // Send back ack from on-chip memory to memSys
        if (Global::spm_enable)
        {
            getFromSpm(); // Get callback from SPM
        }

        if (Global::cache_enable)
        {
            getFromCache(); // Get callback from Cache
        }

        // DRAM & memory bus
        returnReqAck();
        getReqAckFromMemoryDataBus(memDataBus->MemoryDataBusUpdate());

        // mem->update();  // Deprecated for DRAMSim2
        dramUpdate(); // Update DRAMSim3

        // Send req to on-chip memory
        if (Global::spm_enable)
        {
            send2Spm(); // Send req to SPM
            spm->spmUpdate();
            spm->sendReq2Mem(mem);
        }

        if (Global::cache_enable)
        {
            send2Cache();
            cache->cacheUpdate();
            cache->sendReq2Mem(mem);
        }
    }

    // Get req from Lse to memSys
    getLseReq();
}

const uint64_t MemSystem::getMemAccessCnt() const
{
    uint64_t memAccessCnt = 0;
    if (cache != nullptr)
    {
        memAccessCnt += cache->getMemAccessCnt();
    }

    if (spm != nullptr)
    {
        memAccessCnt += spm->getMemAccessCnt();
    }

    return memAccessCnt;
}

void MemSystem::dramUpdate()
{
    // For 1:1 ratios
    if (hostClkFreqHz == dramsimClkFreqHz)
    {
        mem->ClockTick();
        return;
    }

    // Update dramCnt.
    dramCnt += dramsimClkFreqHz;

    while (hostCnt < dramCnt)
    {
        hostCnt += hostClkFreqHz;
        mem->ClockTick();
    }

    if (hostCnt == dramCnt)
    {
        hostCnt = 0;
        dramCnt = 0;
    }
}

// #ifdef DEBUG_MODE
//// For Debug
// const vector<Lse*>& MemSystem::getLseRegistry() const
//{
//     return lseRegistry;
// }
//
// const vector<deque<MemReq>>& MemSystem::getReqQueue() const
//{
//     return reqQueue;
// }
//
// const vector<deque<MemReq>>& MemSystem::getAckQueue() const
//{
//     return ackQueue;
// }
//
// const Coalescer& MemSystem::getCoalescer() const
//{
//     return coalescer;
// }
//
// const uint64_t& MemSystem::getMemAccessCnt() const
//{
//     return memAccessCnt;
// }
// #endif
