#pragma once
#include "./Define.hpp"


/*
*****************************************************
*************    Simulation parameters  ***************
*/
#define DEBUG_MODE DEBUG_MODE


/*
*****************************************************
*************    Execution parameters  ***************
*/

// Architecture para
//1) DGSF: channel size = BRAM size; Speedup = unroll number;
//2) SGMF: channel size = 2;
//3) TIA(instruction-based): much less PEs; fine-grained synchronization(the channel execute cycle is longer)

enum class ArchType
{
    Base,  // Plasticine 64 pe
    DGSF,  // 48 pe
    SGMF,  // 32 pe
    TIA    // 12 pe
};

//#define Base Base
#define DGSF DGSF
//#define SGMF SGMF
//#define TIA TIA

#ifdef DGSF
    #define ARCH DGSF
#endif
#ifdef SGMF
    #define ARCH SGMF 
#endif
#ifdef TIA
    #define ARCH TIA
#endif
#ifdef Base
    #define ARCH Base
#endif

// Array para
#define MAX (std::numeric_limits<int>::max)()
#define FREQ 500 // System freq 500MHz

// Base
#define BASE_INPUT_BUFF_SIZE 1

// DGSF
#define BRAM_BANK_DEPTH 32  // SPM bank depth
#define DGSF_INPUT_BUFF_SIZE BRAM_BANK_DEPTH
#define BRAM_ACCESS_DELAY 3  // Access BRAM delay

// SGMF
#define SGMF_INPUT_BUFF_SIZE 16  // chanBuffer size of PE; Note: INPUT_BUFF_SIZE should be integer multiplies of TAG_SIZE
#define TAG_SIZE SGMF_INPUT_BUFF_SIZE  // Channel data tag size

// Lse
#define LSE_QUEUE_SIZE 16
#define LSE_O3 0

#ifdef DGSF
    #define CHAN_SIZE BRAM_BANK_DEPTH
#endif
#ifdef SGMF
    #define CHAN_SIZE SGMF_INPUT_BUFF_SIZE
#endif


/*
*****************************************************
*************    ALU parameters  ***************
*/

// ALU delay cycle
#define ADD 1
#define MUL 4

/*
************************************************
*************    Memory parameters  *************
*/

// MemSystem
#define NO_MEMORY 0  // Emulate ideal memory(latency = 0)

#define DATA_PRECISION 32  // Data precision is 32bits
#define MEMSYS_REQ_QUEUE_SIZE 32  // reqQueue size in MemSystem
#define BUS_DELAY 15  // Bus delay (between DRAM and Cache/SPM)

// SPM
#define SPM_ENABLE 0
#define SPM_ACCESS_LATENCY 2
#define SPM_BANK_NUM 16
#define SPM_BANK_DEPTH 64
#define SPM_REQ_QUEUE_SIZE 16  // reqQueue size in SPM

// Cache
#define CACHE_ENABLE 1
//#define CACHE_O3 0  // O3 = 1: Cache OoO; O3 = 0: Cache in order;
#define REQ_QUEUE_TO_MEM_SIZE 32 

#define CACHE_MAXLEVEL 2  // Max cache heriarachy level 

#define CACHE_SIZE_L1 16*1024  // byte
#define CACHE_SIZE_L2 256*1024  // byte

#define CACHE_LINE_SIZE_L1 32  // byte
#define CACHE_LINE_SIZE_L2 32  // byte

#define CACHE_MAPPING_WAY_L1 4
#define CACHE_MAPPING_WAY_L2 4

#define CACHE_ACCESS_LATENCY_L1 1  // Cycle
#define CACHE_ACCESS_LATENCY_L2 4  // Cycle

#define CACHE_BANK_NUM_L1 4
#define CACHE_BANK_NUM_L2 8

#define CACHE_REQ_Q_SIZE_PER_BANK_L1 8
#define CACHE_REQ_Q_SIZE_PER_BANK_L2 8