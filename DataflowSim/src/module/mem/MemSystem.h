#pragma once
//#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
#include "../../../../DRAMSim3/src/dramsim3.h"
#include "./Spm.h"
#include "./Cache.h"
#include "../execution/Lse.h"
#include "../mem/MemoryDataBus.h"
#include "./Coalescer.h"
#include "../../define/Para.h"

namespace DFSim
{
    class Lse;
    class MemoryDataBus;

    struct bankRecorderEntry
    {
        bankRecorderEntry() = default;
        bool valid = 0;
        uint64_t addrTag = 0;
        bool hasRegisteredCoalescer = 0;
        //bool hasSent2Mem = 0;
        deque<MemReq> reqQueue;
        //uint64_t rdPtr = 0; // Record current starting point to avoid operate a visited req agian
    };

    class MemSystem
    {
    public:
        MemSystem();
        ~MemSystem();

        // Lse
        uint64_t registerLse(Lse* _lse);  // Register Lse, return Id in LseRegistry
        //bool checkReqQueueIsFull();
        bool addTransaction(MemReq _req);  // Add transaction to MemSys (Interface function for Lse)

        // SPM
        void send2Spm();
        void getFromSpm();

        // Cache
        void send2Cache();
        void getFromCache();

        // MemoryDataBus
        void getReqAckFromMemoryDataBus(vector<MemReq> _reqAcks);
        void returnReqAck();  // return reqStack's reqs back to SPM/Cache

        void MemSystemUpdate();

        static void power_callback(double a, double b, double c, double d) {}  // Unused
        const uint64_t& getMemAccessCnt() const;

    private:
        void getLseReq();
        uint64_t getBankId(uint64_t _addr);  // Store in which bank
        uint64_t getAddrTag(uint64_t _addr);  // Upper address used to distinguish an individual block in coalescing (e.g. addr >> cachelineSize)
        void resetBankRecorder(uint64_t entryId);
        void sendBack2Lse();
        //void writeAck2Lse(MemReq _req);
        uint64_t addrBias(uint64_t _addr);  // Normally data is in byte, for different data width need to bias addr
        int getAckQueueEntry(vector<MemReq> _queue);
        void dramUpdate();  // Update DRAM

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<Lse*>& getLseRegistry() const { return lseRegistry; }
        const vector<deque<MemReq>>& getReqQueue() const { return reqQueue; }
        const vector<deque<MemReq>>& getAckQueue() const { return ackQueue; }
        const Coalescer& getCoalescer() const { return coalescer; }
        //const vector<pair<MemReq, uint64_t>>& getBusDelayFifo() const;
#endif // DEBUG_MODE

    public:
        //DRAMSim::MultiChannelMemorySystem* mem = nullptr;  // DRAM (DRAMSim2)
        dramsim3::MemorySystem* mem = nullptr;  // DRAM (DRAMSim3)
        Spm* spm = nullptr;
        Cache* cache = nullptr;
        MemoryDataBus* memDataBus = nullptr;

    private:
        //uint64_t sendPtr = 0;  // SendPtr for reqQueue, round-robin
        uint64_t reqQueueWritePtr = 0;  // Push Lse req to reqQueue, round-robin
        uint64_t ackQueueReadPtr = 0;  // Pop Lse req from ackQueue, round-robin
        vector<Lse*> lseRegistry;
        vector<Lse*> lseReqTable;  // Record each lse_ptr (Note: if a Lse unrolls N times, it occupies N entries of this table)
        vector<deque<MemReq>> reqQueue;  // Vec1: #bank; Vec2: reqQueue_size (default size = 1, only transmit data to next memory hierarchy)
        vector<deque<MemReq>> ackQueue;  // Vec1: #bank; Vec2: ackQueue_size
        deque<MemReq> reqAckStack;  // Receive reqAck from memory data bus
        Coalescer coalescer = Coalescer(MEMSYS_COALESCER_ENTRY_NUM, MEMSYS_COALESCER_SIZY_PER_ENTRY);
        vector<bankRecorderEntry> bankRecorder;  // Emulate bank conflict, record bank visited status in each round
        uint64_t coalescerFreeEntryNum = 0;  // Record free coalescer entry number
        // For DRAMSim3 update
        uint64_t hostCnt = 0;
        uint64_t dramCnt = 0;
        uint64_t hostClkFreqHz = 0;
        uint64_t dramsimClkFreqHz = 0;
    };

}