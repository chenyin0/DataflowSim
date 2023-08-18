#include "./MsgCoalescer.h"
#include "../../sim/Debug.h"

using namespace DFSim;

MsgCoalescer::MsgCoalescer(uint64_t _entryNum, uint64_t _entrySize, uint64_t _wayNum)
{
    entryNum = _entryNum;
    entrySize = _entrySize;
    wayNum = _wayNum;

    if (entrySize % wayNum != 0)
    {
        Debug::throwError("Entry size cannot be divided exactly by the number of way!", __FILE__, __LINE__);
    }
    else
    {
        setNum = entryNum / wayNum;
    }

    msgCoalescerTable.resize(entryNum);
    // for (auto &entry : msgCoalescerTable)
    // {
    //     entry.coalescerQueue.resize(entrySize);
    // }
}

uint64_t MsgCoalescer::getEntryBaseIndex(uint64_t vertexId)
{
    return vertexId % setNum * wayNum;
}

bool MsgCoalescer::send2MsgCoalescer(uint64_t dstVertexId, uint64_t currVertexId)
/*
Potential bug: One single entry only can coalesce one msg in a round (Current version has no limitation)
*/
{
    uint64_t emptyEntryIndex = 0;
    bool findEmptyEntry = false;
    uint64_t entryBase = getEntryBaseIndex(dstVertexId);
    for (auto i = entryBase; i < entryBase + wayNum; ++i)
    {
        if (msgCoalescerTable[i].valid && dstVertexId == msgCoalescerTable[i].vertexId)
        {
            if (msgCoalescerTable[i].coalescerQueue.size() < entrySize)
            {
                msgCoalescerTable[i].coalescerQueue.push_back(currVertexId);
                return true;
            }
        }
        else if (!msgCoalescerTable[i].valid)
        {
            emptyEntryIndex = i; // Record emptyEntryIndex
            findEmptyEntry = true;
        }
    }

    if (findEmptyEntry)
    {
        msgCoalescerTable[emptyEntryIndex].valid = true;
        msgCoalescerTable[emptyEntryIndex].vertexId = dstVertexId;
        msgCoalescerTable[emptyEntryIndex].coalescerQueue.push_back(currVertexId);
        return true;
    }
    else
    {
        return false;
    }
}

void MsgCoalescer::stateUpdate()
{
    for (auto &entry : msgCoalescerTable)
    {
        if (entry.valid && entry.coalescerQueue.size() > 1)
        {
            entry.coalescerQueue.pop_front(); // Make a coalesce operation
        }
    }
}

pair<bool, deque<uint64_t>> MsgCoalescer::popMsgCoalescer()
{
    for (auto i = 0; i < entryNum; ++i)
    {
        if (msgCoalescerTable[coalescerRdPtr].valid)
        {
            auto msg = msgCoalescerTable[coalescerRdPtr].coalescerQueue;
            msgCoalescerTable[coalescerRdPtr].valid = false;
            msgCoalescerTable[coalescerRdPtr].coalescerQueue.clear();
            coalescerRdPtr = (++coalescerRdPtr) % entryNum;
            return make_pair(true, msg);
        }
    }

    return make_pair(false, deque<uint64_t>());
}