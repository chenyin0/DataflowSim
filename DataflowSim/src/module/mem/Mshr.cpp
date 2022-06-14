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
    bool sendSuccess = 0;
    for (auto& entry : mshrTable)
    {
        if (entry.valid && entry.blockAddr == _blockAddr && entry.mshrQueue.size() < entrySize)
        {
            entry.mshrQueue.emplace_back(_cacheReq);
            sendSuccess = 1;
            return sendSuccess;
        }
    }

    // If not find a existed blockAddr, new one
    for (auto& entry : mshrTable)
    {
        if (!entry.valid)
        {
            entry.valid = 1;
            entry.outstanding = 1;
            entry.blockAddr = _blockAddr;
            entry.mshrQueue.emplace_back(_cacheReq);
            sendSuccess = 1;
            return sendSuccess;
        }
    }

    return sendSuccess;
}

void Mshr::setMshrEntryReady(uint _blockAddr)
{
    bool mshrHit = 0;
    for (auto& entry : mshrTable)
    {
        if (entry.valid && entry.blockAddr == _blockAddr)
        {
            entry.ready = 1;
        }
    }
    //TODO: Need ensure RAW/WAR between different entries with the same blockAddr
}

//CacheReq Mshr::getFromMshr()
//{
//    CacheReq req;
//
//    for (size_t i = 0; i < mshrTable.size(); ++i)
//    {
//        auto& entry = mshrTable[mshrRdPtr + i];
//        if (entry.valid && entry.ready)
//        {
//#ifdef DEBUG_MODE
//            if (entry.mshrQueue.empty())
//            {
//                Debug::throwError("Try to get from an empty MSHR queue!", __FILE__, __LINE__);
//            }
//#endif
//            req = entry.mshrQueue.front();
//            entry.mshrQueue.pop_front();
//            mshrRdPtr = (mshrRdPtr + i) % mshrTable.size();   // Update mshrRdPtr to this entry
//
//            // Clear a mshrQueue if it is empty
//            if (entry.mshrQueue.empty())
//            {
//                entry.valid = 0;
//                entry.ready = 0;
//
//                mshrRdPtr = (++mshrRdPtr) % mshrTable.size();  // Current entry is finish, update mshrRdPtr
//            }
//
//            break;
//        }
//    }
//
//    return req;
//}

//bool Mshr::checkMshrReady()
//{
//    for (auto& entry : mshrTable)
//    {
//        if (entry.valid && entry.ready)
//        {
//            return true;
//        }
//    }
//
//    return false;
//}

vector<pair<uint, CacheReq>> Mshr::peekMshrReadyEntry()
{
    vector<pair<uint, CacheReq>> reqVec;
    for (size_t i = 0; i < mshrTable.size(); ++i)
    {
        uint entryId = (mshrRdPtr + i) % mshrTable.size();  // Round-robin
        auto& entry = mshrTable[entryId];
        if (entry.valid && entry.ready)
        {
            reqVec.emplace_back(std::make_pair(entryId, entry.mshrQueue.front()));
        }
    }
    mshrRdPtr = (++mshrRdPtr) % mshrTable.size();
    return reqVec;
}

void Mshr::clearMshrEntry(vector<uint> _entryIdVec)
{
    for (auto& entryId : _entryIdVec)
    {
        auto& entry = mshrTable[entryId];
        entry.mshrQueue.pop_front();
        // Clear a mshrQueue if it is empty
        if (entry.mshrQueue.empty())
        {
            entry.valid = 0;
            entry.ready = 0;
        }
    }
}

bool Mshr::seekMshrFreeEntry()
{
    for (auto& entry : mshrTable)
    {
        if (!entry.valid)
        {
            return true;
        }
    }

    return false;
}

vector<pair<uint, CacheReq>> Mshr::getOutstandingReq()
{
    vector<pair<uint, CacheReq>> reqVec;

    for (size_t entryId = 0; entryId < mshrTable.size(); ++entryId)
    {
        auto& entry = mshrTable[entryId];
        if (entry.valid && entry.outstanding)
        {
            reqVec.emplace_back(std::make_pair(entryId, entry.mshrQueue.front()));
        }
    }

    return reqVec;
}

void Mshr::clearOutstandingFlag(vector<uint> entryIdVec)
{
    for (auto& entryId : entryIdVec)
    {
        if (mshrTable[entryId].outstanding)
        {
            mshrTable[entryId].outstanding = 0;
        }
        else
        {
            Debug::throwError("This entry is not an outstanding req", __FILE__, __LINE__);
        }
    }
}

#ifdef DEBUG_MODE  // Get private instance for debug
const vector<MshrEntry>& Mshr::getMshrTable() const
{
    return mshrTable;
}
#endif // DEBUG_MODE