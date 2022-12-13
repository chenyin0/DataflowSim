#pragma once
/*
TODO:

1. ADD bank conflict

*/

#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "../../sim/global.h"
#include "../DataType.h"
//#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
#include "../../../../DRAMSim3/src/dramsim3.h"

namespace DFSim
{
    /*struct MemReq;*/
    class Spm
    {
    public:
        Spm();

        // Inferface func for MemSystem
        bool addTransaction(MemReq _req);  // Add transaction to SPM
        vector<MemReq> callBack();  // Get memory access results from SPM
        
        // Interface func with DRAM
        //void sendReq2Mem(DRAMSim::MultiChannelMemorySystem* mem);  // Send memory req to DRAM (DRAMSim2)
        void sendReq2Mem(dramsim3::MemorySystem* mem);  // Send memory req to DRAM (DRAMSim3)
        /*void mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
        void mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk);*/
        void mem_req_complete(MemReq _req);

        void spmUpdate(); 

    private:
        void reqQueueUpdate();

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<pair<MemReq, uint64_t>>& getReqQueue() const { return reqQueue; }
        const uint64_t& getMemAccessCnt() const { return memAccessCnt; }
#endif // DEBUG_MODE 

    private:
        uint64_t sendPtr = 0;
        vector<pair<MemReq, uint64_t>> reqQueue;  // pair<req, latency>
        uint64_t bankNum = Global::spm_bank_num;
        uint64_t bankDepth = Global::spm_bank_depth;
        uint64_t memAccessCnt = 0;
    };
}