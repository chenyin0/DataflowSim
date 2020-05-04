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
		UNLOCK,
		WRITEBACK_DIRTY_BLOCK
	};

	enum class Cache_write_strategy
	{
		WRITE_BACK,
		WRITE_THROUGH
	};

	enum class Cache_write_allocate
	{
		WRITE_ALLOCATE,
		WRITE_NON_ALLOCATE
	};
}
