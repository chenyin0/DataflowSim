#pragma once
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
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
        uint addrTag = 0;
        bool hasRegisteredCoalescer = 0;
        vector<MemReq> reqQueue;
    };

    class MemSystem
    {
    public:
        MemSystem();
        ~MemSystem();

        // Lse
        uint registerLse(Lse* _lse);  // Register Lse, return Id in LseRegistry
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

    private:
        void getLseReq();
        uint getBankId(uint _addr);  // Store in which bank
        uint getAddrTag(uint _addr);  // Upper address used to distinguish an individual block in coalescing (e.g. addr >> cachelineSize)
        void resetBankRecorder();
        void sendBack2Lse();
        //void writeAck2Lse(MemReq _req);
        uint addrBias(uint _addr);  // Normally data is in byte, for different data width need to bias addr
        int getAckQueueEntry(vector<MemReq> _queue);

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<Lse*>& getLseRegistry() const;
        const vector<deque<MemReq>>& getReqQueue() const;
        const vector<deque<MemReq>>& getAckQueue() const;
        const Coalescer& getCoalescer() const;
        //const vector<pair<MemReq, uint>>& getBusDelayFifo() const;
#endif // DEBUG_MODE

    public:
        DRAMSim::MultiChannelMemorySystem* mem = nullptr;  // DRAM
        Spm* spm = nullptr;
        Cache* cache = nullptr;
        MemoryDataBus* memDataBus = nullptr;

    private:
        //uint sendPtr = 0;  // SendPtr for reqQueue, round-robin
        uint reqQueueWritePtr = 0;  // Push Lse req to reqQueue, round-robin
        uint ackQueueReadPtr = 0;  // Pop Lse req from ackQueue, round-robin
        vector<Lse*> lseRegistry;
        vector<Lse*> lseReqTable;  // Record each lse_ptr (Note: if a Lse unrolls N times, it occupies N entries of this table)
        vector<deque<MemReq>> reqQueue;  // Vec1: #bank; Vec2: reqQueue_size (default size = 1, only transmit data to next memory hierarchy)
        vector<deque<MemReq>> ackQueue;  // Vec1: #bank; Vec2: ackQueue_size
        deque<MemReq> reqAckStack;  // Receive reqAck from memory data bus
        Coalescer coalescer = Coalescer(MEMSYS_COALESCER_ENTRY_NUM, MEMSYS_COALESCER_SIZY_PER_ENTRY);
        vector<bankRecorderEntry> bankRecorder;  // Emulate bank conflict, record bank visited status in each round
    };

}