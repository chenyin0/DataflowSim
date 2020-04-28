#pragma once
//
// Created by find on 16-7-19.
// cache line = cache block = 原代码里的cache item ~= cache way
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
	//写内存方法就默认写回吧。
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
		// 隐患
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
		/**原代码中addr的处理有些问题，导致我没有成功运行他的代码。
		 * 检查是否命中
		 * @args:
		 * cache: 模拟的cache
		 * set_base: 当前地址属于哪一个set，其基址是什么。
		 * addr: 要判断的内存地址
		 * @return:
		 * 由于cache的地址肯定不会超过int（因为cache大小决定的）
		 * TODO: check the addr */
		void config_check();  // Check whether parameters are configured correctly
		int check_cache_hit(uint set_base, uint addr, int level);
		/**获取cache当前set中空余的line*/
		int get_cache_free_line(uint set_base, int level);
		/**找到合适的line之后，将数据写入cache line中*/
		void set_cache_line(uint index, uint addr, int level);
		/**对一个指令进行分析*/
		void do_cache_op(uint addr, Cache_operation oper_style);
		///**读入trace文件*/
		//void load_trace(const char* filename);

		/**lock a cache line*/
		int lock_cache_line(uint addr, int level);
		/**unlock a cache line*/
		int unlock_cache_line(uint addr, int level);
		/**@return 返回miss率*/
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
		/**最多多少层cache*/
		//static const int CACHE_MAXLEVEL = 2;
		//const char OPERATION_READ = 'l';
		//const char OPERATION_WRITE = 's';
		//const char OPERATION_LOCK = 'k';
		//const char OPERATION_UNLOCK = 'u';

		vector<uint> a_cache_size = {1*1024, 8*1024};  // 多级cache的大小设置 (byte)
		vector<uint> a_cache_line_size = {64, 64};  // 多级cache的line size（block size）大小 (byte)
		vector<uint> a_mapping_ways = {4, 4};  // 组相连的链接方式 (几路组相连)

		vector<uint> cache_access_latency = {1, 4};  // L1 cycle = 1; L2 cycle = 4;
		vector<uint> reqQueueSize = { 8, 16 };  // L1 reqQueueSize = 8; L2 = 16;
		vector<uint> bankNum = { 8, 16 };  // L1 = 8, L2 = 16

		/**cache的总大小，单位byte*/
		uint cache_size[CACHE_MAXLEVEL];
		/**cache line(Cache block)cache块的大小*/
		uint cache_line_size[CACHE_MAXLEVEL];
		/**总的行数*/
		uint cache_line_num[CACHE_MAXLEVEL];
		/**Bank number*/
		uint cache_bank_num[CACHE_MAXLEVEL];
		/**每个set有多少way*/
		uint cache_mapping_ways[CACHE_MAXLEVEL];
		/**整个cache有多少组*/
		uint cache_set_size[CACHE_MAXLEVEL];
		/**2的多少次方是set的数量，用于匹配地址时，进行位移比较*/
		uint cache_set_shifts[CACHE_MAXLEVEL];
		/**2的多少次方是line的长度，用于匹配地址*/
		uint cache_line_shifts[CACHE_MAXLEVEL];
		/**真正的cache地址列。指针数组*/
		Cache_Line* caches[CACHE_MAXLEVEL];

		/**指令计数器*/
		uint tick_count;
		/**cache缓冲区,由于并没有数据*/
	//    _u8 *cache_buf[CACHE_MAXLEVEL];
		/**缓存替换算法*/
		Cache_swap_style swap_style[CACHE_MAXLEVEL];
		/**读写内存的计数*/
		uint cache_r_count, cache_w_count;
		/**实际写内存的计数，cache --> memory */
		uint cache_w_memory_count;
		/**cache hit和miss的计数*/
		uint cache_hit_count[CACHE_MAXLEVEL];
		uint cache_miss_count[CACHE_MAXLEVEL];
		/**空闲cache line的index记录，在寻找时，返回空闲line的index*/
		uint cache_free_num[CACHE_MAXLEVEL];

		vector<deque<pair<CacheReq, uint>>> reqQueue;  // Emulate L1~Ln cache access latency (pair<req, latency>)
	};
}
