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
        bool sendReq(MemReq _req);  // Call MemSystem's addTransaction func
        void popChanBuffer();
        //void bpUpdate() override;

    public:
        // Store memory request; As for load back data, push the corresponding reqQueueIndex into the deque<Data> channel;
        // MemReq: keep req; Data: keep status;
        vector<pair<MemReq, Data>> reqQueue;  

    protected:
        uint lseId;
        uint lastPopVal = 0;
        uint sendPtr = 0;
        bool isWrite;
        uint orderId = 0;  // Record request sequence
        uint currReqId = 0;  // Used in Lse in order
        MemSystem* memSys = nullptr;
    };
}