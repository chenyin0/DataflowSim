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
	reqQueue_MemSystem -> reqQueue_L1 (hit/miss check, MSHR merge) -> reqQueue_L2 (hit/miss check, MSHR merge) -> ... -> DRAM

2. Receive req:
	DRAM -> reqQueue Ln (update cacheline, hit/miss check again) -> ... -> reqQueue L1 -> reqQueue_MemSystem

*/

#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "../DataType.h"
#include "../EnumType.h"

typedef unsigned char _u8;
//typedef unsigned long long uint;

namespace DFSim
{
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
		void sendReq(DRAMSim::MultiChannelMemorySystem* mem);  // Send memory req to DRAM
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
		/**��һ��ָ����з���*/
		void do_cache_op(uint addr, Cache_operation oper_style);
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

		CacheReq transMemReq2CacheReq(const MemReq& _req);

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
		vector<uint> a_cache_line_size = {64, 64};  // �༶cache��line size��block size����С (byte)
		vector<uint> a_mapping_ways = {4, 4};  // �����������ӷ�ʽ (��·������)

		vector<uint> cache_access_latency = {1, 4};  // L1 cycle = 1; L2 cycle = 4;
		vector<uint> reqQueueSize = { 8, 16 };  // L1 reqQueueSize = 8; L2 = 16;
		vector<uint> bankNum = { 8, 16 };  // L1 = 8, L2 = 16

		/**cache���ܴ�С����λbyte*/
		uint cache_size[CACHE_MAXLEVEL];
		/**cache line(Cache block)cache��Ĵ�С*/
		uint cache_line_size[CACHE_MAXLEVEL];
		/**�ܵ�����*/
		uint cache_line_num[CACHE_MAXLEVEL];
		/**Bank number*/
		uint cache_bank_num[CACHE_MAXLEVEL];
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
		/**��д�ڴ�ļ���*/
		uint cache_r_count, cache_w_count;
		/**ʵ��д�ڴ�ļ�����cache --> memory */
		uint cache_w_memory_count;
		/**cache hit��miss�ļ���*/
		uint cache_hit_count[CACHE_MAXLEVEL];
		uint cache_miss_count[CACHE_MAXLEVEL];
		/**����cache line��index��¼����Ѱ��ʱ�����ؿ���line��index*/
		uint cache_free_num[CACHE_MAXLEVEL];

		vector<deque<pair<CacheReq, uint>>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>)
	};
}
