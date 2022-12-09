#include "./Arbiter.h"

using namespace DFSim;

Arbiter::Arbiter(uint64_t _inPortNum, uint64_t _outPortNum) :
    inPortNum(_inPortNum), outPortNum(_outPortNum)
{
}

vector<uint64_t> Arbiter::trigger(vector<uint64_t> _activePort, vector<uint64_t> _outPort)
{
    vector<uint64_t> validOutPort = outPortSelect(_activePort, _outPort);
    ptrBaseUpdate(validOutPort);

    return validOutPort;
}

// Aribier_RR
Arbiter_RR::Arbiter_RR(uint64_t _inPortNum, uint64_t _outPortNum) :
    Arbiter(_inPortNum, _outPortNum)
{
}

vector<uint64_t> Arbiter_RR::outPortSelect(vector<uint64_t> _activePort, vector<uint64_t> _outPort)
{
    vector<uint64_t> output;
    for (size_t i = 0; i < inPortNum; ++i)
    {
        uint64_t ptr = ptrBase + i;
       
    }

    return output;
}

void Arbiter_RR::ptrBaseUpdate(vector<uint64_t> _validOutPort)
{
    ptrBase = _validOutPort.back();
}

// Arbiter_wp
Arbiter_wp::Arbiter_wp(uint64_t _inPortNum, uint64_t _outPortNum) :
    Arbiter(_inPortNum, _outPortNum)
{
}