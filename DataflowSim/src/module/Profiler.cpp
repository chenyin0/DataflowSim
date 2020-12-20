#include "./Profiler.h"
#include "../sim/Debug.h"

using namespace DFSim;

Profiler::Profiler(Registry* _registry)
{
    profilingTable.resize(_registry->registryTable.size());  // Indexed by moduleId
}

void Profiler::recordComputingCycle(uint _moduleId, ChanType _chanType, uint _clk)
{

}