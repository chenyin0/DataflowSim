#include "./Arbiter.h"

using namespace DFSim;

Arbiter::Arbiter(uint _inPortNum, uint _outPortNum) :
    inPortNum(_inPortNum), outPortNum(_outPortNum)
{
}

vector<uint> Arbiter::trigger(vector<uint> _activePort, vector<uint> _outPort)
{
    vector<uint> validOutPort = outPortSelect(_activePort, _outPort);
    ptrBaseUpdate(validOutPort);

    return validOutPort;
}

// Aribier_RR
Arbiter_RR::Arbiter_RR(uint _inPortNum, uint _outPortNum) :
    Arbiter(_inPortNum, _outPortNum)
{
}

vector<uint> Arbiter_RR::outPortSelect(vector<uint> _activePort, vector<uint> _outPort)
{
    vector<uint> output;
    for (size_t i = 0; i < inPortNum; ++i)
    {
        uint ptr = ptrBase + i;
       
    }

    return output;
}

void Arbiter_RR::ptrBaseUpdate(vector<uint> _validOutPort)
{
    ptrBase = _validOutPort.back();
}

// Arbiter_wp
Arbiter_wp::Arbiter_wp(uint _inPortNum, uint _outPortNum) :
    Arbiter(_inPortNum, _outPortNum)
{
}