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
    Lud src code from Rodinia benchmark suite
*/

namespace DFSimTest
{
    using namespace DFSim;

    class LudTest
    {
    public:
        static void lud_Base(Debug* debug);
        static void lud_DGSF(Debug* debug);

    private:
        static void generateData();  // Generate benchmark data

        static vector<int> matrix;
        static uint matrix_size;

        // Performance parameter
        // Base
        static uint Base_outer_loop_speedup;
        static uint Base_inner_loop_speedup;

        //DGSF
        static uint DGSF_outer_loop_speedup;
        static uint DGSF_inner_loop_speedup;

        static uint DGSF_outer_loop_buffer_size;
    };
}