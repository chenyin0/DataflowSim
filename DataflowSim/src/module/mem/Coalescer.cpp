#include "./Coalescer.h"
#include "../../define/Para.h"
#include "../../sim/Debug.h"

using namespace DFSim;

Coalescer::Coalescer(uint _entryNum, uint _entrySize)
{
    coalescerInit(_entryNum, _entrySize);
}

void Coalescer::coalescerInit(uint _entryNum, uint _entrySize)
{
    entryNum = _entryNum;
    entrySize = _entrySize;

    coalescerTable.resize(_entryNum);
}

bool Coalescer::send2Coalescer(uint _blockAddr, vector<MemReq> _reqVec)
{
    //for (auto& entry : coalescerTable)
    //{
    //    if (entry.valid && entry.blockAddr == _blockAddr && entry.coalescerQueue.size() < entrySize)
    //    {
    //        entry.coalescerQueue.push_back(_reqVec);
    //        return true;
    //    }
    //}

    //// If not find a existed blockAddr, new one
    //for (auto& entry : coalescerTable)
    //{
    //    if (!entry.valid)
    //    {
    //        entry.valid = 1;
    //        entry.blockAddr = _blockAddr;
    //        entry.coalescerQueue.push_back(_memReq);
    //        return true;
    //    }
    //}

    //return false; 

    if (coalescerTable.size() < MEMSYS_COALESCER_ENTRY_NUM)
    {
        deque<MemReq> reqQueue;
        for (auto& req : _reqVec)
        {
            reqQueue.push_back(req);
        }

        CoalescerEntry entry;
        entry.valid = 1;
        entry.blockAddr = _blockAddr;
        entry.coalescerQueue = reqQueue;
        coalescerTable.push_back(entry);

        return true;
    }
    else
    {
        Debug::throwError("Try write a full coalescer", __FILE__, __LINE__);
    }
}

uint Coalescer::searchCoalescer(uint _blockAddr)
{
    for (size_t entryId = 0; entryId < coalescerTable.size(); ++entryId)
    {
        if (coalescerTable[entryId].blockAddr == _blockAddr)
        {
            return entryId;
        }
    }

    Debug::throwError("Not find an entry, this req is not a coalesced one", __FILE__, __LINE__);
}

void Coalescer::popCoalescerEntry(uint entryId)
{
    auto iter = coalescerTable.begin() + entryId;
    coalescerTable.erase(iter);
}

//void Coalescer::writeBackCoalescer(uint _blockAddr)
//{
//    bool match = 0;
//    for (auto ptr = coalescerTable.begin(); ptr != coalescerTable.end(); ++ptr)
//    {
//        if (ptr->valid && ptr->blockAddr == _blockAddr)
//        {
//            ptr->ready = 1;
//            match = 1;
//            break;
//        }
//    }
//
//    if (!match)
//    {
//        Debug::throwError("Can not find a match entry with this request in the coalescor", __FILE__, __LINE__);
//    }
//}

bool Coalescer::checkCoalescerReady()
{
    for (auto& entry : coalescerTable)
    {
        if (entry.valid && entry.ready)
        {
            return true;
        }
    }

    return false;
}

//MemReq Coalescer::getFromCoalescer()
//{
//    MemReq req;
//
//    for (size_t i = 0; i < coalescerTable.size(); ++i)
//    {
//        auto& entry = coalescerTable[coalescerRdPtr + i];
//        if (entry.valid && entry.ready)
//        {
//#ifdef DEBUG_MODE
//            if (entry.coalescerQueue.empty())
//            {
//                Debug::throwError("Try to get from an empty Coalescer queue!", __FILE__, __LINE__);
//            }
//#endif
//            req = entry.coalescerQueue.front();
//            entry.coalescerQueue.pop_front();
//            coalescerRdPtr = (coalescerRdPtr + i) % coalescerTable.size();   // Update coalescerRdPtr to this entry
//
//            // Clear a coalescerQueue if it is empty
//            if (entry.coalescerQueue.empty())
//            {
//                entry.valid = 0;
//                entry.ready = 0;
//
//                coalescerRdPtr = (++coalescerRdPtr) % coalescerTable.size();  // Current entry is finish, update coalescerRdPtr
//            }
//
//            break;
//        }
//    }
//
//    return req;
//}

uint Coalescer::getCoalescerOccupiedEntryNum()
{
    return coalescerTable.size();
}

#ifdef DEBUG_MODE  // Get private instance for debug
const deque<CoalescerEntry>& Coalescer::getCoalescerTable() const
{
    return coalescerTable;
}
#endif // DEBUG_MODE