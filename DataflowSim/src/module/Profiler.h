#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "./Registry.h"

namespace DFSim
{
    struct ProfilingTable
    {
        uint moduleId = 0;
        uint computingCycle = 0;
        uint memoryAccessNum = 0;
        uint lastUpdateClk = 0;  // Avoid double counting
    };

    class Profiler
    {
    public:
        Profiler(Registry* _registry);
        static void recordComputingCycle(uint _moduleId, ChanType _chanType, uint _clk);
        static void recordMemoryAccessNum(uint _moduleId, ChanType _chanType, uint _clk);

    private:
        vector<ProfilingTable> profilingTable;
    };
}
