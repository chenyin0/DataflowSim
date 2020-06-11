#pragma once
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
#include "./Spm.h"
#include "./Cache.h"
#include "../execution/Lse.h"

namespace DFSim
{
    class Lse;
    class MemoryDataBus;

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
        void sendBack2Lse();
        uint addrBias(uint _addr);  // Normally data is in byte, for different data width need to bias addr

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<Lse*>& getLseRegistry() const;
        const vector<MemReq>& getReqQueue() const;
        //const vector<pair<MemReq, uint>>& getBusDelayFifo() const;
#endif // DEBUG_MODE

    public:
        DRAMSim::MultiChannelMemorySystem* mem = nullptr;  // DRAM
        Spm* spm = nullptr;
        Cache* cache = nullptr;
        MemoryDataBus* memDataBus = nullptr;

    private:
        uint sendPtr = 0;  // SendPtr for reqQueue, round-robin
        vector<Lse*> lseRegistry;
        vector<MemReq> reqQueue;
        deque<MemReq> reqAckStack;  // Receive reqAck from memory data bus
    };


    class MemoryDataBus
    {
    public:
        MemoryDataBus();
        void mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
        void mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
        vector<MemReq> MemoryDataBusUpdate();
        //vector<MemReq> getFromBusDelayFifo();  // Interface function to get req back from memory bus

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const deque<pair<MemReq, uint>>& getBusDelayFifo() const;
#endif // DEBUG_MODE

    private:
        uint busDelay = BUS_DELAY;
        deque<pair<MemReq, uint>> busDelayFifo;  // Emulate bus delay when receive data from DRAM
    };
}