#pragma once
#include "../define/Define.hpp"
//#include "../sim/Debug.h"

namespace DFSim
{
    enum class App_name
    {
        SimpleFlowTest,
        MemoryTest,
        Gemm,
        Bfs,
        Aes,
        Lud
    };

    class App_name_convert
    {
    public:
        static string toString(App_name type_)
        {
            if (type_ == App_name::SimpleFlowTest)
                return "SimpleFlowTest";
            if (type_ == App_name::MemoryTest)
                return "MemoryTest";
            if (type_ == App_name::Gemm)
                return "Gemm";
            if (type_ == App_name::Bfs)
                return "Bfs";
            if (type_ == App_name::Aes)
                return "Aes";
            if (type_ == App_name::Lud)
                return "Lud";
            //Debug::throwError("tb_name_convert to string error!", __FILE__, __LINE__);
            return "";
        }
    };

    enum class ArchType
    {
        Base,  // Plasticine 64 pe
        DGSF,  // 48 pe
        SGMF,  // 32 pe
        TIA    // 12 pe
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
        Chan_Lse,
        Chan_PartialMux
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

    enum class Alu_op
    {
        Nop,
        //** Algorithm
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Mac,
        //** Logic
        Cmp,
        Sel,
        And,
        Or,
        Xor,
        Not,
        Shl,  // Left shift
        Shr,  // Right shift
        //** Ld/St
        Load,
        Store,
        //** User-define function
        Func
    };
}
