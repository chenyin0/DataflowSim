#include "../../define/Define.hpp"
#include "Cache.h"
#include "../../sim/Debug.h"

using namespace DFSim;

Cache::Cache()
{
    init();
}

void Cache::init() 
{
    // Check whether configuration is correct
    config_check();

    reqQueue.resize(CACHE_MAXLEVEL);
    ackQueue.resize(CACHE_MAXLEVEL);
    reqBankConflict.resize(CACHE_MAXLEVEL);
    ackBankConflict.resize(CACHE_MAXLEVEL);
    reqQueueBankPtr.resize(CACHE_MAXLEVEL);
    ackQueueBankPtr.resize(CACHE_MAXLEVEL);
    //sendPtr.resize(CACHE_MAXLEVEL);

    for (_u8 i = 0; i < CACHE_MAXLEVEL; ++i)
    {
        cache_size[i] = a_cache_size[i];
        cache_line_size[i] = a_cache_line_size[i];
        cache_line_num[i] = (uint)a_cache_size[i] / a_cache_line_size[i];  // Total cacheline number = cache_size/cacheline_size
        cache_line_shifts[i] = (uint)log2(a_cache_line_size[i]);
        cache_mapping_ways[i] = a_mapping_ways[i];
        cache_set_size[i] = cache_line_num[i] / cache_mapping_ways[i];  // Total set number
        cache_set_shifts[i] = (uint)log2(cache_set_size[i]);
        //cache_free_num[i] = cache_line_num[i];  // Free cacheline
        cache_bank_num[i] = bankNum[i];
        //cache_bank_shifts[i] = (uint)log2(cache_bank_num[i]);

        swap_style[i] = cache_swap_style[i];
        write_strategy[i] = cache_write_strategy[i];
        write_allocate[i] = cache_write_allocate[i];

        reqQueue[i].resize(cache_bank_num[i]);
        //for (auto& reqQueueBank : reqQueue[i])
        //{
        //    reqQueueBank.resize(reqQueueSizePerBank[i]);
        //}

        ackQueue[i].resize(cache_bank_num[i]);
        for (auto& ackQueueBank : ackQueue[i])
        {
            ackQueueBank.resize(ackQueueSizePerBank[i]);
        }

        reqBankConflict[i].resize(cache_bank_num[i]);
        ackBankConflict[i].resize(cache_bank_num[i]);

        mshr.emplace_back(Mshr(mshrPara[i].first, mshrPara[i].second));  // Initial mshr in each cache level

        //sendPtr[i].resize(cache_bank_num[i]);

        //for (auto& reqQueueBank : reqQueue[i])
        //{
        //    reqQueueBank.resize(reqQueueSizePerBank[i]);
        //}

        // Assign space for each cache line
        caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
        memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
    }

    reqQueue2Mem.resize(cache_bank_num[CACHE_MAXLEVEL - 1]);  // Size equals to LLC bank number

    cache_r_count = 0;
    cache_w_count = 0;
    cache_w_memory_count = 0;
    // Used in cacheline replace£¬update cacheline_count to tick_count when cache hit or miss;
    tick_count = 0;
    srand((unsigned)time(NULL));
}

void Cache::config_check()
{
    if(CACHE_MAXLEVEL < 1)
        Debug::throwError("Parameter \"CACHE_MAXLEVEL\" can not be less than 1", __FILE__, __LINE__);

    if(a_cache_size.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"Cache_size\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (a_cache_line_size.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"Cache_line_size\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (a_mapping_ways.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"Mapping_ways\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (cache_access_latency.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"Cache_access_latency\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (reqQueueSizePerBank.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"ReqQueueSize\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (bankNum.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"BankNum\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (cache_swap_style.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"cache_swap_style\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (cache_write_strategy.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"cache_write_strategy\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

    if (cache_write_allocate.size() != CACHE_MAXLEVEL)
        Debug::throwError("Parameter \"cache_write_allocate\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

}

Cache::~Cache() 
{
    for (uint i = 0; i < CACHE_MAXLEVEL; ++i)
    {
        free(caches[i]);
    }
}

uint Cache::getCacheTag(const uint addr, const uint level)
{
    return addr >> (cache_set_shifts[level] + cache_line_shifts[level]);
}

uint Cache::getCacheSetIndex(const uint addr, const uint level)
{
    return (addr >> cache_line_shifts[level]) % cache_set_size[level];
}

uint Cache::getCacheBank(const uint addr, const uint level)
{
    return (addr >> cache_line_shifts[level]) % cache_bank_num[level];
    //return addr % cache_bank_num[level];
}

uint Cache::getCacheBlockOffset(const uint addr, const uint level)
{
    return addr % cache_line_size[level];
}

int Cache::check_cache_hit(uint set_base, uint addr, int level) 
{
    uint i;
    uint _tag = getCacheTag(addr, level);

    for (i = 0; i < cache_mapping_ways[level]; ++i) 
    {
        if ((caches[level][set_base + i].flag & CACHE_FLAG_VALID) &&
            (caches[level][set_base + i].tag == _tag /*(addr >> (cache_set_shifts[level] + cache_line_shifts[level]))*/)) 
        {
            return set_base + i;
        }
    }
    return -1;
}

int Cache::get_cache_free_line(uint set_base, int level) 
{
    uint min_count;
    int free_index = -1;  // Initial free index
    // Find an available cacheline: a dirty cacheline or a free cacheline
    for (uint i = 0; i < cache_mapping_ways[level]; ++i) 
    {
        if (!(caches[level][set_base + i].flag & CACHE_FLAG_VALID)) 
        {
            //if (cache_free_num[level] > 0)
            //    cache_free_num[level]--;
            return set_base + i;
        }
    }
    // If not an available cacheline, execute cache replacement
    //free_index = -1;
    if (swap_style[level] == Cache_swap_style::CACHE_SWAP_RAND)
    {
        // TODO: Random replacement may evict a locked cacheline
        free_index = rand() % cache_mapping_ways[level];
    }
    else if(swap_style[level] == Cache_swap_style::CACHE_SWAP_LRU ||
            swap_style[level] == Cache_swap_style::CACHE_SWAP_FIFO)
    {
        // LRU/FIFO
        min_count = ULONG_LONG_MAX;
        for (uint j = 0; j < cache_mapping_ways[level]; ++j) 
        {
            if (caches[level][set_base + j].count < min_count &&
                !(caches[level][set_base + j].flag & CACHE_FLAG_LOCK)) 
            {
                min_count = caches[level][set_base + j].count;
                free_index = j;
            }
        }
    }
    if (free_index < 0) 
    {
        // If all cachelines are locked, execute complusorily replace
        min_count = ULONG_LONG_MAX;
        for (uint j = 0; j < cache_mapping_ways[level]; ++j) 
        {
            if (caches[level][set_base + j].count < min_count) 
            {
                min_count = caches[level][set_base + j].count;
                free_index = j;
            }
        }
    }
    //
    if (free_index >= 0) 
    {
        free_index += set_base;
        // If the original cacheline has dirty data, set dirty flag
        if (write_strategy[level] == Cache_write_strategy::WRITE_BACK && caches[level][free_index].flag & CACHE_FLAG_DIRTY)
        {
            // Write back to lower level cache
            if (writeBackDirtyCacheline(caches[level][free_index].tag, free_index, level))
            {
                caches[level][free_index].flag &= ~CACHE_FLAG_DIRTY;
                cache_w_memory_count++;
            }
            else
            {
                return -1;  // Not get a free cacheline
            }
        }
    }

    return free_index;
}

bool Cache::writeBackDirtyCacheline(const uint tag, const uint setIndex, const uint level)
{
    CacheReq cacheReq;
    cacheReq.valid = 1;
    // Assume that the lower level cacheblock is larger than upper level cacheblock 
    // (e.g. assume L2 cache_block_size > L1 cache_block_size, or else a L1 cacheblock writeback will split into more than one L2 write req)
    cacheReq.addr = (tag << (cache_set_shifts[level] + cache_line_shifts[level])) | (setIndex << cache_line_shifts[level]);
    cacheReq.cacheOp = Cache_operation::WRITEBACK_DIRTY_BLOCK;

    if (level < CACHE_MAXLEVEL - 1)
    {
        if (sendReq2CacheBank(cacheReq, level + 1))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(sendReq2reqQueue2Mem(cacheReq))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

/** Write data into cacheline */
void Cache::set_cache_line(uint index, uint addr, int level) 
{
    Cache_Line* line = caches[level] + index;
    // line->buf = cache_buf + cache_line_size * index;
    // Update cacheline tag
    line->tag = getCacheTag(addr, level);
    line->flag = (_u8)~CACHE_FLAG_MASK;
    line->flag |= CACHE_FLAG_VALID;
    line->count = tick_count;
    ++tick_count;
}

bool Cache::setCacheBlock(uint addr, uint level)
{
    uint set = getCacheSetIndex(addr, level);  // Set index
    uint set_base = set * cache_mapping_ways[level];  // Cacheline index

    if (check_cache_hit(set_base, addr, level) == -1)  // If cache miss, replace
    {
        int cacheLineIndex = get_cache_free_line(set_base, level);
        if (cacheLineIndex != -1)
        {
            set_cache_line(cacheLineIndex, addr, level);
            //std::cout << "addr:" << addr << " cacheLineIndex: " << cacheLineIndex << " level:" << level << " cnt:" << (caches[level] + cacheLineIndex)->count << std::endl;  // Debug_yin
            return true;
        }
        else
        {
            return false;
        }
    }
    else  // If cache hit
    {
        return true;
    }
}

int Cache::lock_cache_line(uint line_index, int level) 
{
    caches[level][line_index].flag |= CACHE_FLAG_LOCK;
    return 0;
}

int Cache::unlock_cache_line(uint line_index, int level) 
{
    caches[level][line_index].flag &= ~CACHE_FLAG_LOCK;
    return 0;
}

CacheReq Cache::transMemReq2CacheReq(const MemReq& memReq)
{
    CacheReq cacheReq;
    cacheReq.valid = memReq.valid;
    cacheReq.addr = memReq.addr;
    cacheReq.inflight = memReq.inflight;
    cacheReq.ready = memReq.ready;
    //cacheReq.memSysAckQueueBankId = memReq.memSysAckQueueBankId;
    //cacheReq.memSysAckQueueBankEntryId = memReq.memSysAckQueueBankEntryId;
    cacheReq.cnt = memReq.cnt;  // For debug
    cacheReq.coalesced = memReq.coalesced;
    cacheReq.lseId = memReq.lseId;
    cacheReq.lseReqQueueIndex = memReq.lseReqQueueIndex;

    if (memReq.isWrite)
    {
        cacheReq.cacheOp = Cache_operation::WRITE;
    }
    else
    {
        cacheReq.cacheOp = Cache_operation::READ;
    }

    // TODO: Add lock/unlock

    return cacheReq;
}

MemReq Cache::transCacheReq2MemReq(const CacheReq& cacheReq)
{
    MemReq memReq;
    memReq.valid = cacheReq.valid;
    memReq.addr = cacheReq.addr;
    memReq.inflight = cacheReq.inflight;
    memReq.ready = cacheReq.ready;
    //memReq.memSysAckQueueBankId = cacheReq.memSysAckQueueBankId;
    //memReq.memSysAckQueueBankEntryId = cacheReq.memSysAckQueueBankEntryId;
    //memReq.cnt = cacheReq.cnt;
    memReq.coalesced = cacheReq.coalesced;
    memReq.lseId = cacheReq.lseId;
    memReq.lseReqQueueIndex = cacheReq.lseReqQueueIndex;

    if (cacheReq.cacheOp == Cache_operation::WRITE)
    {
        memReq.isWrite = true;
    }
    else
    {
        memReq.isWrite = false;
    }

    return memReq;
}

bool Cache::sendReq2CacheBank(const CacheReq cacheReq, const uint level)
{
    uint addr = cacheReq.addr;
    uint bankId = getCacheBank(addr, level);
    ReqQueueBank& reqQueueBank = reqQueue[level][bankId];

    if (reqQueueBank.size() < reqQueueSizePerBank[level])
    {
        if (!reqBankConflict[level][bankId])
        {
            reqQueueBank.emplace_back(std::make_pair(cacheReq, cache_access_latency[level]));
            reqBankConflict[level][bankId] = 1;  // Set this bank has been occupied in this round
            return true;
        }
    }

    return false;
}

uint Cache::getCacheBlockId(const uint addr, const uint level)
{
    return addr >> cache_line_shifts[level];
}

//bool Cache::addrCoaleseCheck(const uint addr, const uint level)
//{
//    //uint setIndex = getCacheSetIndex(addr, level);
//    uint bankId = getCacheBank(addr, level);
//    const ReqQueueBank& reqQueueBank = reqQueue[level][bankId];
//
//    for (auto& req : reqQueueBank)
//    {
//        if (req.first.valid 
//            && req.first.cacheOp != Cache_operation::WRITEBACK_DIRTY_BLOCK 
//            && ((addr >> cache_line_shifts[level]) == (req.first.addr >> cache_line_shifts[level])))
//        {
//            return true;  // Coalese successfully
//        }
//    }
//
//    return false;  // Not coalese
//}

bool Cache::addTransaction(MemReq _req)
{
    if (reqQueue.size() > 0)
    {
        return sendReq2CacheBank(transMemReq2CacheReq(_req), 0);
    }
    else
    {
        Debug::throwError("Not configure L1 cache!", __FILE__, __LINE__);
    }

    return false;  // Cache bank is full, send req failed
}

MemReq Cache::callBack(uint ackQueueId)
{ 
    MemReq req;
    auto& ackQueueBank = ackQueue[0][ackQueueId];
    if (!ackQueueBank.empty())
    {
        req = transCacheReq2MemReq(ackQueueBank.front());
        ackQueueBank.pop_front();
    }

    return req;
}

bool Cache::sendReq2reqQueue2Mem(const CacheReq cacheReq)
{
    if (reqQueue2Mem.size() < REQ_QUEUE_TO_MEM_SIZE)
    {
        reqQueue2Mem.push_back(transCacheReq2MemReq(cacheReq));
        return true;
    }
    else
    {
        return false;  // Queue is full
    }
}

//void Cache::sendReq2Mem(DRAMSim::MultiChannelMemorySystem* mem)
//{
//    for (auto& req : reqQueue2Mem)
//    {
//       /* if (mem->addTransaction(req.isWrite, req.addr))
//        {
//            reqQueue2Mem.pop_front();
//            ++memAccessCnt;
//        }
//        else
//        {
//            break;
//        }*/
//
//        if (mem->willAcceptTransaction())
//        {
//            mem->addTransaction(req.isWrite, req.addr);
//            reqQueue2Mem.pop_front();
//            ++memAccessCnt;
//        }
//        else
//        {
//            break;
//        }
//    }
//}

void Cache::sendReq2Mem(dramsim3::MemorySystem* mem)
{
    for (auto& req : reqQueue2Mem)
    {
        /* if (mem->addTransaction(req.isWrite, req.addr))
         {
             reqQueue2Mem.pop_front();
             ++memAccessCnt;
         }
         else
         {
             break;
         }*/

        if (mem->WillAcceptTransaction(req.addr, req.isWrite))
        {
            mem->AddTransaction(req.addr, req.isWrite);
            reqQueue2Mem.pop_front();
            ++memAccessCnt;
        }
        else
        {
            break;
        }
    }
}

// Receive cache block(cacheline) from DRAM 
void Cache::mem_req_complete(MemReq _req)
{
    uint llc = CACHE_MAXLEVEL - 1;
    uint addr = _req.addr;

    // Update cacheline
    uint set_llc = getCacheSetIndex(addr, llc);
    uint setBase_llc = set_llc * cache_mapping_ways[llc];

    if (check_cache_hit(setBase_llc, addr, llc) == -1)  // If cache miss, update cacheline of LLC
    {
        uint cache_free_line = get_cache_free_line(setBase_llc, llc);
        set_cache_line(cache_free_line, addr, llc);
    }

    // Update MSHR of LLC
    uint blockAddr = getCacheBlockId(addr, llc);
    mshr[llc].setMshrEntryReady(blockAddr);
}

// Emulate cache access latency
void Cache::updateReqQueueLatency()
{
    if (ClkDomain::getInstance()->checkClkAdd())  // Update cache latency only when system clk update
    {
        for (auto& reqQueueLevel : reqQueue)  // Traverse each cache level
        {
            for (auto& reqQueueBank : reqQueueLevel)  // Traverse each bank
            {
                for (auto& req : reqQueueBank)  // Travese each req
                {
                    // Emulate Cache access latency
                    if (req.second != 0)
                    {
                        --req.second;
                    }
                }
            }
        }
    }
}

void Cache::resetBankConflictRecorder()
{
    for (size_t level = 0; level < CACHE_MAXLEVEL; ++level)
    {
        uint reqBankNum = reqBankConflict[level].size();
        uint ackBankNum = ackBankConflict[level].size();
        reqBankConflict[level].assign(reqBankNum, 0);
        ackBankConflict[level].assign(ackBankNum, 0);
    }
}

void Cache::updateReqQueue()
{
    for (size_t level = 0; level < CACHE_MAXLEVEL; ++level)  // Traverse each cache level, exclude L1 cache
    {
        for (size_t i = 0; i < reqQueue[level].size(); ++i)  // Traverse each bank, round-robin
        {
            if (mshr[level].seekMshrFreeEntry())  // If there is no free entry remains in Mshr, cache stall
            {
                uint bankId = (reqQueueBankPtr[level] + i) % cache_bank_num[level];
                auto& reqQueueBank = reqQueue[level][bankId];
                if (!reqQueueBank.empty())
                {
                    auto& req = reqQueueBank.front();
                    if (req.second == 0 && !req.first.ready)
                    {
                        uint addr = req.first.addr;
                        //uint set = (addr >> cache_line_shifts[i]) % cache_set_size[i];  // Set index
                        uint set = getCacheSetIndex(addr, level);  // Set index
                        uint set_base = set * cache_mapping_ways[level];  // Cacheline index
                        int setIndex = check_cache_hit(set_base, addr, level);

                        if (setIndex != -1)  // Cache hit
                        {
                            if (req.first.cacheOp == Cache_operation::WRITE || req.first.cacheOp == Cache_operation::WRITEBACK_DIRTY_BLOCK)
                            {
                                if (write_strategy[level] == Cache_write_strategy::WRITE_BACK)
                                {
                                    caches[level][setIndex].flag |= CACHE_FLAG_DIRTY;  // Write cache -> dirty only in WRITE_BACK mode
                                    //req.first.ready = 1;
                                    //req.first.inflight = 0;
                                    //reqQueueBank.pop_front();  // Directly pop this req, not push into ackQueue
                                    req.first.ready = 1;
                                    // TODO: need to add a mechanism to ensure RAW/WAW!
                                }
                                else if (write_strategy[level] == Cache_write_strategy::WRITE_THROUGH)
                                {
                                    // In write_through mode, need to send write_req to next cache level by MSHR
                                    // This behavior likes a cache miss
                                    uint blockAddr = getCacheBlockId(req.first.addr, level);
                                    if (!mshr[level].send2Mshr(blockAddr, req.first))
                                    {
                                        Debug::throwError("Send to MSHR unsuccessfully!", __FILE__, __LINE__);
                                    }
                                    //reqQueueBank.pop_front();
                                    req.first.ready = 1;

                                    //if (level < CACHE_MAXLEVEL - 1)  // If it isn't LLC, send the req to the next level cache
                                    //{
                                    //    if (sendReq2CacheBank(req.first, level + 1))  // Send req to next level cache in "write_through"
                                    //    {
                                    //        //req.first.ready = 1;
                                    //        //req.first.inflight = 0;
                                    //        reqQueueBank.pop_front();  // Directly pop this req, not push into ackQueue
                                    //    }
                                    //}
                                    //else  // If it is LLC, send the req to the DRAM
                                    //{
                                    //    if (sendReq2reqQueue2Mem(req.first))  // Send req to DRAM in "write_through"
                                    //    {
                                    //        //req.first.ready = 1;
                                    //        //req.first.inflight = 0;
                                    //        reqQueueBank.pop_front();  // Directly pop this req, not push into ackQueue
                                    //    }
                                    //}
                                }
                            }
                            else  // If it is cache read or other cache operation
                            {
                                req.first.ready = 1;
                                //req.first.inflight = 0;
                            }

                            // Profiling
                            cache_hit_count[level]++;
                        }
                        else  // Cache miss
                        {
                            uint blockAddr = getCacheBlockId(req.first.addr, level);
                            if (!mshr[level].send2Mshr(blockAddr, req.first))
                            {
                                Debug::throwError("Send to MSHR unsuccessfully!", __FILE__, __LINE__);
                            }
                            reqQueueBank.pop_front();

                            // Profiling
                            cache_miss_count[level]++;
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }
        reqQueueBankPtr[level] = (++reqQueueBankPtr[level]) % cache_bank_num[level];
    }
}

void Cache::updateAckQueue()
{
    // ackqueue priority£º
    // 1. The req hit in Upper cache level£¬and set mshr ready in the same time
    // 2. The ready entry in mshr
    // 3. The ready req in reqQueue.front()

    for (int level = CACHE_MAXLEVEL - 1; level >= 0; --level)
    {
        //*** Pop ackQueue
        if (level > 0)  // If level = 0, pop by callback function!
        {
            vector<CacheReq> cacheReqVec;
            for (size_t i = 0; i < ackQueue[level].size(); ++i)
            {
                uint bankId = (ackQueueBankPtr[level] + i) % cache_bank_num[level];
                auto& ackQueueBank = ackQueue[level][bankId];
                if (!ackQueueBank.empty())
                {
                    cacheReqVec.emplace_back(ackQueueBank.front());
                    ackQueueBank.pop_front();
                }
            }
            ackQueueBankPtr[level] = (++ackQueueBankPtr[level]) % cache_bank_num[level];  // Update ackQueuePtr, round-robin

            for (auto& req : cacheReqVec)
            {
                // Update MSHR in upper cache level, set corresponding MSHR entry ready if MSHR hit
                uint blockAddr = getCacheBlockId(req.addr, level - 1);
                mshr[level - 1].setMshrEntryReady(blockAddr);

                // Update cacheline
                if (!(req.cacheOp == Cache_operation::WRITE && write_allocate[level - 1] == Cache_write_allocate::WRITE_NON_ALLOCATE))
                {
                    // Replace cacheline when cache miss
                    if (!setCacheBlock(req.addr, level - 1))
                    {
                        Debug::throwError("Not update cacheline successfully!", __FILE__, __LINE__);
                    }
                }
            }
        }

        //*** Push ackQueue
        // Respond to MSHR ready reqs
        auto reqVec = mshr[level].peekMshrReadyEntry();
        vector<uint> entryIdVec;
        for (auto& req : reqVec)
        {
            uint bankId = getCacheBank(req.second.addr, level);
            // Write/Writeback OP not push into the ackQueue; (TODO: add a mechanism to ensure RAW/WAW)
            if (/*req.second.cacheOp != Cache_operation::WRITE &&*/ req.second.cacheOp != Cache_operation::WRITEBACK_DIRTY_BLOCK)
            {
                if (!ackBankConflict[level][bankId])
                {
                    auto& ackQueueBank = ackQueue[level][bankId];
                    if (ackQueueBank.size() < ackQueueSizePerBank[level])
                    {
                        req.second.ready = 1;
                        ackQueueBank.emplace_back(req.second);
                        entryIdVec.emplace_back(req.first);
                        ackBankConflict[level][bankId] = 1;
                    }
                }
            }
            else  // Directly clear this req rather than push into the ackQueue
            {
                entryIdVec.emplace_back(req.first);
            }
        }
        if (!reqVec.empty())
        {
            mshr[level].clearMshrEntry(entryIdVec);
        }

        // Ready req in reqQueue 
        for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)
        {
            if (!reqQueue[level][bankId].empty())
            {
                if (reqQueue[level][bankId].front().first.ready && !ackBankConflict[level][bankId])
                {
                    if (ackQueue[level][bankId].size() < ackQueueSizePerBank[level])
                    {
                        ackQueue[level][bankId].emplace_back(reqQueue[level][bankId].front().first);
                        reqQueue[level][bankId].pop_front();
                    }
                }
            }
        }

    }

}

void Cache::sendMshrOutstandingReq()
{
    for (size_t level = 0; level < CACHE_MAXLEVEL; ++level)
    {
        // Get outstanding req without bank conflict
        auto reqVec = mshr[level].getOutstandingReq();
        vector<uint> mshrEntryId;
        if (level < CACHE_MAXLEVEL - 1)  // If it isn't LLC, send the req to the next level cache
        {
            for (auto& req : reqVec)
            {
                if (sendReq2CacheBank(req.second, level + 1))
                {
                    mshrEntryId.emplace_back(req.first);  // Record corresponding the entryId of bank-conflict-free reqs in MSHR
                }
            }
        }
        else  // If it is LLC, send the req to the DRAM
        {
            for (auto& req : reqVec)
            {
                if (sendReq2reqQueue2Mem(req.second))  // Send req to DRAM 
                {
                    mshrEntryId.emplace_back(req.first);  // Record corresponding the entryId of bank-conflict-free reqs in MSHR
                }
            }
        }

        // Clear mshr outstanding flag
        mshr[level].clearOutstandingFlag(mshrEntryId);
    }
}

// Cache update
void Cache::cacheUpdate()
{
    // Reset req/ack bankConflictRecorder
    resetBankConflictRecorder();
    // Receive ready reqs
    updateAckQueue();
    // Send MSHR req to next level cache
    sendMshrOutstandingReq();
    // Check reqs hit/miss status, and send to MSHR
    updateReqQueue();
    // Update reqQueue latency
    updateReqQueueLatency();
}


//// For debug
//#ifdef DEBUG_MODE
//const vector<vector<ReqQueueBank>>& Cache::getReqQueue() const
//{
//    return DFSim::Cache::reqQueue;
//}
//
//const vector<vector<deque<CacheReq>>>& Cache::getAckQueue() const
//{
//    return DFSim::Cache::ackQueue;
//}
//
//const deque<MemReq>& Cache::getReqQueue2Mem() const
//{
//    return reqQueue2Mem;
//}
//
//const vector<Mshr>& Cache::getMshr() const
//{
//    return mshr;
//}
//#endif