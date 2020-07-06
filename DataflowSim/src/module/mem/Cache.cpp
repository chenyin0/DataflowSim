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
    sendPtr.resize(CACHE_MAXLEVEL);

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
    // ָ��������Ҫ�������滻���Ե�ʱ���ṩ�Ƚϵ�key�������л���miss��ʱ����Ӧline������Լ���countΪ��ʱ��tick_count;
    tick_count = 0;
    //    cache_buf = (_u8 *) malloc(cache_size);
    //    memset(cache_buf, 0, this->cache_size);
        // Ϊÿһ�з���ռ�
    //for (int i = 0; i < 2; ++i) 
    //{
    //    caches[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * cache_line_num[i]);
    //    memset(caches[i], 0, sizeof(Cache_Line) * cache_line_num[i]);
    //}
    ////����ʱ��Ĭ������
    //swap_style[0] = Cache_swap_style::CACHE_SWAP_LRU;
    //swap_style[1] = Cache_swap_style::CACHE_SWAP_LRU;
    re_init();
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
    /**ѭ�����ҵ�ǰset������way��line����ͨ��tagƥ�䣬�鿴��ǰ��ַ�Ƿ���cache��*/
    uint i;
    for (i = 0; i < cache_mapping_ways[level]; ++i) 
    {
        if ((caches[level][set_base + i].flag & CACHE_FLAG_VALID) &&
            (caches[level][set_base + i].tag == getCacheTag(addr, level) /*(addr >> (cache_set_shifts[level] + cache_line_shifts[level]))*/)) 
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

/**������д��cache line*/
void Cache::set_cache_line(uint index, uint addr, int level) 
{
    Cache_Line* line = caches[level] + index;
    // ����ÿ��line��buf������cache���buf���ظ��Ķ��Ҳ�û��������ݡ�
//    line->buf = cache_buf + cache_line_size * index;
    // �������line��tagλ
    //line->tag = addr >> (cache_set_shifts[level] + cache_line_shifts[level]);
    line->tag = getCacheTag(addr, level);
    line->flag = (_u8)~CACHE_FLAG_MASK;
    line->flag |= CACHE_FLAG_VALID;
    line->count = tick_count;
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

///**����Ҫ��level*/
//void Cache::do_cache_op(uint addr, Cache_operation oper_style) 
//{
//    uint set_l1, set_l2, set_base_l1, set_base_l2;
//    long long hit_index_l1, hit_index_l2, free_index_l1, free_index_l2;
//    tick_count++;
//    if (oper_style == Cache_operation::READ)
//        cache_r_count++;
//    if (oper_style == Cache_operation::WRITE)
//        cache_w_count++;
//    set_l2 = (addr >> cache_line_shifts[1]) % cache_set_size[1];
//    set_base_l2 = set_l2 * cache_mapping_ways[1];
//    hit_index_l2 = check_cache_hit(set_base_l2, addr, 1);
//    set_l1 = (addr >> cache_line_shifts[0]) % cache_set_size[0];
//    set_base_l1 = set_l1 * cache_mapping_ways[0];
//    hit_index_l1 = check_cache_hit(set_base_l1, addr, 0);
//
//    // lock��������ֻ����L2�ġ�(L2 is shared)
//    if (oper_style == Cache_operation::LOCK || oper_style == Cache_operation::UNLOCK)
//    {
//        if (hit_index_l2 >= 0) 
//        {
//            if (oper_style == Cache_operation::LOCK)
//            {
//                cache_hit_count[1]++;
//                if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[1])
//                {
//                    caches[1][hit_index_l2].lru_count = tick_count;
//                }
//                lock_cache_line((uint)hit_index_l2, 1);
//                // TODO: �����ص�L1��
//                // ���L1miss����һ����Ҫ������ȥ
////                if( hit_index_l1 < 0 ){
////                    free_index_l1 = get_cache_free_line(set_base_l1, 0);
////                    if(free_index_l1 >= 0){
////                        set_cache_line((uint)free_index_l1, addr, 0);
////                        //��Ҫmiss++��
////                    }else{
////                        printf("I should not be here");
////                    }
////                }
//            }
//            else if(oper_style == Cache_operation::UNLOCK)
//            {
//                unlock_cache_line((uint)hit_index_l2, 1);
//            }
//        }
//        else 
//        {
//            // lock miss
//            if (oper_style == Cache_operation::LOCK)
//            {
//                cache_miss_count[1]++;
//                free_index_l2 = get_cache_free_line(set_base_l2, 1);
//                if (free_index_l2 >= 0) 
//                {
//                    set_cache_line((uint)free_index_l2, addr, 1);
//                    lock_cache_line((uint)free_index_l2, 1);
//                    // TODO: ����L1
//                    // ͬʱ��Ҫ�鿴L1�Ƿ�hit
////                    if(hit_index_l1 < 0){
////                        free_index_l1 = get_cache_free_line(set_base_l1, 0);
////                        if(free_index_l1 >= 0){
////                            set_cache_line((uint)free_index_l1, addr, 0);
////                            //��Ҫmiss++��
////                        }else{
////                            printf("I should not be here.");
////                        }
////                    }
//                }
//                else 
//                {
//                    //����ֵӦ��ȷ����>=0��
//                    printf("I should not be here.");
//                }
//            }
//            else 
//            {
//                // miss��unlock �Ȳ���
//            }
//        }
//    }
//    else 
//    {
//        // L1������
//        if (hit_index_l1 >= 0) 
//        {
//            // һ����read����write��lock���Ѿ���ǰ�洦����ˡ�
//            cache_hit_count[0]++;
//            //ֻ����LRU��ʱ��Ÿ���ʱ�������һ������ʱ������ڱ��������ݵ�ʱ�����Է���FIFO
//            if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[0])
//                caches[0][hit_index_l1].lru_count = tick_count;
//            //ֱ��Ĭ������Ϊд�ط�����Ҫ�滻�����������˵�ʱ���д�ء�
//            //�����ˣ�����Ǹ����ݣ���ֱ��д�أ����ǵ��´Σ���û�����У�����ǡ��ƥ�䵽�˵�ǰline��ʱ����ʱ�ı�Ǿ��������ˣ�������д���ڴ�
//            //TODO: error :�Ȳ��ÿ���д�ص����⣬���ﰴ���룬��Ӧ��ֱ�Ӵ�L1д�ص��ڴ�
//            if (oper_style == Cache_operation::WRITE)
//            {
//                // �������������
////                caches[0][hit_index_l1].flag |= CACHE_FLAG_DIRTY;
//                // L2���У���������д�뵽L2
//                if (hit_index_l2 >= 0) 
//                {
//                    cache_hit_count[1]++;
//                    caches[1][hit_index_l2].flag |= CACHE_FLAG_DIRTY;
//                    if (Cache_swap_style::CACHE_SWAP_LRU == swap_style[1])
//                    {
//                        caches[1][hit_index_l2].lru_count = tick_count;
//                    }
//                    //���L2miss����ô�ҵ�һ���¿飬������д��L2
//                }
//                else 
//                {
//                    //��Ҫ���cache2 miss count���Ȳ�����
//                    cache_miss_count[1]++;
//                    free_index_l2 = get_cache_free_line(set_base_l2, 1);
//                    set_cache_line((uint)free_index_l2, addr, 1);
//                    caches[1][free_index_l2].flag |= CACHE_FLAG_DIRTY;
//                }
//            }
//        }
//        else 
//        {
//            // L1 miss
//            cache_miss_count[0]++;
//            // �Ȳ鿴L2�Ƿ�hit�����hit��ֱ��ȡ��������������
//            if (hit_index_l2 >= 0) 
//            {
//                // ��L2��hit, ��Ҫд�ص�L1��
//                cache_hit_count[1]++;
//                free_index_l1 = get_cache_free_line(set_base_l1, 0);
//                set_cache_line((uint)free_index_l1, addr, 0);
//            }
//            else 
//            {
//                // not in L2,���ڴ���ȡ
//                cache_miss_count[1]++;
//                free_index_l2 = get_cache_free_line(set_base_l2, 1);
//                set_cache_line((uint)free_index_l2, addr, 1);
//                free_index_l1 = get_cache_free_line(set_base_l1, 0);
//                set_cache_line((uint)free_index_l1, addr, 0);
//            }
//        }
//        //Fix BUG:�ڽ�CachesimӦ�õ�����Ӧ����ʱ������tickcountû�����ӣ����������¡���Ȼ�ᵼ���滻�㷨ʧЧ��
//        // Bug Fix: ��hm�У���Ҫͨ���ⲿ��������tickcount++,���ڻ�������Ϊʲô��
////    tick_count++;
//    }
//}

///**���ļ���ȡtrace�����������޸�Ŀ���Ϊ��������Ŀ��������Ҫ�ĵ�*/
//void Cache::load_trace(const char* filename) 
//{
//    char buf[128];
//    // ����Լ���input·��
//    FILE* fin;
//    // ��¼����trace��ָ��Ķ�д������cache���ƣ��������Ķ�д������Ȼ��һ��������Ҫ��������õ�д�ط�����д�����cache�У�ֱ�����滻��
//    uint rcount = 0, wcount = 0;
//    fin = fopen(filename, "r");
//    if (!fin) 
//    {
//        printf("load_trace %s failed\n", filename);
//        return;
//    }
//    while (fgets(buf, sizeof(buf), fin)) 
//    {
//        _u8 style = 0;
//        // ԭ�������õ�ָ�룬�о���ȫû��Ҫ�����Һ�������ǿ������ת��ʵ�����������⡣addr�������һ����ֵ��32λunsigned int��
//        uint addr = 0;
//        sscanf(buf, "%c %x", &style, &addr);
//        do_cache_op(addr, style);
//        switch (style) 
//        {
//        case 'l':
//            rcount++;
//            break;
//        case 's':
//            wcount++;
//            break;
//        case 'k':
//            break;
//        case 'u':
//            break;
//
//        }
//    }
//    // ָ��ͳ��
//    printf("all r/w/sum: %lld %lld %lld \nread rate: %f%%\twrite rate: %f%%\n",
//        rcount, wcount, tick_count,
//        100.0 * rcount / tick_count,
//        100.0 * wcount / tick_count
//    );
//    // miss��
//    printf("L1 miss/hit: %lld/%lld\t hit/miss rate: %f%%/%f%%\n",
//        cache_miss_count[0], cache_hit_count[0],
//        100.0 * cache_hit_count[0] / (cache_hit_count[0] + cache_miss_count[0]),
//        100.0 * cache_miss_count[0] / (cache_miss_count[0] + cache_hit_count[0]));
//    printf("L2 miss/hit: %lld/%lld\t hit/miss rate: %f%%/%f%%\n",
//        cache_miss_count[1], cache_hit_count[1],
//        100.0 * cache_hit_count[1] / (cache_hit_count[1] + cache_miss_count[1]),
//        100.0 * cache_miss_count[1] / (cache_miss_count[1] + cache_hit_count[1]));
//    // ��дͨ��
////    printf("read : %d Bytes \t %dKB\n write : %d Bytes\t %dKB \n",
////           cache_r_count * cache_line_size,
////           (cache_r_count * cache_line_size) >> 10,
////           cache_w_count * cache_line_size,
////           (cache_w_count * cache_line_size) >> 10);
//    fclose(fin);
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
    //cacheReq.cnt = memReq.cnt;

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
    memReq.memSysReqQueueIndex = cacheReq.memSysReqQueueIndex;
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
        //if (reqQueueBank.size() < reqQueueSizePerBank[level])
        //{
        //    reqQueueBank.push_back(make_pair(cacheReq, cache_access_latency[level]));
        //    return true;  // Send successfully
        //}
        //else
        //{
        //    return false;  // Send failed
        //}
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
            //if (reqQueueBank.size() < reqQueueSizePerBank[level])
            //{
            //    reqQueueBank.push_back(make_pair(cacheReq, cache_access_latency[level]));
            //    return true;  // Send successfully
            //}
            //else
            //{
            //    return false;  // Send failed
            //}
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
        if (req.first.valid && ((addr >> cache_line_shifts[level]) == (req.first.addr >> cache_line_shifts[level])))
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

//vector<MemReq> Cache::callBackInOrder()
//{
//    vector<MemReq> readyReq;
//    uint cnt = (std::numeric_limits<uint>::max)();  // Initial the max value
//    uint _bankId = 0;
//    uint _entryId = 0;
//    bool getValid = 0;
//    bool getReq = 0;
//
//    if (reqQueue.size() >= 1)
//    {
//        while (1)
//        {
//            for (size_t bankId = 0; bankId < reqQueue[0].size(); ++bankId)
//            {
//                auto reqQueueBank = reqQueue[0][bankId];
//                for (size_t entryId = 0; entryId < reqQueueBank.size(); ++entryId)
//                {
//                    auto req = reqQueueBank[entryId].first;
//                    if (req.valid && req.ready)
//                    {
//                        getValid = 1;
//                        if (req.cnt < cnt)
//                        {
//                            cnt = req.cnt;
//                            _entryId = entryId;
//                            _bankId = bankId;
//                            getReq = 1;
//                        }
//                    }
//                }
//            }
//
//            if (getValid)
//            {
//                auto& req = reqQueue[0][_bankId][_entryId];
//                readyReq.push_back(transCacheReq2MemReq(req.first));
//                req.first.valid = 0;
//            }
//
//            if (!getReq)
//            {
//                break;
//            }
//
//            getValid = 0;
//            getReq = 0;
//        }
//    }
//    else
//    {
//        Debug::throwError("Not configure L1 cache!", __FILE__, __LINE__);
//    }
//
//    return readyReq;
//}

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
                if (req.second == 0 && req.first.valid && !req.first.ready)
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
    // Update reqQueue latency
    reqQueueUpdate();
    // Send req from reqQueue to each cache level
    updateReqQueueOfCacheLevel();
    // Update each cache level according to next level req's status (e.g. If L2 hit, update L1 cacheline status)
    updateCacheLine();
}

// For debug
#ifdef DEBUG_MODE
const vector<vector<ReqQueueBank>>& Cache::getReqQueue() const
{
    return reqQueue;
}

#endif