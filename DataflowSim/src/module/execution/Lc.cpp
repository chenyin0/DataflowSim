#include "Lc.h"
#include "../Registry.h"

using namespace DFSim;

Lc::Lc(ChanBase* _loopVar, ChanBase* _getEnd, ChanBase* _sendEnd, Mux* _mux) 
    : loopVar(_loopVar), getEnd(_getEnd), sendEnd(_sendEnd), mux(_mux)
{
    init();
}

Lc::~Lc()
{
    //delete getEnd;
    //delete sendEnd;
    //delete loopVar;
    //delete mux;
}

void Lc::init()
{
    moduleId = Registry::registerLc(this);

    getEnd->noDownstream = 1;
    getEnd->downstream = { nullptr };

    sendEnd->noUpstream = 1;
    sendEnd->upstream = { nullptr };

    getEnd->enable = 1;
    sendEnd->enable = 1;
    loopVar->enable = 1;

    //loopVar->isCond = 1;
    loopVar->isLoopVar = 1;
    loopVar->isFeedback = 1;
}

void Lc::addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd)
{
    loopVar->addUpstream(_getActive);
    loopVar->addDownstream(_sendActive);
    getEnd->addUpstream(_getEnd);
    sendEnd->addDownstream(_sendEnd);

    // Add getActive to LC Mux's trueChan and falseChan
    //mux->trueChan->addUpstream(_getActive);
    //mux->falseChan->addUpstream(_getActive);
}

void Lc::addDependence(vector<Channel*> _initDepend, vector<Channel*> _updateDepend)
{
    mux->falseChan->addUpstream(_initDepend);
    mux->trueChan->addUpstream(_updateDepend);

    // Update trueChan/falseChan upstream status
    if (mux->trueChan->upstream.empty())
    {
        mux->trueChan->noUpstream = 1;
    }
    else
    {
        mux->trueChan->noUpstream = 0;
    }

    if (mux->falseChan->upstream.empty())
    {
        mux->falseChan->noUpstream = 1;
    }
    else
    {
        mux->falseChan->noUpstream = 0;
    }
}

void Lc::getEndUpdate()
{
    getEnd->get({ 1 });

    //if (!getEnd->channel.empty() && getEnd->channel.back().lastOuter)
    //{
    //    getLastOuter.push_back(1);
    //}

    if (!getEnd->channel.empty())
    {
        ++loopEnd;
    }
}

void Lc::sendEndUpdate()
{
    if (sendEnd->valid && sendEnd->enable)
    {
        sendEnd->pop();
        sendEnd->statusUpdate();
    }

    if (!loopNumQ.empty() && loopEnd == loopNumQ.front())
    {
        sendEnd->get({ 1 });
        loopNumQ.pop_front();
        loopEnd = 0;

        //if (!getLastOuter.empty())
        //{
        //    getLastOuter.pop_front();
        //}
    }
}

void Lc::loopUpdate()
{
    // cond valid, signify cond will sends out a data
    if (loopVar->valid)
    {
        loopNum++;
        if (sel == 0)
        {
            loopNumQ.push_back(loopNum);
            loopNum = 0;
        }
    }

    // Add last tag
    if (!loopVar->channel.empty() && sel == 0)
    {
        loopVar->channel.front().last = 1;  // If loop 3 times, there will be i = 0, 1, 2(last = 1!)
        //loopVar->produceLast.push_back(1);
    }

    // Only when a loopVar pops out a data with last tag, send lastTag to each upstream channel in keepMode
    if (loopVar->valid && loopVar->channel.front().last)
    {
        //loopVar->produceLast.push_back(1);
        loopVar->sendLastTag();
    }

    //if (!loopVar->getLast.empty())
    //{
    //    loopVar->channel.back().lastOuter = 1;  // The last gotten by LC->loopVar is only from the outer loop
    //    loopVar->getLast.pop_front();
    //}
}

void Lc::selUpdate(bool newSel)
{
    //if (this->mux->muxSuccess)
    //{
    //    sel = newSel;
    //}

    // 1) MuxSuccess is true: a new data will be pushed into loopVar;
    // 2) loopVar->valid && loopVar->channel.size()>=2: loopVar is going to pop out the front data, and there is still another data in the channel;
    if (this->mux->muxSuccess || !loopVar->channel.empty())
    {
        sel = newSel;
    }
}

void Lc::lcUpdate(bool newSel)
{
    selUpdate(newSel);
    loopUpdate();  //Update cond channel
    getEndUpdate();  //Update getEnd
    sendEndUpdate();  //Update sendEnd
}


//// class LcDGSF
//LcDGSF::LcDGSF(ChanDGSF* _loopVar, ChanDGSF* _getEnd, ChanDGSF* _sendEnd, Mux* _mux, uint _graphSize) : 
//    Lc(_loopVar, _getEnd, _sendEnd, _mux), loopVar(_loopVar), graphSize(_graphSize)
//{
//    //init();
//    loopVar->isLoopVar = 1;
//    loopVar->isFeedback = 1;
//}
//
//LcDGSF::~LcDGSF()
//{
//    //delete loopVar;
//}
//
//void LcDGSF::loopUpdate()
//{
//    // cond valid, signify cond will sends out a data
//    if (loopVar->valid)
//    {
//        loopNum++;
//        if (sel == 0)
//        {
//            loopNumQ.push_back(loopNum);
//            loopNum = 0;
//        }
//    }
//
//    // add last tag
//    if (!loopVar->channel.empty() && sel == 0)
//    {
//        loopVar->channel.front().last = 1;  // If loop 3 times, there will be i = 0, 1, 2(last = 1!)
//    }
//
//    // Only when a loopVar pops out a data with last tag, send lastTag to each upstream channel in keepMode 
//    if (loopVar->valid && loopVar->channel.front().last)
//    {
//        //loopVar->produceLast.push_back(1);
//        loopVar->sendLastTag();
//    }
//
//    // When 1)loopNum = graphSize, or 2)current loop is over, set graphSwitch to 1
//    if (loopNum == graphSize || (!loopVar->channel.empty() && loopVar->channel.front().last == 1))
//    {
//        loopVar->channel.front().graphSwitch = 1;
//    }
//
//    //if (!loopVar->getLast.empty())
//    //{
//    //    loopVar->channel.back().lastOuter = 1;  // When current LC get a last from outer loop
//    //    loopVar->getLast.pop_front();
//    //}
//}


// class LcSGMF
LcSGMF::LcSGMF(ChanSGMF* _loopVar, ChanBase* _getEnd, ChanBase* _sendEnd, MuxSGMF* _mux) :
    Lc(_loopVar, _getEnd, _sendEnd, _mux), loopVar(_loopVar), mux(_mux)
{
    _loopVar->tagUpdateMode = 1;  // Set loopVar in tagUpdateMode

    loopVar->isLoopVar = 1;
    loopVar->isFeedback = 1;

    mux->isLcMux = 1;  // Set the mux to isLcMux;

    //// In Lc of SGMF, add outChan as upstream for the downstream(loopVar) automatically
    //for (auto& chan : _downstream)
    //{
    //    chan->addUpstream({ outChan }); 
    //}
}

LcSGMF::~LcSGMF()
{
    //delete loopVar;
    //delete mux;
}

//void LcSGMF::loopUpdate()
//{
//    // cond valid, signify cond will sends out a data
//    if (loopVar->valid)
//    {
//        loopNum++;
//        if (sel == 0)
//        {
//            loopNumQ.push_back(loopNum);
//            loopNum = 0;
//        }
//    }
//
//    // Add last tag
//    if (!loopVar->channel.empty() && sel == 0)
//    {
//        loopVar->channel.front().last = 1;  // If loop 3 times, there will be i = 0, 1, 2(last = 1!)
//    }
//
//    if (!loopVar->getLast.empty())
//    {
//        loopVar->channel.back().lastOuter = 1;  // The last gotten by LC->loopVar is only from the outer loop
//        loopVar->getLast.pop_front();
//    }
//}