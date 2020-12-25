#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "./Registry.h"
#include "../sim/Debug.h"

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
        Profiler(Registry* _registry, Debug* _debugger);
        ~Profiler();
        void init();
        static void recordComputingCycle(uint _moduleId, ChanType _chanType, uint _clk);
        static void recordMemoryAccessNum(uint _moduleId, ChanType _chanType, uint _clk);
        void updateBufferMaxDataNum();  // Statistic the peak data number in buffer for path balancing
        void printBufferMaxDataNum(string chanName, Channel* chanPtr);

    private:
        vector<ProfilingTable> profilingTable;
        vector<uint> maxDataNum;  // Record each buffer's peak data number
        Registry* registry = nullptr;
        Debug* debugger = nullptr;
    };
}
