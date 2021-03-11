#pragma once
#include "../../define/Define.hpp"
/*
Module Arbiter:

1. Select m from n by trigger function, in which m is outPortNum and n is inPortNum
3. activePort resent the valid port in this round
2. return a vector<uint>, represent the selected port Id

*/

namespace DFSim
{
    class Arbiter
    {
    public:
        Arbiter(uint _inPortNum, uint _outPortNum);
        vector<uint> trigger(vector<uint> _activePort, vector<uint> _outPort);  // outPort may not the eventual outPort, due to port contention 

    protected:
        virtual vector<uint> outPortSelect(vector<uint> _activePort, vector<uint> _outPort) = 0;
        virtual void ptrBaseUpdate(vector<uint> _validOutPort) = 0;

        uint inPortNum;
        uint outPortNum;
        uint ptrBase = 0;
    };

    // Round-robin, update ptrBase to the last valid port
    class Arbiter_RR : public Arbiter
    {
    public:
        Arbiter_RR(uint _inPortNum, uint _outPortNum);
        virtual vector<uint> outPortSelect(vector<uint> _activePort, vector<uint> _outPort) override;
        virtual void ptrBaseUpdate(vector<uint> _validOutPort) override;
    };

    // Wait-priority, update ptrBase to the last valid port
    class Arbiter_wp : public Arbiter
    {
    public:
        Arbiter_wp(uint _inPortNum, uint _outPortNum);
        //virtual vector<uint> outPortSelect(vector<uint> _activePort, vector<uint> _outPort) override;
        //virtual void ptrBaseUpdate(vector<uint> _validOutPort) override;
    };


}