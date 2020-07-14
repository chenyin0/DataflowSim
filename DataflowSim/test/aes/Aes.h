#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/execution/Lc.h"
#include "../../src/module/execution/Mux.h"
#include "../../src/module/ClkSys.h"
#include "../../src/sim/Debug.h"
#include "../../src/module/Registry.h"

/*
    Aes src code
    from MachSuite

    Only mapping the kernel of Aes(the loop of the encryption)

*/

/*
//************************
DFG analyze:

    chan_aes_subBytes:      1pe,    16cycle (1cycle * 16loop)
    chan_aes_shiftRows:     16pe,   2cycle  (2cycle * 1loop)
    chan_aes_mixColumns:    31pe,   24cycle (6cycle * 4loop)

    chan_truePath_aes_addRoundKey:  16pe,   1cycle (16loop unrolling)

    chan_falsePath_aes_expandEncKey:    22pe,   4cycle
    chan_falsePath_aes_addRoundKey:  16pe,   1cycle (16loop unrolling)

//*************************
Config parameter:
    
    Array_total_pe_num = 128

    Base:
        truePath = 1
        falsePath = 1
        total_pe = 102

    DGSF:
        non_branch_pe_num = 48
        truePath = (128 - 48)/16 = 5
        falsePath = (128 - 48)/16 = 2

*/

namespace DFSimTest
{
    using namespace DFSim;

    class AesTest
    {
    public:
        static void aes_Base(Debug* debug);
        static void aes_DGSF(Debug* debug);

    private:
        // Performance parameter
        // Base
        static uint Base_speedup;

        //DGSF
        static uint DGSF_non_branch_speedup;
        static uint DGSF_truePath_speedup;
        static uint DGSF_falsePath_speedup;
    };
}