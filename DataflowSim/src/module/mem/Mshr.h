#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim
{
    struct MshrEntry
    {
        MshrEntry() = default;

        uint64_t blockAddr = 0;
        deque<CacheReq> mshrQueue;
        bool valid = 0;
        bool ready = 0;
        bool outstanding = 0;  // Set to 1, signify wait to send req to next cache level
    };

    class Mshr
    {
    public:
        Mshr(uint64_t _entryNum, uint64_t _entrySize);
        void mshrInit(uint64_t _entryNum, uint64_t _entrySize);
        bool send2Mshr(uint64_t _blockAddr, CacheReq _cacheReq);  // Cache miss, send to MSHR (blockAddr = addr >> log2(cacheline_size))
        void setMshrEntryReady(uint64_t _blockAddr);  // Get a new cache_block, set corresponding MSHR entry ready if MSHR hit
        //bool checkMshrReady();  // Check MSHR before getFromMshr
        vector<pair<uint64_t, CacheReq>> peekMshrReadyEntry();  // Peek the first req of each ready MSHR entry, <EntryId, CacheReq>
        void clearMshrEntry(vector<uint64_t> _entryIdVec);
        bool seekMshrFreeEntry();
        vector<pair<uint64_t, CacheReq>> getOutstandingReq();  // <entryId, req>
        void clearOutstandingFlag(vector<uint64_t> entryIdVec);
        //CacheReq getFromMshr();  // Before getFromMshr, must check whether MSHR is ready!

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<MshrEntry>& getMshrTable() const;
#endif // DEBUG_MODE

    private:
        uint64_t entryNum = 0;
        uint64_t entrySize = 0;
        vector<MshrEntry> mshrTable;
        uint64_t mshrRdPtr = 0;
    };
}