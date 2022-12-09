#pragma once
#include "../../define/Define.hpp"
/*
Module Arbiter:

1. Select m from n by trigger function, in which m is outPortNum and n is inPortNum
3. activePort resent the valid port in this round
2. return a vector<uint64_t>, represent the selected port Id

*/

namespace DFSim
{
    class Arbiter
    {
    public:
        Arbiter(uint64_t _inPortNum, uint64_t _outPortNum);
        vector<uint64_t> trigger(vector<uint64_t> _activePort, vector<uint64_t> _outPort);  // outPort may not the eventual outPort, due to port contention 

    protected:
        virtual vector<uint64_t> outPortSelect(vector<uint64_t> _activePort, vector<uint64_t> _outPort) = 0;
        virtual void ptrBaseUpdate(vector<uint64_t> _validOutPort) = 0;

        uint64_t inPortNum;
        uint64_t outPortNum;
        uint64_t ptrBase = 0;
    };

    // Round-robin, update ptrBase to the last valid port
    class Arbiter_RR : public Arbiter
    {
    public:
        Arbiter_RR(uint64_t _inPortNum, uint64_t _outPortNum);
        virtual vector<uint64_t> outPortSelect(vector<uint64_t> _activePort, vector<uint64_t> _outPort) override;
        virtual void ptrBaseUpdate(vector<uint64_t> _validOutPort) override;
    };

    // Wait-priority, update ptrBase to the last valid port
    class Arbiter_wp : public Arbiter
    {
    public:
        Arbiter_wp(uint64_t _inPortNum, uint64_t _outPortNum);
        //virtual vector<uint64_t> outPortSelect(vector<uint64_t> _activePort, vector<uint64_t> _outPort) override;
        //virtual void ptrBaseUpdate(vector<uint64_t> _validOutPort) override;
    };


}