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
        /** fifo_count records the first access time£¬lru_count records the last access time */
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

        void updateCacheAccessCnt() { ++cacheAccessCnt; }

    private:
        void init();
        void config_check();  // Check whether parameters are configured correctly
        int check_cache_hit(uint set_base, uint addr, int level);
        /** Get the free cacheline of current set */
        int get_cache_free_line(uint set_base, int level);
        /** Write to this cacheline */
        void set_cache_line(uint index, uint addr, int level);
        ///** Analyze a operation */
        //void do_cache_op(uint addr, Cache_operation oper_style);
        ///** Read trace files */
        //void load_trace(const char* filename);

        /**lock a cache line*/
        int lock_cache_line(uint addr, int level);
        /**unlock a cache line*/
        int unlock_cache_line(uint addr, int level);

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
        //bool addrCoaleseCheck(const uint addr, const uint level);
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
        const vector<vector<ReqQueueBank>>& getReqQueue() const { return DFSim::Cache::reqQueue; }
        const vector<vector<deque<CacheReq>>>& getAckQueue() const { return DFSim::Cache::ackQueue; }
        const deque<MemReq>& getReqQueue2Mem() const { return reqQueue2Mem; }
        const vector<Mshr>& getMshr() const { return mshr; }
        const uint& getCacheAccessCnt() const { return cacheAccessCnt; }
        const uint& getMemAccessCnt() const { return memAccessCnt; }
#endif // DEBUG_MODE

    private:
        const unsigned char CACHE_FLAG_VALID = 0x01;
        const unsigned char CACHE_FLAG_DIRTY = 0x02;
        const unsigned char CACHE_FLAG_LOCK = 0x04;
        const unsigned char CACHE_FLAG_MASK = 0xff;

        //const char OPERATION_READ = 'l';
        //const char OPERATION_WRITE = 's';
        //const char OPERATION_LOCK = 'k';
        //const char OPERATION_UNLOCK = 'u';

        // 2 level cache
        //vector<uint> a_cache_size = { CACHE_SIZE_L1, CACHE_SIZE_L2 };  // Cache size of each level (byte)
        //vector<uint> a_cache_line_size = { CACHE_LINE_SIZE_L1, CACHE_LINE_SIZE_L2 };  // Cacheline size of each level (byte)
        //vector<uint> a_mapping_ways = { CACHE_MAPPING_WAY_L1, CACHE_MAPPING_WAY_L2 };  // Way number in each set

        //vector<uint> cache_access_latency = { CACHE_ACCESS_LATENCY_L1, CACHE_ACCESS_LATENCY_L2 };  // L1 cycle = 1; L2 cycle = 4;
        //vector<uint> reqQueueSizePerBank = { CACHE_REQ_Q_SIZE_PER_BANK_L1, CACHE_REQ_Q_SIZE_PER_BANK_L2 };  
        //vector<uint> ackQueueSizePerBank = { CACHE_ACK_Q_SIZE_PER_BANK_L1, CACHE_ACK_Q_SIZE_PER_BANK_L2 };
        //vector<uint> bankNum = { CACHE_BANK_NUM_L1, CACHE_BANK_NUM_L2 };  // L1 = 8, L2 = 16
        //vector<pair<uint, uint>> mshrPara = { make_pair(CACHE_MSHR_ENTRY_NUM_L1, CACHE_MSHR_SIZE_PER_ENTRY_L1),
        //                                     make_pair(CACHE_MSHR_ENTRY_NUM_L2, CACHE_MSHR_SIZE_PER_ENTRY_L2) };

        //vector<Cache_swap_style> cache_swap_style = { Cache_swap_style::CACHE_SWAP_LRU , Cache_swap_style::CACHE_SWAP_LRU };
        //vector<Cache_write_strategy> cache_write_strategy = { Cache_write_strategy::WRITE_BACK, Cache_write_strategy::WRITE_BACK };
        //vector<Cache_write_allocate> cache_write_allocate = { Cache_write_allocate::WRITE_ALLOCATE, Cache_write_allocate::WRITE_ALLOCATE };

        // 1 level cache
        vector<uint> a_cache_size = { CACHE_SIZE_L1};  // Cache size of each level (byte)
        vector<uint> a_cache_line_size = { CACHE_LINE_SIZE_L1};  // Cacheline size of each level (byte)
        vector<uint> a_mapping_ways = { CACHE_MAPPING_WAY_L1};  // Way number in each set

        vector<uint> cache_access_latency = { CACHE_ACCESS_LATENCY_L1};  // L1 cycle = 1; L2 cycle = 4;
        vector<uint> reqQueueSizePerBank = { CACHE_REQ_Q_SIZE_PER_BANK_L1};
        vector<uint> ackQueueSizePerBank = { CACHE_ACK_Q_SIZE_PER_BANK_L1};
        vector<uint> bankNum = { CACHE_BANK_NUM_L1};  // L1 = 8, L2 = 16
        vector<pair<uint, uint>> mshrPara = { make_pair(CACHE_MSHR_ENTRY_NUM_L1, CACHE_MSHR_SIZE_PER_ENTRY_L1)};

        vector<Cache_swap_style> cache_swap_style = { Cache_swap_style::CACHE_SWAP_LRU };
        vector<Cache_write_strategy> cache_write_strategy = { Cache_write_strategy::WRITE_BACK };
        vector<Cache_write_allocate> cache_write_allocate = { Cache_write_allocate::WRITE_ALLOCATE };

        /** Cache size of each level£¬byte */
        vector<uint> cache_size = vector<uint>(CACHE_MAXLEVEL);
        /** Cacheline size */
        vector<uint> cache_line_size = vector<uint>(CACHE_MAXLEVEL);
        /** Cacheline number */
        vector<uint> cache_line_num = vector<uint>(CACHE_MAXLEVEL);
        /** Bank number */
        vector<uint> cache_bank_num = vector<uint>(CACHE_MAXLEVEL);
        /** Bank shift*/
        //vector<uint> cache_bank_shifts = vector<uint>(CACHE_MAXLEVEL);
        /** Way number of each set*/
        vector<uint> cache_mapping_ways = vector<uint>(CACHE_MAXLEVEL);
        /** Cache set number */
        vector<uint> cache_set_size = vector<uint>(CACHE_MAXLEVEL);
        /** Cache set shift */
        vector<uint> cache_set_shifts = vector<uint>(CACHE_MAXLEVEL);
        /** Cacheline shift */
        vector<uint> cache_line_shifts = vector<uint>(CACHE_MAXLEVEL);
        /** Cache array, Pointer array */
        vector<Cache_Line*> caches = vector<Cache_Line*>(CACHE_MAXLEVEL);

        /** Instruction counter */
        uint tick_count;
        /** Cache buffer */
        //_u8 *cache_buf[CACHE_MAXLEVEL];
        /** Replacement algorithm */
        vector<Cache_swap_style> swap_style = vector<Cache_swap_style>(CACHE_MAXLEVEL);
        // Write strategy
        vector<Cache_write_strategy> write_strategy = vector<Cache_write_strategy>(CACHE_MAXLEVEL);
        // Write allocate
        vector<Cache_write_allocate> write_allocate = vector<Cache_write_allocate>(CACHE_MAXLEVEL);
        /** Read/write counter */
        uint cache_r_count, cache_w_count;
        /** Access memory counter£¬cache --> memory */
        uint cache_w_memory_count;
        /** Hit/miss counter */
        vector<uint> cache_hit_count = vector<uint>(CACHE_MAXLEVEL);
        vector<uint> cache_miss_count = vector<uint>(CACHE_MAXLEVEL);
        ///** Record free cacheline */
        //vector<uint> cache_free_num = vector<uint>(CACHE_MAXLEVEL);
        /** Profiling counter **/
        uint cacheAccessCnt = 0;
        uint memAccessCnt = 0;

        vector<Mshr> mshr;
        vector<vector<ReqQueueBank>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>), fifo mode
        vector<vector<deque<CacheReq>>> ackQueue;  // Fifo mode
        vector<deque<bool>> reqBankConflict;  // Record bank visiting, emulate bank confliction
        vector<deque<bool>> ackBankConflict;  // Record bank visiting, emulate bank confliction
        deque<MemReq> reqQueue2Mem;  // reqQueue to DRAM (Beyond llc reqQueue)
        //vector<vector<uint>> sendPtr;  // sendPtr of each level cache's each bank ( sendPtr[level][bank] )
        vector<uint> reqQueueBankPtr;  // Round-robin to send reqQueue req to next cachelevel
        vector<uint> ackQueueBankPtr;  // Round-robin to traverse ackQueue
    };
}
