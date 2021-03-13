#pragma once

/*
Cache Module

1. Multi-hierarchy (L1, L2, ... Ln)
2. Operation: write/read, lock/unlock
3. Non-blocking (MSHR)


Req flow:
1. Send req:
    reqQueue_MemSystem -> reqQueue_L1 (hit/miss check, MSHR coalescing) -> reqQueue_L2 (hit/miss check, MSHR coalescing) -> ... -> DRAM

2. Receive req:
    DRAM -> reqQueue Ln (update cacheline) -> ... -> reqQueue L1 (update cacheline) -> reqQueue_MemSystem


Address bit fields:
  32bit address space = 2^32words
    MSB                                    LSB
    Tag + Set(Index) + Block_offset(Cacheline)   Lower bits of Set is bank_index    

*/

#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "../DataType.h"
#include "../EnumType.h"
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
#include "Mshr.h"

typedef unsigned char _u8;
//typedef unsigned long long uint;

namespace DFSim
{
    //struct CacheReq;
    //enum class Cache_swap_style;
    //enum class Cache_write_strategy;
    //enum class Cache_write_allocate;

    using ReqQueueBank = deque<pair<CacheReq, uint>>;  // Emulate bank conflict, <CacheReq, cache_access_latency[level]>

    class Cache_Line 
    {
    public:
        uint tag;
        /**计数，FIFO里记录最一开始的访问时间，LRU里记录上一次访问的时间*/
        union 
        {
            uint count;
            uint lru_count;
            uint fifo_count;
        };
        _u8 flag;
        //_u8* buf;
    };

    class Cache 
    {
    public:
        Cache();
        ~Cache();

        // Interface function for MemSystem
        bool addTransaction(MemReq _req);  // Add transaction to Cache
        MemReq callBack(uint ackQueueId);  // Get memory access results from Cache

        // Interface func with DRAM
        void sendReq2Mem(DRAMSim::MultiChannelMemorySystem* mem);  // Send memory req to DRAM
        void mem_req_complete(MemReq _req);

        void cacheUpdate();
        //void reqQueueUpdate();
        // Profiling
        double get_miss_rate(int level)
        {
            return 100.0 * cache_miss_count[level] / (cache_miss_count[level] + cache_hit_count[level]);
        }

    private:
        void init();
        void config_check();  // Check whether parameters are configured correctly
        int check_cache_hit(uint set_base, uint addr, int level);
        /**获取cache当前set中空余的line*/
        int get_cache_free_line(uint set_base, int level);
        /**找到合适的line之后，将数据写入cache line中*/
        void set_cache_line(uint index, uint addr, int level);
        ///**对一个指令进行分析*/
        //void do_cache_op(uint addr, Cache_operation oper_style);
        ///**读入trace文件*/
        //void load_trace(const char* filename);

        /**lock a cache line*/
        int lock_cache_line(uint addr, int level);
        /**unlock a cache line*/
        int unlock_cache_line(uint addr, int level);
        /**@return 返回miss率*/

        //void re_init();

        CacheReq transMemReq2CacheReq(const MemReq& memReq);
        MemReq transCacheReq2MemReq(const CacheReq& cacheReq);

        uint getCacheTag(const uint addr, const uint level);
        uint getCacheSetIndex(const uint addr, const uint level);
    public:
        uint getCacheBank(const uint addr, const uint level);
        uint getCacheBlockId(const uint addr, const uint level);  // For memSystem coalescing
    private:
        uint getCacheBlockOffset(const uint addr, const uint level);
        bool sendReq2CacheBank(const CacheReq cacheReq, const uint level);
        bool addrCoaleseCheck(const uint addr, const uint level);
        bool sendReq2reqQueue2Mem(const CacheReq cacheReq);
        bool setCacheBlock(uint addr, uint level);  // Set a cacheblock which the addr belongs to a specific cache level 
        bool writeBackDirtyCacheline(const uint tag, const uint setIndex, const uint level);

        //void updateReqQueueOfCacheLevel();
        //void updateCacheLine();
        void updateReqQueueLatency();
        void updateReqQueue();  // Send req to cacheline to check hit/miss
        void updateAckQueue();  // Send back ack and replace the cacheline of lower level
        void resetBankConflictRecorder();
        void sendMshrOutstandingReq();

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<vector<ReqQueueBank>>& getReqQueue() const;
        const deque<MemReq>& getReqQueue2Mem() const;
#endif // DEBUG_MODE

    private:
        const unsigned char CACHE_FLAG_VALID = 0x01;
        const unsigned char CACHE_FLAG_DIRTY = 0x02;
        const unsigned char CACHE_FLAG_LOCK = 0x04;
        const unsigned char CACHE_FLAG_MASK = 0xff;
        /**最多多少层cache*/
        //static const int CACHE_MAXLEVEL = 2;
        //const char OPERATION_READ = 'l';
        //const char OPERATION_WRITE = 's';
        //const char OPERATION_LOCK = 'k';
        //const char OPERATION_UNLOCK = 'u';

        vector<uint> a_cache_size = { CACHE_SIZE_L1, CACHE_SIZE_L2 };  // 多级cache的大小设置 (byte)
        vector<uint> a_cache_line_size = { CACHE_LINE_SIZE_L1, CACHE_LINE_SIZE_L2 };  // 多级cache的line size（block size）大小 (byte)
        vector<uint> a_mapping_ways = { CACHE_MAPPING_WAY_L1, CACHE_MAPPING_WAY_L2 };  // 组相连的链接方式 (几路组相连)

        vector<uint> cache_access_latency = { CACHE_ACCESS_LATENCY_L1, CACHE_ACCESS_LATENCY_L2 };  // L1 cycle = 1; L2 cycle = 4;
        vector<uint> reqQueueSizePerBank = { CACHE_REQ_Q_SIZE_PER_BANK_L1, CACHE_REQ_Q_SIZE_PER_BANK_L2 };  
        vector<uint> ackQueueSizePerBank = { CACHE_ACK_Q_SIZE_PER_BANK_L1, CACHE_ACK_Q_SIZE_PER_BANK_L2 };
        vector<uint> bankNum = { CACHE_BANK_NUM_L1, CACHE_BANK_NUM_L2 };  // L1 = 8, L2 = 16
        vector<pair<uint, uint>> mshrPara = { make_pair(CACHE_MSHR_ENTRY_NUM_L1, CACHE_MSHR_SIZE_PER_ENTRY_L1),
                                             make_pair(CACHE_MSHR_ENTRY_NUM_L2, CACHE_MSHR_SIZE_PER_ENTRY_L2) };
        vector<Cache_swap_style> cache_swap_style = { Cache_swap_style::CACHE_SWAP_LRU , Cache_swap_style::CACHE_SWAP_LRU };
        vector<Cache_write_strategy> cache_write_strategy = { Cache_write_strategy::WRITE_BACK, Cache_write_strategy::WRITE_BACK };
        vector<Cache_write_allocate> cache_write_allocate = { Cache_write_allocate::WRITE_ALLOCATE, Cache_write_allocate::WRITE_ALLOCATE };

        /**cache的总大小，单位byte*/
        vector<uint> cache_size = vector<uint>(CACHE_MAXLEVEL);
        /**cache line(Cache block)cache块的大小*/
        vector<uint> cache_line_size = vector<uint>(CACHE_MAXLEVEL);
        /**总的行数*/
        vector<uint> cache_line_num = vector<uint>(CACHE_MAXLEVEL);
        /**Bank number*/
        vector<uint> cache_bank_num = vector<uint>(CACHE_MAXLEVEL);
        /**2的多少次方是bank的数量，用于匹配地址时，进行位移比较*/
        vector<uint> cache_bank_shifts = vector<uint>(CACHE_MAXLEVEL);
        /**每个set有多少way*/
        vector<uint> cache_mapping_ways = vector<uint>(CACHE_MAXLEVEL);
        /**整个cache有多少组*/
        vector<uint> cache_set_size = vector<uint>(CACHE_MAXLEVEL);
        /**2的多少次方是set的数量，用于匹配地址时，进行位移比较*/
        vector<uint> cache_set_shifts = vector<uint>(CACHE_MAXLEVEL);
        /**2的多少次方是line的长度，用于匹配地址*/
        vector<uint> cache_line_shifts = vector<uint>(CACHE_MAXLEVEL);
        /**真正的cache地址列。指针数组*/
        vector<Cache_Line*> caches = vector<Cache_Line*>(CACHE_MAXLEVEL);

        /**指令计数器*/
        uint tick_count;
        /**cache缓冲区,由于并没有数据*/
        //_u8 *cache_buf[CACHE_MAXLEVEL];
        /**缓存替换算法*/
        vector<Cache_swap_style> swap_style = vector<Cache_swap_style>(CACHE_MAXLEVEL);
        // Write strategy
        vector<Cache_write_strategy> write_strategy = vector<Cache_write_strategy>(CACHE_MAXLEVEL);
        // Write allocate
        vector<Cache_write_allocate> write_allocate = vector<Cache_write_allocate>(CACHE_MAXLEVEL);
        /**读写内存的计数*/
        uint cache_r_count, cache_w_count;
        /**实际写内存的计数，cache --> memory */
        uint cache_w_memory_count;
        /**cache hit和miss的计数*/
        vector<uint> cache_hit_count = vector<uint>(CACHE_MAXLEVEL);
        vector<uint> cache_miss_count = vector<uint>(CACHE_MAXLEVEL);
        /**空闲cache line的index记录，在寻找时，返回空闲line的index*/
        vector<uint> cache_free_num = vector<uint>(CACHE_MAXLEVEL);

        //vector<vector<pair<CacheReq, uint>>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>)
        vector<Mshr> mshr;
        vector<vector<ReqQueueBank>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>), fifo mode
        vector<vector<deque<CacheReq>>> ackQueue;  // Fifo mode
        vector<deque<bool>> reqBankConflict;  // Record bank visiting, emulate bank confliction
        vector<deque<bool>> ackBankConflict;  // Record bank visiting, emulate bank confliction
        deque<MemReq> reqQueue2Mem;  // reqQueue to DRAM (Beyond llc reqQueue)
        //vector<vector<uint>> sendPtr;  // sendPtr of each level cache's each bank ( sendPtr[level][bank] )
        vector<uint> reqQueueBankPtr;  // Round-robin to send reqQueue req to next cachelevel
        vector<uint> ackQueueBankPtr;  // Round-robin to traverse ackQueue

        //uint reqCnt = 0;  // Record the sequence of each cacheReq
    };
}
