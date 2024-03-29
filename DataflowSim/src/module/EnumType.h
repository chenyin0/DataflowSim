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
        Lud,
        FFT,
        Viterbi,
        Cfd,
        Gesummv,
        Sort,
        Ge,
        Cholesky,
        HotSpot,
        GCN
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
                return "gemm";
            if (type_ == App_name::Bfs)
                return "bfs";
            if (type_ == App_name::Aes)
                return "aes";
            if (type_ == App_name::Lud)
                return "lud";
            if (type_ == App_name::FFT)
                return "fft";
            if (type_ == App_name::Viterbi)
                return "viterbi";
            if (type_ == App_name::Cfd)
                return "cfd";
            if (type_ == App_name::Gesummv)
                return "gesummv";
            if (type_ == App_name::Sort)
                return "sort";
            if (type_ == App_name::Ge)
                return "ge";
            if (type_ == App_name::Cholesky)
                return "cholesky";
            if (type_ == App_name::HotSpot)
                return "hotspot";
            if (type_ == App_name::GCN)
                return "gcn";
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
        SelPartial,
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
