#pragma once

// Architecture para
//1) DGSF: channel size = BRAM size; Speedup = unroll number;
//2) SGMF: channel size = 2;
//3) TIA(instruction-based): much less PEs; fine-grained synchronization(the channel execute cycle is longer)

#define DGSF DGSF
//#define SGMF SGMF
//#define TIA TIA
//#define Base Base

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
#define MAX 2^31-1
#define FREQ 500 // system freq 500MHz
#define BRAM_BANK_DEPTH 32  // SPM bank depth
#define INPUT_BUFF_SIZE 16  // input buffer size of PE

#ifdef DGSF
	#define CHAN_SIZE BRAM_BANK_DEPTH
#endif
#ifdef SGMF
	#define CHAN_SIZE INPUT_BUFF_SIZE
#endif