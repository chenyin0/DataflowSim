#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim
{
    class MemoryDataBus
    {
    public:
        MemoryDataBus();
        void mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
        void mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
        vector<MemReq> MemoryDataBusUpdate();
        //vector<MemReq> getFromBusDelayFifo();  // Interface function to get req back from memory bus

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const deque<pair<MemReq, uint>>& getBusDelayFifo() const;
#endif // DEBUG_MODE

    private:
        uint busDelay = BUS_DELAY;
        deque<pair<MemReq, uint>> busDelayFifo;  // Emulate bus delay when receive data from DRAM
    };
}