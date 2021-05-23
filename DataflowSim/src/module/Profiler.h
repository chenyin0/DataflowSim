#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "./Registry.h"
#include "../sim/Debug.h"
#include "./mem/MemSystem.h"

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
        Profiler(Registry* _registry, MemSystem* _memSys, Debug* _debugger);
        ~Profiler();
        void init();
        // System performance
        static void recordComputingCycle(uint _moduleId, ChanType _chanType, uint _clk);
        // Memory access
        //static void recordMemoryAccessNum(uint _moduleId, ChanType _chanType, uint _clk);
        void printLseProfiling(string lseName, Lse* _lsePtr);
        void printCacheMissRate();
        // Channel status
        void updateBufferMaxDataNum();  // Record the peak data number in chanBuffer for path balancing
        void printBufferMaxDataNum(string chanName, Channel* chanPtr);
        void updateChanUtilization();
        void printChanProfiling();
        void printLseProfiling();

    private:
        vector<ProfilingTable> profilingTable;
        vector<uint> maxDataNum;  // Record each buffer's peak data number
        Registry* registry = nullptr;
        Debug* debugger = nullptr;
        MemSystem* memSys = nullptr;
    };
}
