#pragma once
#include "../../define/Define.hpp"

namespace DFSim
{
    struct MsgCoalescerEntry
    {
        MsgCoalescerEntry() = default;

        uint64_t vertexId = 0;
        deque<uint64_t> coalescerQueue; // Store the delta msg
        bool valid = 0;
        // bool ready = 0;
    };

    class MsgCoalescer
    {
    public:
        MsgCoalescer(uint64_t _entryNum, uint64_t _entrySize, uint64_t _wayNum);
        void update();
        bool send2MsgCoalescer(uint64_t dstVertexId, uint64_t currVertexId);
        void stateUpdate();
        pair<bool, deque<uint64_t>> popMsgCoalescer();

    private:
        uint64_t getEntryBaseIndex(uint64_t vertexId);

    private:
        uint64_t entryNum = 0;
        uint64_t wayNum = 0; // N-ways set-associative mapping
        uint64_t setNum = 0;
        uint64_t entrySize = 0; // coalescerQueue size of each entry
        deque<MsgCoalescerEntry> msgCoalescerTable;
        uint64_t coalescerRdPtr = 0;
    };
}