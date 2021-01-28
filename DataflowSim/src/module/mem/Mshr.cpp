#include "./Mshr.h"
#include "../../define/Para.h"
#include "../../sim/Debug.h"

using namespace DFSim;

Mshr::Mshr(uint _entryNum, uint _entrySize)
{
    mshrInit(_entryNum, _entrySize);
}

void Mshr::mshrInit(uint _entryNum, uint _entrySize)
{
    entryNum = _entryNum;
    entrySize = _entrySize;

    mshrTable.resize(_entryNum);
}

bool Mshr::send2Mshr(uint _blockAddr, CacheReq _cacheReq)
{
    for (auto& entry : mshrTable)
    {
        if (entry.valid && entry.blockAddr == _blockAddr && entry.mshrQueue.size() < entrySize)
        {
            entry.mshrQueue.push_back(_cacheReq);
            return true;
        }
    }

    // If not find a existed blockAddr, new one
    for (auto& entry : mshrTable)
    {
        if (!entry.valid)
        {
            entry.valid = 1;
            entry.blockAddr = _blockAddr;
            entry.mshrQueue.push_back(_cacheReq);
            return true;
        }
    }

    return false;  // MSHR fail, stall req
}

bool Mshr::lookUpMshr(uint _blockAddr)
{
    for (auto& entry : mshrTable)
    {
        if (entry.valid && entry.blockAddr == _blockAddr)
        {
            entry.ready = 1;
            return true;
        }
    }

    return false;
}

CacheReq Mshr::getFromMshr()
{
    CacheReq req;

    for (size_t i = 0; i < mshrTable.size(); ++i)
    {
        auto& entry = mshrTable[mshrRdPtr + i];
        if (entry.valid && entry.ready)
        {
#ifdef DEBUG_MODE
            if (entry.mshrQueue.empty())
            {
                Debug::throwError("Try to get from an empty MSHR queue!", __FILE__, __LINE__);
            }
#endif
            req = entry.mshrQueue.front();
            entry.mshrQueue.pop_front();
            mshrRdPtr = (mshrRdPtr + i) % mshrTable.size();   // Update mshrRdPtr to this entry

            // Clear a mshrQueue if it is empty
            if (entry.mshrQueue.empty())
            {
                entry.valid = 0;
                entry.ready = 0;

                mshrRdPtr = (++mshrRdPtr) % mshrTable.size();  // Current entry is finish, update mshrRdPtr
            }

            break;
        }
    }

    return req;
}

bool Mshr::checkMshrReady()
{
    for (auto& entry : mshrTable)
    {
        if (entry.valid && entry.ready)
        {
            return true;
        }
    }

    return false;
}