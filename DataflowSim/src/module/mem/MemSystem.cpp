#include "./MemSystem.h"

using namespace DFSim;

MemSystem::MemSystem()
{
	mem = new MultiChannelMemorySystem("./DRAMSim2/ini/DDR3_micron_16M_8B_x8_sg15.ini", "./DRAMSim2/ini/DRAMSimini/system.ini", ".", "example_app", 16384);
}

MemSystem::~MemSystem()
{
	delete mem;
}