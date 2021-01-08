//
// Created by find on 16-7-19.
// Cache architect
// memory address  format:
// |tag|组号 log2(组数)|组内块号log2(mapping_ways)|块内地址 log2(cache line)|
//

#include "../../define/Define.hpp"
#include "Cache.h"
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <cstdio>
#include <time.h>
#include <climits>

#include "../../sim/Debug.h"

using namespace DFSim;

Cache::Cache()
{
    init();
}

/**@arg a_cache_size[] 多级cache的大小设置
 * @arg a_cache_line_size[] 多级cache的line size（block size）大小
 * @arg a_mapping_ways[] 组相连的链接方式*/
void Cache::init() 
{
    // Check whether configuration is correct
    config_check();

    reqQueue.resize(CACHE_MAXLEVEL);
    sendPtr.resize(CACHE_MAXLEVEL);

    for (_u8 i = 0; i < CACHE_MAXLEVEL; ++i)
    {
        cache_size[i] = a_cache_size[i];
        cache_line_size[i] = a_cache_line_size[i];
        cache_line_num[i] = (uint)a_cache_size[i] / a_cache_line_size[i];  // 总的line数 = cache总大小/ 每个line的大小（一般64byte，模拟的时候可配置）
        cache_line_shifts[i] = (uint)log2(a_cache_line_size[i]);
        cache_mapping_ways[i] = a_mapping_ways[i];  // 几路组相联
        cache_set_size[i] = cache_line_num[i] / cache_mapping_ways[i];  // 总共有多少set
        cache_set_shifts[i] = (uint)log2(cache_set_size[i]);  // 其二进制占用位数，同其他shifts
        cache_free_num[i] = cache_line_num[i];  // 空闲块（line）
        cache_bank_num[i] = bankNum[i];
        cache_bank_shifts[i] = (uint)log2(cache_bank_num[i]);

        swap_style[i] = cache_swap_style[i];
        write_strategy[i] = cache_write_strategy[i];
        write_allocate[i] = cache_write_allocate[i];

        reqQueue[i].resize(cache_bank_num[i]);
        for (auto& reqQueueBank : reqQueue[i])
        {
            reqQueueBank.resize(reqQueueSizePerBank[i]);
        }
        sendPtr[i].resize(cache_bank_num[i]);

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
    // 指令数，主要用来在替换策略的时候提供比较的key，在命中或者miss的时候，相应line会更新自己的count为当时的tick_count;
    tick_count = 0;

    //memset(cache_hit_count, 0, sizeof(cache_hit_count));
    //memset(cache_miss_count, 0, sizeof(cache_miss_count));

    //    cache_buf = (_u8 *) malloc(cache_size);
    //    memset(cache_buf, 0, this->cache_size);
        // 为每一行分配空间
    //for (int i = 0; i < 2; ++i) 
    //{
    //    caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
    //    memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
    //}
    ////测试时的默认配置
    //swap_style[0] = Cache_swap_style::CACHE_SWAP_LRU;
    //swap_style[1] = Cache_swap_style::CACHE_SWAP_LRU;
    //re_init();
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
    /**循环查找当前set的所有way（line），通过tag匹配，查看当前地址是否在cache中*/
    uint i;
    uint _tag = getCacheTag(addr, level);  // Debug_yin

    for (i = 0; i < cache_mapping_ways[level]; ++i) 
    {
        //uint taggg = caches[level][set_base + i].tag;  // Debug_yin
        if ((caches[level][set_base + i].flag & CACHE_FLAG_VALID) &&
            (caches[level][set_base + i].tag == _tag /*(addr >> (cache_set_shifts[level] + cache_line_shifts[level]))*/)) 
        {
            return set_base + i;
        }
    }
    return -1;
}

/**获取当前set中可用的line，如果没有，就找到要被替换的块*/
int Cache::get_cache_free_line(uint set_base, int level) 
{
    uint min_count;
    int free_index = -1;  // Initial free index
    /**从当前cache set里找可用的空闲line，可用：脏数据，空闲数据
     * cache_free_num是统计的整个cache的可用块*/
    for (uint i = 0; i < cache_mapping_ways[level]; ++i) 
    {
        if (!(caches[level][set_base + i].flag & CACHE_FLAG_VALID)) 
        {
            if (cache_free_num[level] > 0)
                cache_free_num[level]--;
            return set_base + i;
        }
    }
    /**没有可用line，则执行替换算法
     * lock状态的块如何处理？？*/
    //free_index = -1;
    if (swap_style[level] == Cache_swap_style::CACHE_SWAP_RAND)
    {
        // TODO: 随机替换Lock状态的line后面再改
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
        //如果全部被锁定了，应该会走到这里来。那么强制进行替换。强制替换的时候，需要setline?
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
        //如果原有的cache line是脏数据，标记脏位
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
    //else 
    //{
    //    printf("I should not show\n");
    //}
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

/**将数据写入cache line*/
void Cache::set_cache_line(uint index, uint addr, int level) 
{
    Cache_Line* line = caches[level] + index;
    // 这里每个line的buf和整个cache类的buf是重复的而且并没有填充内容。
//    line->buf = cache_buf + cache_line_size * index;
    // 更新这个line的tag位
    //line->tag = addr >> (cache_set_shifts[level] + cache_line_shifts[level]);
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
    cacheReq.memSysAckQueueBankId = memReq.memSysAckQueueBankId;
    cacheReq.memSysAckQueueBankEntryId = memReq.memSysAckQueueBankEntryId;
    cacheReq.cnt = memReq.cnt;  // For debug

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
    memReq.memSysAckQueueBankId = cacheReq.memSysAckQueueBankId;
    memReq.memSysAckQueueBankEntryId = cacheReq.memSysAckQueueBankEntryId;
    //memReq.cnt = cacheReq.cnt;

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

    if (level == 0)  // L1 cache not coalesces address
    {
        ReqQueueBank& reqQueueBank = reqQueue[level][bankId];

        for (auto& req : reqQueueBank)
        {
            if (!req.first.valid)
            {
                req = make_pair(cacheReq, cache_access_latency[level]);
                //req.first.cnt = reqCnt;  // Record this req's sequence
                //++reqCnt;  // Update req counter
                //std::cout << req.first.inflight << std::endl;  // Debug
                return true;  // Send successfully
            }
        }

        return false;  // Send failed
    }
    else
    {
        if (addrCoaleseCheck(addr, level))
        {
            return true;  // Coalece successfully = send successfully
        }
        else
        {
            ReqQueueBank& reqQueueBank = reqQueue[level][bankId];

            for (auto& req : reqQueueBank)
            {
                if (!req.first.valid)
                {
                    req = make_pair(cacheReq, cache_access_latency[level]);
                    return true;  // Send successfully
                }
            }

            return false;  // Send failed
        }
    }
}

bool Cache::addrCoaleseCheck(const uint addr, const uint level)
{
    //uint setIndex = getCacheSetIndex(addr, level);
    uint bankId = getCacheBank(addr, level);
    const ReqQueueBank& reqQueueBank = reqQueue[level][bankId];

    for (auto& req : reqQueueBank)
    {
        if (req.first.valid 
            && req.first.cacheOp != Cache_operation::WRITEBACK_DIRTY_BLOCK 
            && ((addr >> cache_line_shifts[level]) == (req.first.addr >> cache_line_shifts[level])))
        {
            return true;  // Coalese successfully
        }
    }

    return false;  // Not coalese
}

bool Cache::addTransaction(MemReq _req)
{
    /*bool addSuccess = 0;*/
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

vector<MemReq> Cache::callBack()
{
    vector<MemReq> readyReq;

    if (reqQueue.size() >= 1)
    {
        for (auto& reqQueueBank : reqQueue[0])
        {
            for (auto& req : reqQueueBank)
            {
                if (req.first.valid && req.first.ready)
                {
                    readyReq.push_back(transCacheReq2MemReq(req.first));
                    req.first.valid = 0;  // Clear req after being sent back
                }
            }
        }
    }
    else
    {
        Debug::throwError("Not configure L1 cache!", __FILE__, __LINE__);
    }
    
    return readyReq;
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

void Cache::sendReq2Mem(DRAMSim::MultiChannelMemorySystem* mem)
{
    for (auto& req : reqQueue2Mem)
    {
        if (mem->addTransaction(req.isWrite, req.addr))
        {
            reqQueue2Mem.pop_front();
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
    //uint set_llc = (addr >> cache_line_shifts[llc]) % cache_set_size[llc];
    uint set_llc = getCacheSetIndex(addr, llc);
    uint setBase_llc = set_llc * cache_mapping_ways[llc];

    if (check_cache_hit(setBase_llc, addr, llc) == -1)  // If cache miss, update cacheline of LLC
    {
        uint cache_free_line = get_cache_free_line(setBase_llc, llc);
        set_cache_line(cache_free_line, addr, llc);
    }
}

// Emulate cache access latency
void Cache::reqQueueUpdate()
{
    if (ClkDomain::getInstance()->checkClkAdd())  // Update cache latency only when system clk update
    {
        for (auto& reqQueueLevel : reqQueue)  // Traverse each cache level
        {
            for (auto& reqQueueBank : reqQueueLevel)  // Traverse each bank
            {
                for (auto& req : reqQueueBank)  // Travese each req
                {
                    // Emulate SPM access latency
                    if (req.second != 0)
                    {
                        --req.second;
                    }
                }
            }
        }
    }
}

void Cache::updateReqQueueOfCacheLevel()
{
    for (size_t level = 0; level < CACHE_MAXLEVEL; ++level)  // Traverse each cache level
    {
        for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)  // Traverse each bank
        {
            uint& _sendPtr = sendPtr[level][bankId];
            ReqQueueBank& reqQueueBank = reqQueue[level][bankId];

            for (size_t i = 0; i < reqQueueSizePerBank[level]; ++i)  // Traverse each req of bank
            {
                auto& req = reqQueueBank[_sendPtr];
                if (req.second == 0 && req.first.valid && !req.first.ready)  // Req.second is latency counter
                {
                    uint addr = req.first.addr;
                    //uint set = (addr >> cache_line_shifts[i]) % cache_set_size[i];  // Set index
                    uint set = getCacheSetIndex(addr, level);  // Set index
                    uint set_base = set * cache_mapping_ways[level];  // Cacheline index
                    int setIndex = check_cache_hit(set_base, addr, level);

                    //// debug_yin !!!!!!!!!!!!!!!!!!!!!!!
                    //if (addr == 4020 && req.first.cnt == 2363)
                    //{
                    //    std::cout << ">>>" << std::endl;
                    //    
                    //    for (size_t level = 0; level < reqQueue.size(); ++level)
                    //    {
                    //        std::cout << std::setw(8) << "Cache_L" << level + 1 << "_reqQueue:" << std::endl;

                    //        for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)
                    //        {
                    //            std::cout << std::setw(8) << "Bank_" << bankId << "_L" << level + 1 << ":" << std::endl;

                    //            // Print each req
                    //            std::cout << std::setw(12) << "addr:";
                    //            for (auto& req : reqQueue[level][bankId])
                    //            {
                    //                if (req.first.valid)
                    //                {
                    //                    std::cout << std::setw(5) << req.first.addr;
                    //                }
                    //            }

                    //            std::cout << std::endl;
                    //            std::cout << std::setw(12) << "isWt:";
                    //            for (auto& req : reqQueue[level][bankId])
                    //            {
                    //                if (req.first.valid)
                    //                {
                    //                    if (req.first.cacheOp == Cache_operation::WRITE)
                    //                    {
                    //                        std::cout << std::setw(5) << "1";
                    //                    }
                    //                    else
                    //                    {
                    //                        std::cout << std::setw(5) << "0";
                    //                    }
                    //                }
                    //            }

                    //            std::cout << std::endl;
                    //            std::cout << std::setw(12) << "inflg:";
                    //            for (auto& req : reqQueue[level][bankId])
                    //            {
                    //                if (req.first.valid)
                    //                {
                    //                    std::cout << std::setw(5) << req.first.inflight;
                    //                }
                    //            }

                    //            std::cout << std::endl;
                    //            std::cout << std::setw(12) << "rdy:";
                    //            for (auto& req : reqQueue[level][bankId])
                    //            {
                    //                if (req.first.valid)
                    //                {
                    //                    std::cout << std::setw(5) << req.first.ready;
                    //                }
                    //            }

                    //            std::cout << std::endl;
                    //            std::cout << std::setw(12) << "OrderId:";
                    //            for (auto& req : reqQueue[level][bankId])
                    //            {
                    //                if (req.first.valid)
                    //                {
                    //                    std::cout << std::setw(5) << req.first.cnt;
                    //                }
                    //            }

                    //            std::cout << std::endl;
                    //        }

                    //        std::cout << std::endl;
                    //    }
                    //    std::cout << std::endl;
                    //}

                    if (setIndex != -1)  // Cache hit
                    {
                        if (req.first.cacheOp == Cache_operation::WRITE || req.first.cacheOp == Cache_operation::WRITEBACK_DIRTY_BLOCK)
                        {
                            if (write_strategy[level] == Cache_write_strategy::WRITE_BACK)
                            {
                                caches[level][setIndex].flag |= CACHE_FLAG_DIRTY;  // Write cache -> dirty only in WRITE_BACK mode
                                req.first.ready = 1;
                                req.first.inflight = 0;
                            }
                            else if (write_strategy[level] == Cache_write_strategy::WRITE_THROUGH)
                            {
                                if (level < CACHE_MAXLEVEL - 1)  // If it isn't LLC, send the req to the next level cache
                                {
                                    if (sendReq2CacheBank(req.first, level + 1))  // Send req to next level cache in "write_through"
                                    {
                                        req.first.ready = 1;
                                        req.first.inflight = 0;
                                    }
                                }
                                else  // If it is LLC, send the req to the DRAM
                                {
                                    if (sendReq2reqQueue2Mem(req.first))  // Send req to DRAM in "write_through"
                                    {
                                        req.first.ready = 1;
                                        req.first.inflight = 0;
                                    }
                                }
                            }
                        }
                        else  // If it is cache read or other cache operation
                        {
                            req.first.ready = 1;
                            req.first.inflight = 0;
                        }

                        // Profiling
                        cache_hit_count[level]++;
                    }
                    else  // Cache miss
                    {   
                        if (!req.first.inflight)  // This req hasn't been sent to the next level cache
                        {
                            if (level < CACHE_MAXLEVEL - 1)  // If it isn't LLC, send the req to the next level cache
                            {
                                if (sendReq2CacheBank(req.first, level + 1))  // Send req to next level cache bank
                                {
                                    // Note: Writeback dirty block not need to wait cacheblock return
                                    if (req.first.cacheOp == Cache_operation::WRITEBACK_DIRTY_BLOCK)  
                                    {
                                        req.first.ready = 1;
                                        req.first.inflight = 0;
                                    }
                                    else
                                    {
                                        req.first.inflight = 1;
                                    }
                                }
                            }
                            else  // If it is LLC, send the req to the DRAM
                            {
                                if (sendReq2reqQueue2Mem(req.first))
                                {
                                    // Note: Writeback dirty block not need to wait cacheblock return
                                    if (req.first.cacheOp == Cache_operation::WRITEBACK_DIRTY_BLOCK)
                                    {
                                        req.first.ready = 1;
                                        req.first.inflight = 0;
                                    }
                                    else
                                    {
                                        req.first.inflight = 1;
                                    }
                                }
                            }

                            // Profiling
                            cache_miss_count[level]++;
                        }
                    }

                    _sendPtr = (_sendPtr + 1) % reqQueueSizePerBank[level];  // Update sendPtr (round-robin)
                    break;  // Each cycle only response one req in a bank, emulate bank conflict
                }

                _sendPtr = (_sendPtr + 1) % reqQueueSizePerBank[level];  // Update sendPtr (round-robin)
            }
        }
    }
}

void Cache::updateCacheLine()
{
    for (size_t level = 1; level < CACHE_MAXLEVEL; ++level)  // Traverse each cache level, exclude L1 cache
    {
        for (size_t bankId = 0; bankId < reqQueue[level].size(); ++bankId)  // Traverse each bank
        {
            ReqQueueBank& reqQueueBank = reqQueue[level][bankId];
            for (auto& req : reqQueueBank)
            {
                if (req.first.valid && req.first.ready)
                {
                    uint addr = req.first.addr;

                    // If cache_write and upper cache is write_non_allocate, not update the cacheline of upper level cache, just send back req ack directly
                    if (req.first.cacheOp == Cache_operation::WRITE && write_allocate[level - 1] == Cache_write_allocate::WRITE_NON_ALLOCATE)
                    {
                        uint bankId_upper_level = getCacheBank(addr, level - 1);
                        for (auto& req : reqQueue[level - 1][bankId_upper_level])
                        {
                            if (req.first.valid && !req.first.ready && req.first.inflight && req.first.addr == addr)
                            {
                                req.first.ready = 1;
                                req.first.inflight = 0;
                            }
                        }

                        // Clear req
                        req.first.valid = 0;
                        req.first.ready = 0;
                        req.first.inflight = 0;
                    }
                    else if (req.first.cacheOp == Cache_operation::WRITEBACK_DIRTY_BLOCK)
                    {
                        // Clear req, not update upper level cacheline
                        req.first.valid = 0;
                        req.first.ready = 0;
                        req.first.inflight = 0;
                    }
                    else  // Update the cacheline of upper level cache
                    {
                        if (setCacheBlock(addr, level - 1))
                        {
                            // Clear req
                            req.first.valid = 0;
                            req.first.ready = 0;
                            req.first.inflight = 0;
                        }
                    }
                }
            }
        }
    }
}

// Cache update
void Cache::cacheUpdate()
{
    // Send req from reqQueue to each cache level
    updateReqQueueOfCacheLevel();
    // Update each cache level according to next level req's status (e.g. If L2 hit, update L1 cacheline status)
    updateCacheLine();
    // Update reqQueue latency
    reqQueueUpdate();
}

// For debug
#ifdef DEBUG_MODE
const vector<vector<ReqQueueBank>>& Cache::getReqQueue() const
{
    return reqQueue;
}

const deque<MemReq>& Cache::getReqQueue2Mem() const
{
    return reqQueue2Mem;
}
#endif