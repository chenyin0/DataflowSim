#pragma once

// Architecture para
//1) DGSF: channel size = BRAM size; Speedup = unroll number;
//2) SGMF: channel size = 2;
//3) TIA(instruction-based): much less PEs; fine-grained synchronization(the channel execute cycle is longer)

//#define DGSF
#define SGMF
//#define TIA

// Array para
#define MAX 2^31-1
#define FREQ 500 // system freq 500MHz
#define BANK_DEPTH 32  // SPM bank depth