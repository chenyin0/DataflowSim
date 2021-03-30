#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim 
{
    struct CoalescerEntry
    {
        CoalescerEntry() = default;

        uint blockAddr = 0;
        deque<MemReq> coalescerQueue;
        bool valid = 0;
        bool ready = 0;
    };

    class Coalescer
    {
        friend class MemSystem;
    public:
        Coalescer(uint _entryNum, uint _entrySize);
        void coalescerInit(uint _entryNum, uint _entrySize);
        bool send2Coalescer(uint _blockAddr, vector<MemReq> _reqVec);
        uint searchCoalescer(uint _blockAddr);  // Search and return corresponding coalescer entry
        void popCoalescerEntry(uint entryId);
        //void writeBackCoalescer(uint _blockAddr);  // When receive a coaleced ack, set corresponding entry ready in coalescor
        bool checkCoalescerReady();  // Coalescer has the highest priority 
        //MemReq getFromCoalescer();  // Before getFromCoaleser, must check whether coalescer is ready
        uint getCoalescerOccupiedEntryNum();

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const deque<CoalescerEntry>& getCoalescerTable() const;
#endif // DEBUG_MODE

    private:
        uint entryNum = 0;
        uint entrySize = 0;
        deque<CoalescerEntry> coalescerTable;  // Fifo mode, only support spatial coalescing, not support temporal coalescing
        uint coalescerRdPtr = 0;
    };

}