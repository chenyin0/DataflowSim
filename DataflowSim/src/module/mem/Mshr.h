#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim
{
    struct MshrEntry
    {
        MshrEntry() = default;

        uint blockAddr = 0;
        deque<CacheReq> mshrQueue;
        bool valid = 0;
        bool ready = 0;
        bool outstanding = 0;  // Set to 1, signify wait to send req to next cache level
    };

    class Mshr
    {
    public:
        Mshr(uint _entryNum, uint _entrySize);
        void mshrInit(uint _entryNum, uint _entrySize);
        bool send2Mshr(uint _blockAddr, CacheReq _cacheReq);  // Cache miss, send to MSHR (blockAddr = addr >> log2(cacheline_size))
        void setMshrEntryReady(uint _blockAddr);  // Get a new cache_block, set corresponding MSHR entry ready if MSHR hit
        //bool checkMshrReady();  // Check MSHR before getFromMshr
        vector<pair<uint, CacheReq>> peekMshrReadyEntry();  // Peek the first req of each ready MSHR entry, <EntryId, CacheReq>
        void clearMshrEntry(vector<uint> _entryIdVec);
        bool seekMshrFreeEntry();
        vector<pair<uint, CacheReq>> getOutstandingReq();  // <entryId, req>
        void clearOutstandingFlag(vector<uint> entryIdVec);
        //CacheReq getFromMshr();  // Before getFromMshr, must check whether MSHR is ready!

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<MshrEntry>& getMshrTable() const;
#endif // DEBUG_MODE

    private:
        uint entryNum = 0;
        uint entrySize = 0;
        vector<MshrEntry> mshrTable;
        uint mshrRdPtr = 0;
    };
}