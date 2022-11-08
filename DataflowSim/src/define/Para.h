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

#define Base Base
//#define DGSF DGSF  // Deprecated! Merge into Base
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
#define FREQ 1000000000 // System freq 1GHz
#define ARRAY_SIZE 48
#define TIA_ARRAY_SIZE 20

// Base
#define BASE_INPUT_BUFF_SIZE 4

// DGSF
#define BRAM_BANK_DEPTH 128  // SPM bank depth  128
#define DGSF_INPUT_BUFF_SIZE BRAM_BANK_DEPTH
#define BRAM_ACCESS_DELAY 2  // Access BRAM delay, emulate subgraph switch overhead

// SGMF
#define SGMF_INPUT_BUFF_SIZE 32  // chanBuffer size of PE; Note: INPUT_BUFF_SIZE should be integer multiplies of TAG_SIZE
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
#define SUB 1
#define MUL 4
#define MAC 4
#define DIV 20
#define RELU 1

/*
************************************************
*************    Memory parameters  *************
*/

// MemSystem
#define NO_MEMORY 0  // Emulate ideal memory(latency = 0)

#define DATA_PRECISION 45856  // Data precision is 32bits
#define BUS_DELAY 20  // Bus delay (between DRAM and Cache/SPM)
#define BANK_BLOCK_SIZE 64*8  // Block size of bank (bit)

#define MEMSYS_QUEUE_BANK_NUM 32  //32  // Number of MemSystem reqQueue bank, emulate bandwidth contention
#define MEMSYS_REQ_QUEUE_SIZE_PER_BANK 4 // reqQueue size per bank (default size = 1)
#define MEMSYS_ACK_QUEUE_SIZE_PER_BANK 4 // ackQueue size per bank (equal to L1$/SPM reqQueue size)

#define MEMSYS_COALESCING_ENABLE 1
#define MEMSYS_COALESCER_ENTRY_NUM 32  // 32
#define MEMSYS_COALESCER_SIZY_PER_ENTRY 16 //(BANK_BLOCK_SIZE/DATA_PRECISION)  // 8

// SPM
#define SPM_ENABLE 0
#define SPM_ACCESS_LATENCY 2
#define SPM_BANK_NUM 16
#define SPM_BANK_DEPTH 128
#define SPM_REQ_QUEUE_SIZE 16  // reqQueue size in SPM

// Cache
#define CACHE_ENABLE 1
#define CACHE_ALL_HIT 0
#define REQ_QUEUE_TO_MEM_SIZE 128 

#define CACHE_MAXLEVEL 1  // Max cache heriarachy level 

#define CACHE_SIZE_L1 16*1024*1024  // byte 16KB
#define CACHE_SIZE_L2 1*1024*1024  // byte 256KB

//#define CACHE_SIZE_L1 1*1024  // byte
//#define CACHE_SIZE_L2 16*1024  // byte

// SGMF paper parameter
//#define CACHE_SIZE_L1 64*1024  // byte
//#define CACHE_SIZE_L2 786*1024  // byte

#define CACHE_LINE_SIZE_L1 45856  // byte 32
#define CACHE_LINE_SIZE_L2 32  // byte

#define CACHE_MAPPING_WAY_L1 16  // 4
#define CACHE_MAPPING_WAY_L2 16

#define CACHE_ACCESS_LATENCY_L1 1  // Cycle
#define CACHE_ACCESS_LATENCY_L2 4  // Cycle

#define CACHE_BANK_NUM_L1 MEMSYS_QUEUE_BANK_NUM
#define CACHE_BANK_NUM_L2 8

#define CACHE_REQ_Q_SIZE_PER_BANK_L1 16  // 4
#define CACHE_REQ_Q_SIZE_PER_BANK_L2 4  // 4

#define CACHE_ACK_Q_SIZE_PER_BANK_L1 16
#define CACHE_ACK_Q_SIZE_PER_BANK_L2 4

//#define CACHE_COALESCE_TABLE_ENTRY_NUM CACHE_BANK_NUM_L1
//#define CACHE_COALESCE_TABLE_SIZE_PER_ENTRY 4

//#define CACHE_BANK_FIFO_COALESCE_ENABLE_L1 1
//#define CACHE_BANK_FIFO_COALESCE_ENABLE_L2 1

#define CACHE_MSHR_ENABLE_L1 1
#define CACHE_MSHR_ENABLE_L2 1

//** MSHR entry size, reference value is 32 in a typical GPU
#define CACHE_MSHR_ENTRY_NUM_L1 1024 // CACHE_BANK_NUM_L1
#define CACHE_MSHR_ENTRY_NUM_L2 CACHE_BANK_NUM_L2

//** MSHR slot size, reference value is 4 or 8 in a typical GPU or CPU
#define CACHE_MSHR_SIZE_PER_ENTRY_L1 8
#define CACHE_MSHR_SIZE_PER_ENTRY_L2 4