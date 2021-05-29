/*
Module Lc

1. loop controller

*/

/*
Usage:
1. Declare:
    Lc* lc0 = &Lc();
    lc0->addPort({getAct}, {sendAct}, {getEnd}, {sendEnd});

2. Code insert:
    Mux mux0 = new Mux({lc->var}, {empty or other channel*}, {lc->var});

    int i = init val;
    int i = mux0->mux(i, init val, lc->sel);
    mux0->muxUpdate(lc->sel);
    lc->loopVar->get(i);
    i = lc->loopVar.assign() -> update; // e.g. i = lc->loopVar.assign() + 1 (to implement i++);
    lc->sel = i < boundary;  // => if(i < boundary)
    lc->lcUpdate();
*/

#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"
#include "./Channel.h"
#include "./Mux.h"

namespace DFSim
{
    class Lc
    {
    public:
        //LC(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);
        Lc(ChanBase* _loopVar, ChanBase* _getEnd, ChanBase* _sendEnd, Mux* _mux);
        Lc(const string& moduleName_);
        virtual ~Lc();
        void lcUpdate(bool newSel);
        void selUpdate(bool newSel);
        void addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);
        // initDepend: external initial loop var; updateDepend: for variable loop boundary and variable loop updating
        void addDependence(vector<Channel*> _initDepend, vector<Channel*> _updateDepend);  
    protected:
        void init();
        void getEndUpdate();
        void sendEndUpdate();
        virtual void loopUpdate();

    public:
        string moduleName;
        string masterName;  // If this module affiliates to a upper module, store the name of it, or else store "None";
        ModuleType moduleTypr = ModuleType::Lc;
        uint moduleId;
        bool sel = 0;  // Sel for loop control, default set to select initial value
        uint loopNum = 0; // loop number
        uint loopEnd = 0; // the number of finished loop
        uint currLoopId = 0; // If current loop Id == loop number, loop end
        deque<uint> loopNumQ; // Store each loop number
        bool isOuterMostLoop = 0;  // Signify this is the outer-most loop, used in assigning "lastOuter" to the outer-most loop 
        //deque<bool> getLastOuter;  // LC->cond gets a last from outer loop
        uint subgraphId = 0;

    public:
        ChanBase* getEnd = nullptr;
        ChanBase* sendEnd = nullptr;
        ChanBase* loopVar = nullptr;  // 1) Get condition result; 2) Used as getActive and sendActive;
        //MuxLc* mux = new MuxLc({ loopVar }, { }, { loopVar });
        Mux* mux = nullptr;
        int var = 0;  // loop variable
    };


/*
LcDGSF usage:

1. set graphSize after declaring; (graphSize is equal to BRAM bank depth) 

*/
    //// LC for DGSF
    //class LcDGSF : public Lc
    //{
    //public:
    //    LcDGSF(ChanDGSF* _loopVar, ChanDGSF* _getEnd, ChanDGSF* _sendEnd, Mux* _mux, uint _graphSize);
    //    ~LcDGSF();
    //    void loopUpdate() override;
    //    uint graphSize;

    //public:
    //    ChanDGSF* loopVar;  // 1) Get condition result; 2) Used as getActive and sendActive;
    //};


/*
LcSGMF usage:

1. Set loopVar in tagUpdate mode

*/
// LC for SGMF
    class LcSGMF : public Lc
    {
    public:
        LcSGMF(ChanSGMF* _loopVar, ChanBase* _getEnd, ChanBase* _sendEnd, MuxSGMF* _mux);
        ~LcSGMF();
        //void loopUpdate() override;

    public:
        ChanSGMF* loopVar;
        //MuxSGMFLc* mux = new MuxSGMFLc({ loopVar }, { }, { loopVar });
        MuxSGMF* mux;
    };

}

