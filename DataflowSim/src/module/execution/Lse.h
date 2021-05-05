/*
Module Lse:

1. Trigger mode: 
    Applicate scenario: 
        Inner loop executes many times then trigger outer loop execute once
    //Usage:
    //    uint trig_cnt = 3;  // Define trig_cnt in the benchmark_config.cpp
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
        /*Lse(uint _size, uint _cycle, MemSystem* _memSys);
        Lse(uint _size, uint _cycle, MemSystem* _memSys, uint _speedup);*/
        Lse(uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys);
        Lse(uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys, uint _speedup);
        Lse(string _moduleName, uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys, uint _speedup);
        ~Lse();

        //void get(bool _isWrite, uint _addr);  // Load/store addr
        //void get(vector<int> data);  // Load/store addr
        // Trigger mode (e.g. inner loop executes many times then trigger outer loop execute once,
        // or upper pe updates local reg many times then sends the data to lse once)
        //void get(vector<int> data, uint& trig_cnt);
        /*void get(bool _isWrite, uint _addr, uint& trig_cnt);*/  
        uint assign();  // Return corresponding addr
        //bool checkSend(Data _data, Channel* upstream) override;
        void ackCallback(MemReq _req);  // Callback func for MemSys
        void setInflight(MemReq& _req);  // Set req inflight to signify this req has been sent to memSys
        pair<bool, MemReq> peekReqQueue();  // Peek the suspendReq by arbitor in memSys

    protected:
        void initial();
        //bool checkUpstream(uint bufferId) override;
        //bool push(bool _isWrite, uint _addr);  // Push memory access request into reqQueue
        //void push(bool _isWrite, uint _addr, bool& trigger);  // Trigger mode
        //void pushReqQ(bool _isWrite, uint _addr);
        void pushReqQ();  // chanBuffer[0] must store addr!!!
        vector<int> pop() override;  // Pop memory request from reqQueue when the data has sent to the downstream channel
        void statusUpdate() override;
        void pushChannel();
        //bool sendReq(MemReq _req);  // Call MemSystem's addTransaction func
        void popChanBuffer();
        //void bpUpdate() override;

        void updateMemAccessRecord(uint _reqIndex);

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const uint& getCurrReqId() const;
#endif // DEBUG_MODE

    public:
        // Store memory request; As for load back data, push the corresponding reqQueueIndex into the deque<Data> channel;
        // MemReq: keep req; Data: keep status;
        vector<pair<MemReq, Data>> reqQueue;
        pair<bool, MemReq> suspendReq;  // Outstanding req, need send to memSys; <valid, req>
        bool noLatencyMode = 0;  // Emulate no latency memory access

        // Profiling
        uint memAccessLat = 0;
        uint avgMemAccessLat = 0;
        uint memAccessCnt = 0;
        uint latTotal = 0;

        uint memReqCnt = 0;  // Send req to memSystem reqQueue
        uint memReqBlockCnt = 0;  // Send unsuccessfully due memSystem reqQueue is full

    protected:
        uint lseId;
        uint lastPopVal = 0;
        // reqQueue ptr
        uint pushQueuePtr = 0;  // Push req to reqQueue
        uint sendChanPtr = 0;  // Send ready req to channel
        uint sendMemPtr = 0;  // Send req to memory system
        
        bool isWrite;
        uint orderId = 0;  // Record request sequence
        uint currReqId = 0;  // Used in Lse in order
        MemSystem* memSys = nullptr;
    };
}