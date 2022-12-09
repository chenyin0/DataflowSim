#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim 
{
    struct CoalescerEntry
    {
        CoalescerEntry() = default;

        uint64_t blockAddr = 0;
        deque<MemReq> coalescerQueue;
        bool valid = 0;
        bool ready = 0;
    };

    class Coalescer
    {
        friend class MemSystem;
    public:
        Coalescer(uint64_t _entryNum, uint64_t _entrySize);
        void coalescerInit(uint64_t _entryNum, uint64_t _entrySize);
        bool send2Coalescer(uint64_t _blockAddr, deque<MemReq> _reqQueue);
        uint64_t searchCoalescer(uint64_t _blockAddr);  // Search and return corresponding coalescer entry
        void popCoalescerEntry(uint64_t entryId);
        //void writeBackCoalescer(uint64_t _blockAddr);  // When receive a coaleced ack, set corresponding entry ready in coalescor
        bool checkCoalescerReady();  // Coalescer has the highest priority 
        //MemReq getFromCoalescer();  // Before getFromCoaleser, must check whether coalescer is ready
        uint64_t getCoalescerOccupiedEntryNum();

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const deque<CoalescerEntry>& getCoalescerTable() const;
#endif // DEBUG_MODE

    private:
        uint64_t entryNum = 0;
        uint64_t entrySize = 0;
        deque<CoalescerEntry> coalescerTable;  // Fifo mode, only support spatial coalescing, not support temporal coalescing
        uint64_t coalescerRdPtr = 0;
    };

}