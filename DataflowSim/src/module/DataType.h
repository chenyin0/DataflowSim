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
        uint cycle = 0;

        // DGSF
        bool graphSwitch = 0;

        // SGMF
        uint tag = 0;
        //uint chanBundleAddr = 0;  // In multi-cycle channel, record the actual addr of the data in chanBundles

        // Lse
        uint lseReqQueueIndex = 0;
    };


    struct MemReq
    {
        MemReq() = default;

        bool valid = 0;
        uint addr = 0;
        //uint value = 0;
        uint cnt = 0;  // Record memReq sequence

        bool isWrite = 0;  // Ld or St req
        bool inflight = 0;  // Signify the req has sent out but hasn't been responsed
        bool ready = 0;  // Signify the data has been load already
        bool hasPushChan = 0;

        uint lseId = 0;  // Signify belongs to which Lse
        uint lseReqQueueIndex = 0;  // Signify the req store which entry of the reqQueue in Lse
        uint memSysAckQueueBankId = 0;  // Signify the req recorded in which bank of the ackQueue in memSys
        uint memSysAckQueueBankEntryId = 0;  // Signify the req recorded in with entry of the ackQueue bank
        uint spmReqQueueIndex = 0;  // Signify the req store which entry of the reqQueue in SPM

        // SGMF
        uint tag = 0;
    };

    struct CacheReq
    {
        CacheReq() = default;

        bool valid = 0;
        uint addr = 0;
        uint cnt = 0;  // Record cacheReq sequence (for debug)
        //uint setBase = 0;  // Cacheline Id (equal indicate in the same cacheline)
        //int hitIndex = -1;  // Cache miss = -1; Cache hit = cacheline index
        
        bool inflight = 0;
        bool ready = 0;

        Cache_operation cacheOp;

        uint memSysAckQueueBankId = 0;  // Signify the req store which entry of the reqQueue in memSys
        uint memSysAckQueueBankEntryId = 0;  // Signify the req recorded in with entry of the ackQueue bank
    };
}