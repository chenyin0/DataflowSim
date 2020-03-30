#pragma once
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"

namespace DFSim
{
	class MemSystem
	{
	public:
		MemSystem();
		~MemSystem();

	private:
		MultiChannelMemorySystem* mem;  // DRAM
	};


	class DramInterface
	{
	public:
		void read_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
		void write_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
	};
}