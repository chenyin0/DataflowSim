#pragma once
#include "./Define.hpp"

// Architecture para
//1) DGSF: channel size = BRAM size; Speedup = unroll number;
//2) SGMF: channel size = 2;
//3) TIA(instruction-based): much less PEs; fine-grained synchronization(the channel execute cycle is longer)

enum class ArchType
{
	Base,
	DGSF,
	SGMF,
	TIA
};

#define Base Base
//#define DGSF DGSF
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

// DGSF
#define BRAM_BANK_DEPTH 32  // SPM bank depth

// SGMF
#define INPUT_BUFF_SIZE 16  // Input buffer size of PE
#define CHANNEL_BUNDLE_SIZE 2  // PE chanBundle size (Din1, Din2...)

#ifdef DGSF
	#define CHAN_SIZE BRAM_BANK_DEPTH
#endif
#ifdef SGMF
	#define CHAN_SIZE INPUT_BUFF_SIZE
#endif