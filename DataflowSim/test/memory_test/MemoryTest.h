#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/mem/Spm.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/ClkSys.h"
//#include "../src/module/execution/Lc.h"
#include "../../src/sim/Debug.h"

namespace DFSimTest
{
    using namespace DFSim;

    class MemoryTest
    {
    public:
        static void memory_test(Debug* debug);
    };
}
