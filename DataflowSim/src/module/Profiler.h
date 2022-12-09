#pragma once
//#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "./Registry.h"
#include "../sim/Debug.h"
#include "./mem/MemSystem.h"
#include "./execution/GraphScheduler.h"

namespace DFSim
{
    struct ProfilingTable
    {
        uint64_t moduleId = 0;
        uint64_t computingCycle = 0;
        uint64_t memoryAccessNum = 0;
        uint64_t lastUpdateClk = 0;  // Avoid double counting
    };

    class Profiler
    {
    public:
        Profiler(Registry* _registry, MemSystem* _memSys, Debug* _debugger);
        ~Profiler();
        void init();
        // System performance
        static void recordComputingCycle(uint64_t _moduleId, ChanType _chanType, uint64_t _clk);
        // Memory access
        //static void recordMemoryAccessNum(uint64_t _moduleId, ChanType _chanType, uint64_t _clk);
        void printLseProfiling(string lseName, Lse* _lsePtr);
        void printCacheMissRate();
        // Channel status
        void updateBufferMaxDataNum();  // Record the peak data number in chanBuffer for path balancing
        void printBufferMaxDataNum(string chanName, Channel* chanPtr);
        void updateChanUtilization();
        void updateChanUtilization(uint64_t _currSubgraphId);
        void printChanProfiling(GraphScheduler* _graphScheduler);
        void printLseProfiling();
        void printPowerProfiling();
        void tiaProfiling();
        void printDramProfiling();
        void printCacheProfiling();

    /*private:*/
        static float transEnergy2Power(float _energy);  // Energy: pJ; Power: mW
        static float transEnergy2Power(float _energy, uint64_t _cycle);  // Energy: pJ; Power: mW

    private:
        vector<ProfilingTable> profilingTable;
        vector<uint64_t> maxDataNum;  // Record each buffer's peak data number
        Registry* registry = nullptr;
        Debug* debug = nullptr;
        MemSystem* memSys = nullptr;

        // Channel
        uint64_t chanActiveNumTotal = 0;
        // GraphSchduler
        uint64_t lastSubgraphId = 0;
        uint64_t graphSwitchTimes = 0;
    };
}
