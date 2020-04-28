#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
	// Cache replacement algorithm
	enum class Cache_swap_style
	{
		CACHE_SWAP_FIFO,
		CACHE_SWAP_LRU,
		CACHE_SWAP_RAND,
		CACHE_SWAP_MAX
	};

	enum class Cache_operation
	{
		READ,
		WRITE,
		LOCK,
		UNLOCK
	};
}
