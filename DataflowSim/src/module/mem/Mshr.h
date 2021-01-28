#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"

namespace DFSim
{
    struct MshrEntry
    {
        MshrEntry() = default;

        uint blockAddr = 0;
        deque<CacheReq> mshrQueue;
        bool valid = 0;
        bool ready = 0;
    };

    class Mshr
    {
    public:
        Mshr(uint _entryNum, uint _entrySize);
        void mshrInit(uint _entryNum, uint _entrySize);
        bool send2Mshr(uint _blockAddr, CacheReq _cacheReq);  // Cache miss, send to MSHR (blockAddr = addr >> log2(cacheline_size))
        bool lookUpMshr(uint _blockAddr);  // Get a new cache_block, check MSHR
        bool checkMshrReady();  // Check MSHR before getFromMshr
        CacheReq getFromMshr();  // Before getFromMshr, must check whether MSHR is ready!

    private:
        uint entryNum = 0;
        uint entrySize = 0;
        vector<MshrEntry> mshrTable;
        uint mshrRdPtr = 0;
    };
}