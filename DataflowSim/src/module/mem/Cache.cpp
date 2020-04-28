//
// Created by find on 16-7-19.
// Cache architect
// memory address  format:
// |tag|��� log2(����)|���ڿ��log2(mapping_ways)|���ڵ�ַ log2(cache line)|
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

/**@arg a_cache_size[] �༶cache�Ĵ�С����
 * @arg a_cache_line_size[] �༶cache��line size��block size����С
 * @arg a_mapping_ways[] �����������ӷ�ʽ*/
void Cache::init(/*uint a_cache_size[3], uint a_cache_line_size[3], uint a_mapping_ways[3]*/) 
{
	// Check whether configuration is correct
	config_check();

	reqQueue.resize(CACHE_MAXLEVEL);

	for (_u8 i = 0; i < CACHE_MAXLEVEL; ++i)
	{
		cache_size[i] = a_cache_size[i];
		cache_line_size[i] = a_cache_line_size[i];
		cache_line_num[i] = (uint)a_cache_size[i] / a_cache_line_size[i];  // �ܵ�line�� = cache�ܴ�С/ ÿ��line�Ĵ�С��һ��64byte��ģ���ʱ������ã�
		cache_line_shifts[i] = (uint)log2(a_cache_line_size[i]);
		cache_mapping_ways[i] = a_mapping_ways[i];  // ��·������
		cache_set_size[i] = cache_line_num[i] / cache_mapping_ways[i];  // �ܹ��ж���set
		cache_set_shifts[i] = (uint)log2(cache_set_size[i]);  // �������ռ��λ����ͬ����shifts
		cache_free_num[i] = cache_line_num[i];  // ���п飨line��
		cache_bank_num[i] = bankNum[i];

		reqQueue[i].resize(reqQueueSize[i]);

		// Assign space for each cache line
		caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
		memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
	}

	cache_r_count = 0;
	cache_w_count = 0;
	cache_w_memory_count = 0;
	// ָ��������Ҫ�������滻���Ե�ʱ���ṩ�Ƚϵ�key�������л���miss��ʱ����Ӧline������Լ���countΪ��ʱ��tick_count;
	tick_count = 0;
	//    cache_buf = (_u8 *) malloc(cache_size);
	//    memset(cache_buf, 0, this->cache_size);
		// Ϊÿһ�з���ռ�
	//for (int i = 0; i < 2; ++i) 
	//{
	//	caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
	//	memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
	//}
	//����ʱ��Ĭ������
	swap_style[0] = Cache_swap_style::CACHE_SWAP_LRU;
	swap_style[1] = Cache_swap_style::CACHE_SWAP_LRU;
	re_init();
	srand((unsigned)time(NULL));
}

void Cache::config_check()
{
	if(a_cache_size.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"Cache_size\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

	if (a_cache_line_size.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"Cache_line_size\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

	if (a_mapping_ways.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"Mapping_ways\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

	if (cache_access_latency.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"Cache_access_latency\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

	if (reqQueueSize.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"ReqQueueSize\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

	if (bankNum.size() != CACHE_MAXLEVEL)
		Debug::throwError("Parameter \"BankNum\" demension not consistent with CACHE_MAXLEVEL", __FILE__, __LINE__);

}

/**�����ĳ�ʼ��������һ��ʼ�������;��Ҫ��tick_count���������caches����գ�ִ�дˡ���Ҫ��Ϊtick_count���������ܻᳬ��unsigned long long������һ��tick_count���㣬caches���count����Ҳ�ͳ����˴���*/
void Cache::re_init() 
{
	tick_count = 0;
	memset(cache_hit_count, 0, sizeof(cache_hit_count));
	memset(cache_miss_count, 0, sizeof(cache_miss_count));

	for (_u8 i = 0; i < CACHE_MAXLEVEL; ++i)
	{
		cache_free_num[i] = cache_line_num[i];
		memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
	}

	/*cache_free_num[0] = cache_line_num[0];
	cache_free_num[1] = cache_line_num[1];
	memset(caches[0], 0, sizeof(Cache_Line) * cache_line_num[0]);
	memset(caches[1], 0, sizeof(Cache_Line) * cache_line_num[1]);*/
	//    memset(cache_buf, 0, this->cache_size);
}

Cache::~Cache() 
{
	/*free(caches[0]);
	free(caches[1]);*/

	for (uint i = 0; i < CACHE_MAXLEVEL; ++i)
	{
		free(caches[i]);
	}

	//    free(cache_buf);
}

int Cache::check_cache_hit(uint set_base, uint addr, int level) 
{
	/**ѭ�����ҵ�ǰset������way��line����ͨ��tagƥ�䣬�鿴��ǰ��ַ�Ƿ���cache��*/
	uint i;
	for (i = 0; i < cache_mapping_ways[level]; ++i) 
	{
		if ((caches[level][set_base + i].flag & CACHE_FLAG_VALID) &&
			(caches[level][set_base + i].tag == (addr >> (cache_set_shifts[level] + cache_line_shifts[level])))) 
		{
			return set_base + i;
		}
	}
	return -1;
}

/**��ȡ��ǰset�п��õ�line�����û�У����ҵ�Ҫ���滻�Ŀ�*/
int Cache::get_cache_free_line(uint set_base, int level) 
{
	uint min_count;
	int free_index = -1;  // Initial free index
	/**�ӵ�ǰcache set���ҿ��õĿ���line�����ã������ݣ���������
	 * cache_free_num��ͳ�Ƶ�����cache�Ŀ��ÿ�*/
	for (uint i = 0; i < cache_mapping_ways[level]; ++i) 
	{
		if (!(caches[level][set_base + i].flag & CACHE_FLAG_VALID)) 
		{
			if (cache_free_num[level] > 0)
				cache_free_num[level]--;
			return set_base + i;
		}
	}
	/**û�п���line����ִ���滻�㷨
	 * lock״̬�Ŀ���δ�����*/
	//free_index = -1;
	if (swap_style[level] == Cache_swap_style::CACHE_SWAP_RAND)
	{
		// TODO: ����滻Lock״̬��line�����ٸ�
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
		//���ȫ���������ˣ�Ӧ�û��ߵ�����������ôǿ�ƽ����滻��ǿ���滻��ʱ����Ҫsetline?
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
		//���ԭ�е�cache line�������ݣ������λ
		if (caches[level][free_index].flag & CACHE_FLAG_DIRTY) 
		{
			// TODO: д�ص�L2 cache�С�
			caches[level][free_index].flag &= ~CACHE_FLAG_DIRTY;
			cache_w_memory_count++;
		}
	}
	//else 
	//{
	//	printf("I should not show\n");
	//}
	return free_index;
}

/**������д��cache line*/
void Cache::set_cache_line(uint index, uint addr, int level) 
{
	Cache_Line* line = caches[level] + index;
	// ����ÿ��line��buf������cache���buf���ظ��Ķ��Ҳ�û��������ݡ�
//    line->buf = cache_buf + cache_line_size * index;
	// �������line��tagλ
	line->tag = addr >> (cache_set_shifts[level] + cache_line_shifts[level]);
	line->flag = (_u8)~CACHE_FLAG_MASK;
	line->flag |= CACHE_FLAG_VALID;
	line->count = tick_count;
}

/**����Ҫ��level*/
void Cache::do_cache_op(uint addr, Cache_operation oper_style) 
{
	uint set_l1, set_l2, set_base_l1, set_base_l2;
	long long hit_index_l1, hit_index_l2, free_index_l1, free_index_l2;
	tick_count++;
	if (oper_style == Cache_operation::READ)
		cache_r_count++;
	if (oper_style == Cache_operation::WRITE)
		cache_w_count++;
	set_l2 = (addr >> cache_line_shifts[1]) % cache_set_size[1];
	set_base_l2 = set_l2 * cache_mapping_ways[1];
	hit_index_l2 = check_cache_hit(set_base_l2, addr, 1);
	set_l1 = (addr >> cache_line_shifts[0]) % cache_set_size[0];
	set_base_l1 = set_l1 * cache_mapping_ways[0];
	hit_index_l1 = check_cache_hit(set_base_l1, addr, 0);

	// lock��������ֻ����L2�ġ�(L2 is shared)
	if (oper_style == Cache_operation::LOCK || oper_style == Cache_operation::UNLOCK)
	{
		if (hit_index_l2 >= 0) 
		{
			if (oper_style == Cache_operation::LOCK)
			{
				cache_hit_count[1]++;
				if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[1])
				{
					caches[1][hit_index_l2].lru_count = tick_count;
				}
				lock_cache_line((uint)hit_index_l2, 1);
				// TODO: �����ص�L1��
				// ���L1miss����һ����Ҫ������ȥ
//                if( hit_index_l1 < 0 ){
//                    free_index_l1 = get_cache_free_line(set_base_l1, 0);
//                    if(free_index_l1 >= 0){
//                        set_cache_line((uint)free_index_l1, addr, 0);
//                        //��Ҫmiss++��
//                    }else{
//                        printf("I should not be here");
//                    }
//                }
			}
			else if(oper_style == Cache_operation::UNLOCK)
			{
				unlock_cache_line((uint)hit_index_l2, 1);
			}
		}
		else 
		{
			// lock miss
			if (oper_style == Cache_operation::LOCK)
			{
				cache_miss_count[1]++;
				free_index_l2 = get_cache_free_line(set_base_l2, 1);
				if (free_index_l2 >= 0) 
				{
					set_cache_line((uint)free_index_l2, addr, 1);
					lock_cache_line((uint)free_index_l2, 1);
					// TODO: ����L1
					// ͬʱ��Ҫ�鿴L1�Ƿ�hit
//                    if(hit_index_l1 < 0){
//                        free_index_l1 = get_cache_free_line(set_base_l1, 0);
//                        if(free_index_l1 >= 0){
//                            set_cache_line((uint)free_index_l1, addr, 0);
//                            //��Ҫmiss++��
//                        }else{
//                            printf("I should not be here.");
//                        }
//                    }
				}
				else 
				{
					//����ֵӦ��ȷ����>=0��
					printf("I should not be here.");
				}
			}
			else 
			{
				// miss��unlock �Ȳ���
			}
		}
	}
	else 
	{
		// L1������
		if (hit_index_l1 >= 0) 
		{
			// һ����read����write��lock���Ѿ���ǰ�洦����ˡ�
			cache_hit_count[0]++;
			//ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
			if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[0])
				caches[0][hit_index_l1].lru_count = tick_count;
			//ֱ��Ĭ������Ϊд�ط�����Ҫ�滻�����������˵�ʱ���д�ء�
			//�����ˣ�����Ǹ����ݣ���ֱ��д�أ����ǵ��´Σ���û�����У�����ǡ��ƥ�䵽�˵�ǰline��ʱ����ʱ�ı�Ǿ��������ˣ�������д���ڴ�
			//TODO: error :�Ȳ��ÿ���д�ص����⣬���ﰴ���룬��Ӧ��ֱ�Ӵ�L1д�ص��ڴ�
			if (oper_style == Cache_operation::WRITE)
			{
				// �������������
//                caches[0][hit_index_l1].flag |= CACHE_FLAG_DIRTY;
				// L2���У���������д�뵽L2
				if (hit_index_l2 >= 0) 
				{
					cache_hit_count[1]++;
					caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
					if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[1])
					{
						caches[1][hit_index_l2].lru_count = tick_count;
					}
					//���L2miss����ô�ҵ�һ���¿飬������д��L2
				}
				else 
				{
					//��Ҫ���cache2 miss count���Ȳ�����
					cache_miss_count[1]++;
					free_index_l2 = get_cache_free_line(set_base_l2, 1);
					set_cache_line((uint)free_index_l2, addr, 1);
					caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
				}
			}
		}
		else 
		{
			// L1 miss
			cache_miss_count[0]++;
			// �Ȳ鿴L2�Ƿ�hit�����hit��ֱ��ȡ��������������
			if (hit_index_l2 >= 0) 
			{
				// ��L2��hit, ��Ҫд�ص�L1��
				cache_hit_count[1]++;
				free_index_l1 = get_cache_free_line(set_base_l1, 0);
				set_cache_line((uint)free_index_l1, addr, 0);
			}
			else 
			{
				// not in L2,���ڴ���ȡ
				cache_miss_count[1]++;
				free_index_l2 = get_cache_free_line(set_base_l2, 1);
				set_cache_line((uint)free_index_l2, addr, 1);
				free_index_l1 = get_cache_free_line(set_base_l1, 0);
				set_cache_line((uint)free_index_l1, addr, 0);
			}
		}
		//Fix BUG:�ڽ�CachesimӦ�õ�����Ӧ����ʱ������tickcountû�����ӣ����������¡���Ȼ�ᵼ���滻�㷨ʧЧ��
		// Bug Fix: ��hm�У���Ҫͨ���ⲿ��������tickcount++,���ڻ�������Ϊʲô��
//    tick_count++;
	}
}

///**���ļ���ȡtrace�����������޸�Ŀ���Ϊ��������Ŀ��������Ҫ�ĵ�*/
//void Cache::load_trace(const char* filename) 
//{
//	char buf[128];
//	// ����Լ���input·��
//	FILE* fin;
//	// ��¼����trace��ָ��Ķ�д������cache���ƣ��������Ķ�д������Ȼ��һ��������Ҫ��������õ�д�ط�����д�����cache�У�ֱ�����滻��
//	uint rcount = 0, wcount = 0;
//	fin = fopen(filename, "r");
//	if (!fin) 
//	{
//		printf("load_trace %s failed\n", filename);
//		return;
//	}
//	while (fgets(buf, sizeof(buf), fin)) 
//	{
//		_u8 style = 0;
//		// ԭ�������õ�ָ�룬�о���ȫû��Ҫ�����Һ�������ǿ������ת��ʵ�����������⡣addr�������һ����ֵ��32λunsigned int��
//		uint addr = 0;
//		sscanf(buf, "%c %x", &style, &addr);
//		do_cache_op(addr, style);
//		switch (style) 
//		{
//		case 'l':
//			rcount++;
//			break;
//		case 's':
//			wcount++;
//			break;
//		case 'k':
//			break;
//		case 'u':
//			break;
//
//		}
//	}
//	// ָ��ͳ��
//	printf("all r/w/sum: %lld %lld %lld \nread rate: %f%%\twrite rate: %f%%\n",
//		rcount, wcount, tick_count,
//		100.0 * rcount / tick_count,
//		100.0 * wcount / tick_count
//	);
//	// miss��
//	printf("L1 miss/hit: %lld/%lld\t hit/miss rate: %f%%/%f%%\n",
//		cache_miss_count[0], cache_hit_count[0],
//		100.0 * cache_hit_count[0] / (cache_hit_count[0] + cache_miss_count[0]),
//		100.0 * cache_miss_count[0] / (cache_miss_count[0] + cache_hit_count[0]));
//	printf("L2 miss/hit: %lld/%lld\t hit/miss rate: %f%%/%f%%\n",
//		cache_miss_count[1], cache_hit_count[1],
//		100.0 * cache_hit_count[1] / (cache_hit_count[1] + cache_miss_count[1]),
//		100.0 * cache_miss_count[1] / (cache_miss_count[1] + cache_hit_count[1]));
//	// ��дͨ��
////    printf("read : %d Bytes \t %dKB\n write : %d Bytes\t %dKB \n",
////           cache_r_count * cache_line_size,
////           (cache_r_count * cache_line_size) >> 10,
////           cache_w_count * cache_line_size,
////           (cache_w_count * cache_line_size) >> 10);
//	fclose(fin);
//
//}


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
	cacheReq.memSysReqQueueIndex = memReq.memSysReqQueueIndex;

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

bool Cache::addTransaction(MemReq _req)
{
	bool addSuccess = 0;
	if (reqQueue.size() >= 1)
	{
		deque<pair<CacheReq, uint>> &reqQueue_L1 = reqQueue[0];

		for (size_t i = 0; i < reqQueue_L1.size(); ++i)
		{
			if (!reqQueue_L1[i].first.valid)
			{
				//_req.spmReqQueueIndex = i;  // Record the entry of reqQueue in SPM
				reqQueue_L1[i].first = transMemReq2CacheReq(_req);
				reqQueue_L1[i].second = cache_access_latency[0];  // Emulate L1 cache access latency
				addSuccess = 1;
				break;
			}
		}
	}
	else
	{
		Debug::throwError("Not configure L1 cache!", __FILE__, __LINE__);
	}

	return addSuccess;
}

