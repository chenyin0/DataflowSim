#pragma once
//
// Created by find on 16-7-19.
// cache line = cache block = ԭ�������cache item ~= cache way
//

/*
Cache Module

1. Multi-hierarchy (L1, L2, ... Ln)
2. Operation: write/read, lock/unlock
3. MSHR


Req flow:
1. Send req:
	reqQueue_MemSystem -> reqQueue_L1 (hit/miss check, MSHR coalescing) -> reqQueue_L2 (hit/miss check, MSHR coalescing) -> ... -> DRAM

2. Receive req:
	DRAM -> reqQueue Ln (update cacheline) -> ... -> reqQueue L1 (update cacheline) -> reqQueue_MemSystem


Address bit fields:
  32λ��ַѰַ�ռ�2^32words
	MSB                                    LSB
	Tag + Set(Index) + Block_offset(Cacheline)   Lower bits of Set is bank_index    

*/

#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "../DataType.h"
#include "../EnumType.h"
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"

typedef unsigned char _u8;
//typedef unsigned long long uint;

namespace DFSim
{
	using ReqQueueBank = vector<pair<CacheReq, uint>>;  // Emulate bank conflict

	//д�ڴ淽����Ĭ��д�ذɡ�
	class Cache_Line 
	{
	public:
		uint tag;
		/**������FIFO���¼��һ��ʼ�ķ���ʱ�䣬LRU���¼��һ�η��ʵ�ʱ��*/
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
		// ����
	public:
		Cache();
		~Cache();

		// Interface function for MemSystem
		bool addTransaction(MemReq _req);  // Add transaction to Cache
		vector<MemReq> callBack();  // Get memory access results from Cache

		// Interface func with DRAM
		void sendReq2Mem(DRAMSim::MultiChannelMemorySystem* mem);  // Send memory req to DRAM
		void mem_req_complete(MemReq _req);

		void cacheUpdate();
		void reqQueueUpdate();

	private:
		void init(/*uint a_cache_size[], uint a_cache_line_size[], uint a_mapping_ways[]*/);
		/**ԭ������addr�Ĵ�����Щ���⣬������û�гɹ��������Ĵ��롣
		 * ����Ƿ�����
		 * @args:
		 * cache: ģ���cache
		 * set_base: ��ǰ��ַ������һ��set�����ַ��ʲô��
		 * addr: Ҫ�жϵ��ڴ��ַ
		 * @return:
		 * ����cache�ĵ�ַ�϶����ᳬ��int����Ϊcache��С�����ģ�
		 * TODO: check the addr */
		void config_check();  // Check whether parameters are configured correctly
		int check_cache_hit(uint set_base, uint addr, int level);
		/**��ȡcache��ǰset�п����line*/
		int get_cache_free_line(uint set_base, int level);
		/**�ҵ����ʵ�line֮�󣬽�����д��cache line��*/
		void set_cache_line(uint index, uint addr, int level);
		///**��һ��ָ����з���*/
		//void do_cache_op(uint addr, Cache_operation oper_style);
		///**����trace�ļ�*/
		//void load_trace(const char* filename);

		/**lock a cache line*/
		int lock_cache_line(uint addr, int level);
		/**unlock a cache line*/
		int unlock_cache_line(uint addr, int level);
		/**@return ����miss��*/
		double get_miss_rate(int level) 
		{
			return 100.0 * cache_miss_count[level] / (cache_miss_count[level] + cache_hit_count[level]);
		}

		void re_init();

		CacheReq transMemReq2CacheReq(const MemReq& memReq);
		MemReq transCacheReq2MemReq(const CacheReq& cacheReq);

		uint getCacheTag(const uint addr, const uint level);
		uint getCacheSetIndex(const uint addr, const uint level);
		uint getCacheBank(const uint addr, const uint level);
		uint getCacheBlockOffset(const uint addr, const uint level);

		bool sendReq2CacheBank(const CacheReq cacheReq, const uint level);
		bool addrCoaleseCheck(const uint addr, const uint level);
		bool sendReq2reqQueue2Mem(const CacheReq cacheReq);
		bool setCacheBlock(uint addr, uint level);  // Set a cacheblock which the addr belongs to a specific cache level 
		bool writeBackDirtyCacheline(const uint tag, const uint setIndex, const uint level);

		void updateReqQueueOfCacheLevel();
		void updateCacheLine();

#ifdef DEBUG_MODE  // Get private instance for debug
	public:
		const vector<vector<ReqQueueBank>>& getReqQueue() const;
#endif // DEBUG_MODE

	private:
		const unsigned char CACHE_FLAG_VALID = 0x01;
		const unsigned char CACHE_FLAG_DIRTY = 0x02;
		const unsigned char CACHE_FLAG_LOCK = 0x04;
		const unsigned char CACHE_FLAG_MASK = 0xff;
		/**�����ٲ�cache*/
		//static const int CACHE_MAXLEVEL = 2;
		//const char OPERATION_READ = 'l';
		//const char OPERATION_WRITE = 's';
		//const char OPERATION_LOCK = 'k';
		//const char OPERATION_UNLOCK = 'u';

		vector<uint> a_cache_size = {1*1024, 8*1024};  // �༶cache�Ĵ�С���� (byte)
		vector<uint> a_cache_line_size = {32, 32};  // �༶cache��line size��block size����С (byte)
		vector<uint> a_mapping_ways = {4, 4};  // �����������ӷ�ʽ (��·������)

		vector<uint> cache_access_latency = { 1, 4 };  // L1 cycle = 1; L2 cycle = 4;
		vector<uint> reqQueueSizePerBank = { 8, 4 };  // L1 each bank's reqQueueSize = 2; L2 = 2;
		vector<uint> bankNum = { 4, 4 };  // L1 = 8, L2 = 16
		vector<Cache_swap_style> cache_swap_style = { Cache_swap_style::CACHE_SWAP_LRU , Cache_swap_style::CACHE_SWAP_LRU };
		vector<Cache_write_strategy> cache_write_strategy = { Cache_write_strategy::WRITE_BACK, Cache_write_strategy::WRITE_BACK };
		vector<Cache_write_allocate> cache_write_allocate = { Cache_write_allocate::WRITE_ALLOCATE, Cache_write_allocate::WRITE_ALLOCATE };

		/**cache���ܴ�С����λbyte*/
		uint cache_size[CACHE_MAXLEVEL];
		/**cache line(Cache block)cache��Ĵ�С*/
		uint cache_line_size[CACHE_MAXLEVEL];
		/**�ܵ�����*/
		uint cache_line_num[CACHE_MAXLEVEL];
		/**Bank number*/
		uint cache_bank_num[CACHE_MAXLEVEL];
		/**2�Ķ��ٴη���bank������������ƥ���ַʱ������λ�ƱȽ�*/
		uint cache_bank_shifts[CACHE_MAXLEVEL];
		/**ÿ��set�ж���way*/
		uint cache_mapping_ways[CACHE_MAXLEVEL];
		/**����cache�ж�����*/
		uint cache_set_size[CACHE_MAXLEVEL];
		/**2�Ķ��ٴη���set������������ƥ���ַʱ������λ�ƱȽ�*/
		uint cache_set_shifts[CACHE_MAXLEVEL];
		/**2�Ķ��ٴη���line�ĳ��ȣ�����ƥ���ַ*/
		uint cache_line_shifts[CACHE_MAXLEVEL];
		/**������cache��ַ�С�ָ������*/
		Cache_Line* caches[CACHE_MAXLEVEL];

		/**ָ�������*/
		uint tick_count;
		/**cache������,���ڲ�û������*/
	//    _u8 *cache_buf[CACHE_MAXLEVEL];
		/**�����滻�㷨*/
		Cache_swap_style swap_style[CACHE_MAXLEVEL];
		// Write strategy
		Cache_write_strategy write_strategy[CACHE_MAXLEVEL];
		// Write allocate
		Cache_write_allocate write_allocate[CACHE_MAXLEVEL];
		/**��д�ڴ�ļ���*/
		uint cache_r_count, cache_w_count;
		/**ʵ��д�ڴ�ļ�����cache --> memory */
		uint cache_w_memory_count;
		/**cache hit��miss�ļ���*/
		uint cache_hit_count[CACHE_MAXLEVEL];
		uint cache_miss_count[CACHE_MAXLEVEL];
		/**����cache line��index��¼����Ѱ��ʱ�����ؿ���line��index*/
		uint cache_free_num[CACHE_MAXLEVEL];

		//vector<vector<pair<CacheReq, uint>>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>)
		vector<vector<ReqQueueBank>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>)
		deque<MemReq> reqQueue2Mem;  // reqQueue to DRAM (Beyond llc reqQueue)
		vector<vector<uint>> sendPtr;  // sendPtr of each level cache's each bank ( sendPtr[level][bank] )
	};
}
