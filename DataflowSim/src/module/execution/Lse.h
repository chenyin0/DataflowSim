/*
Module Lse:

1. Trigger mode:
    Applicate scenario:
        Inner loop executes many times then trigger outer loop execute once
    //Usage:
    //    uint64_t trig_cnt = 3;  // Define trig_cnt in the benchmark_config.cpp
    //    Lse.get(isWrite, addr, &trig_cnt);  // get function decrease trig_cnt by reference &trig_cnt
    //    if(trig_cnt == 0) {  // Reset trig_cnt in config.cpp
    //        trig_cnt = 3;
    //    }

2. Lse can't be configured in keepMode!


TODO:
1. Trigger mode:
    Applicate scenario:
        Inner loop executes many times then trigger outer loop execute once
*/

#pragma once
#include "./Channel.h"
#include "../mem/MemSystem.h"

namespace DFSim
{
    class MemSystem;

    class Lse : public Channel
    {
    public:
        /*Lse(uint64_t _size, uint64_t _cycle, MemSystem* _memSys);
        Lse(uint64_t _size, uint64_t _cycle, MemSystem* _memSys, uint64_t _speedup);*/
        Lse(uint64_t _size, uint64_t _cycle, bool _isWrite, MemSystem *_memSys);
        Lse(uint64_t _size, uint64_t _cycle, bool _isWrite, MemSystem *_memSys, uint64_t _speedup);
        Lse(string _moduleName, uint64_t _size, uint64_t _cycle, bool _isWrite, MemSystem *_memSys, uint64_t _speedup);
        ~Lse();

        // void get(bool _isWrite, uint64_t _addr);  // Load/store addr
        // void get(vector<int> data);  // Load/store addr
        //  Trigger mode (e.g. inner loop executes many times then trigger outer loop execute once,
        //  or upper pe updates local reg many times then sends the data to lse once)
        // void get(vector<int> data, uint64_t& trig_cnt);
        /*void get(bool _isWrite, uint64_t _addr, uint64_t& trig_cnt);*/
        virtual uint64_t assign() override; // Return corresponding addr
        // bool checkSend(Data _data, Channel* upstream) override;
        void ackCallback(MemReq _req);     // Callback func for MemSys
        void setInflight(MemReq &_req);    // Set req inflight to signify this req has been sent to memSys
        pair<bool, MemReq> peekReqQueue(); // Peek the suspendReq by arbitor in memSys
        void statusUpdate() override;
        vector<int> pop() override; // Pop memory request from reqQueue when the data has sent to the downstream channel

    protected:
        void initial();
        // bool checkUpstream(uint64_t bufferId) override;
        // bool push(bool _isWrite, uint64_t _addr);  // Push memory access request into reqQueue
        // void push(bool _isWrite, uint64_t _addr, bool& trigger);  // Trigger mode
        // void pushReqQ(bool _isWrite, uint64_t _addr);
        void pushReqQ();            // chanBuffer[0] must store addr!!!
        // vector<int> pop() override; // Pop memory request from reqQueue when the data has sent to the downstream channel
        // void statusUpdate() override;
        void pushChannel();
        // bool sendReq(MemReq _req);  // Call MemSystem's addTransaction func
        void popChanBuffer();
        // void bpUpdate() override;
        virtual void funcUpdate() override;

        void updateMemAccessRecord(uint64_t _reqIndex);

#ifdef DEBUG_MODE // Get private instance for debug
    public:
        const uint64_t &getCurrReqId() const;
#endif // DEBUG_MODE

    public:
        // Store memory request; As for load back data, push the corresponding reqQueueIndex into the deque<Data> channel;
        // MemReq: keep req; Data: keep status;
        vector<pair<MemReq, Data>> reqQueue;
        vector<pair<bool, MemReq>> suspendReqVec; // Outstanding req, need send to memSys; <valid, req>; The vector size equal to the LSE speedup
        bool noLatencyMode = 0;                   // Emulate no latency memory access
        vector<int> *memorySpace = nullptr;       // Point to load/store array
        uint64_t baseAddr = 0;

        // Profiling
        uint64_t memAccessLat = 0;
        uint64_t avgMemAccessLat = 0;
        uint64_t memAccessCnt = 0;
        uint64_t latTotal = 0;

        uint64_t memReqCnt = 0;      // Send req to memSystem reqQueue
        uint64_t memReqBlockCnt = 0; // Send unsuccessfully due memSystem reqQueue is full

    protected:
        uint64_t lseId;
        uint64_t lastPopVal = 0;
        // reqQueue ptr
        uint64_t pushQueuePtr = 0; // Push req to reqQueue
        uint64_t sendChanPtr = 0;  // Send ready req to channel
        uint64_t sendMemPtr = 0;   // Send req to memory system

        bool isWrite;
        uint64_t orderId = 0;   // Record request sequence
        uint64_t currReqId = 0; // Used in Lse in order
        MemSystem *memSys = nullptr;

    public:
        deque<int> valueQueue; // Store value in store mode  (Temporally set to public for GCN trace injection)
    };
}