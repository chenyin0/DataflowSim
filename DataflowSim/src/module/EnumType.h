#pragma once
#include "../define/Define.hpp"
//#include "../sim/Debug.h"

namespace DFSim
{
    enum class TestBench_name
    {
        SimpleFlowTest,
        MemoryTest,
        Gemm,
        Bfs,
        Aes
    };

    class tb_name_convert
    {
    public:
        static string toString(TestBench_name type_)
        {
            if (type_ == TestBench_name::SimpleFlowTest)
                return "SimpleFlowTest";
            if (type_ == TestBench_name::MemoryTest)
                return "MemoryTest";
            if (type_ == TestBench_name::Gemm)
                return "Gemm";
            if (type_ == TestBench_name::Bfs)
                return "Bfs";
            if (type_ == TestBench_name::Aes)
                return "Aes";
            //Debug::throwError("tb_name_convert to string error!", __FILE__, __LINE__);
            return "";
        }
    };

    enum class Debug_mode
    {
        Turn_off,
        Print_detail,
        Print_brief
    };

    enum class ModuleType
    {
        Channel,
        Lc,
        Mux
    };

    enum class ChanType
    {
        Chan_Base,
        Chan_DGSF,
        Chan_SGMF,
        Chan_Lse
    };

    // Cache replacement algorithm
    enum class Cache_swap_style
    {
        CACHE_SWAP_FIFO,
        CACHE_SWAP_LRU,
        CACHE_SWAP_RAND,
        CACHE_SWAP_MAX
    };

    enum class Cache_operation
    {
        READ,
        WRITE,
        LOCK,
        UNLOCK,
        WRITEBACK_DIRTY_BLOCK
    };

    enum class Cache_write_strategy
    {
        WRITE_BACK,
        WRITE_THROUGH
    };

    enum class Cache_write_allocate
    {
        WRITE_ALLOCATE,
        WRITE_NON_ALLOCATE
    };
}
