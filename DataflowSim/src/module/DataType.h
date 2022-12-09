#pragma once
#include "../define/Define.hpp"
#include "./EnumType.h"

namespace DFSim
{
    struct Data
    {
        Data() {}

        void reset()
        {
            value = 0;
            valid = 0;
            cond = 0;
            last = 0;
            cycle = 0;
        }

        int value = 0;
        bool valid = 0;
        bool cond = 1;
        bool last = 0;  // loop last
        bool lastOuter = 0; // outer loop last flag
        uint64_t cycle = 0;

        // DGSF
        bool graphSwitch = 0;

        // SGMF
        uint64_t tag = 0;
        //uint64_t chanBundleAddr = 0;  // In multi-cycle channel, record the actual addr of the data in chanBundles

        // Lse
        uint64_t lseReqQueueIndex = 0;
    };


    struct MemReq
    {
        MemReq() = default;

        bool valid = 0;
        uint64_t addr = 0;
        //uint64_t value = 0;
        uint64_t cnt = 0;  // Record memReq sequence

        bool isWrite = 0;  // Ld or St req
        bool inflight = 0;  // Signify the req has sent out but hasn't been responsed
        bool ready = 0;  // Signify the data has been load already
        bool hasPushChan = 0;

        uint64_t lseId = 0;  // Signify belongs to which Lse
        uint64_t lseReqQueueIndex = 0;  // Signify the req store which entry of the reqQueue in Lse
        //uint64_t memSysAckQueueBankId = 0;  // Signify the req recorded in which bank of the ackQueue in memSys
        //uint64_t memSysAckQueueBankEntryId = 0;  // Signify the req recorded in with entry of the ackQueue bank
        uint64_t spmReqQueueIndex = 0;  // Signify the req stored in which entry of the reqQueue in SPM

        bool coalesced = 0;  // Record this req is a coalesced request

        // SGMF
        uint64_t tag = 0;
    };

    struct CacheReq
    {
        CacheReq() = default;

        bool valid = 0;
        uint64_t addr = 0;
        uint64_t cnt = 0;  // Record cacheReq sequence (for debug)
        //uint64_t setBase = 0;  // Cacheline Id (equal indicate in the same cacheline)
        //int hitIndex = -1;  // Cache miss = -1; Cache hit = cacheline index
        uint64_t lseId = 0;
        uint64_t lseReqQueueIndex = 0;
        
        bool inflight = 0;
        bool ready = 0;

        Cache_operation cacheOp;

        //uint64_t memSysAckQueueBankId = 0;  // Signify the req store which entry of the reqQueue in memSys
        //uint64_t memSysAckQueueBankEntryId = 0;  // Signify the req recorded in with entry of the ackQueue bank

        bool coalesced = 0;  // Record this req is a coalesced request
    };
}