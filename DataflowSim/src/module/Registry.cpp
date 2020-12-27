#include "./Registry.h"
#include "../sim/Debug.h"

using namespace DFSim;

uint Registry::moduleId = 0;
vector<RegistryTable> Registry::registryTable;

Registry::Registry()
{
}

Registry::~Registry()
{
    for (auto& entry : registryTable)
    {
        if (entry.chanPtr != nullptr)
        {
            delete entry.chanPtr;
        }
        if (entry.lcPtr != nullptr)
        {
            delete entry.lcPtr;
        }
        if (entry.muxPtr != nullptr)
        {
            delete entry.muxPtr;
        }
    }
}

int Registry::registerChan(Channel* chan)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }
    else
    {
        RegistryTable entry;
        entry.chanPtr = chan;
        entry.moduleId = Registry::moduleId;
        entry.moduleType = ModuleType::Channel;
        registryTable.push_back(entry);
        //Registry::moduleId++;

        return Registry::moduleId++;
    }
}

int Registry::registerLc(Lc* lc)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }
    else
    {
        RegistryTable entry;
        entry.lcPtr = lc;
        entry.moduleId = Registry::moduleId;
        entry.moduleType = ModuleType::Lc;
        registryTable.push_back(entry);
        //Registry::moduleId++;

        return Registry::moduleId++;
    }
}

int Registry::registerMux(Mux* mux)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }
    else
    {
        RegistryTable entry;
        entry.muxPtr = mux;
        entry.moduleId = Registry::moduleId;
        entry.moduleType = ModuleType::Mux;
        registryTable.push_back(entry);
        //Registry::moduleId++;

        return Registry::moduleId++;
    }
}

void Registry::tableInit()
{
    initChannel();
    checkConnectRule();
    checkLc();
}

void Registry::pathBalance()
{
    // Ensure each channel buffer size greater than 2
    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            Channel* chan = entry.chanPtr;
            chan->size = (chan->size >= 2) ? chan->size : 2;
        }
    }
}

void Registry::initLastTagQueue(Channel* _chan)
{
    //for (auto& entry : registryTable)
    //{
    //    if (entry.chanPtr != nullptr)
    //    {
    //        Channel* chan = entry.chanPtr;
    //        if (chan->keepMode)
    //        {
    //            chan->lastTagQueue.resize(chan->downstream.size());  // Resize a queue for each downstream channel
    //            for (size_t i = 0; i < chan->lastTagQueue.size(); ++i)
    //            {
    //                uint _moduleId = chan->downstream[i]->moduleId;
    //                deque<bool> queue;
    //                chan->lastTagQueue[i] = make_pair(_moduleId, queue);
    //            }
    //        }
    //    }
    //}

    if (_chan->keepMode)
    {
        _chan->lastTagQueue.resize(_chan->downstream.size());  // Resize a queue for each downstream channel
        for (size_t i = 0; i < _chan->lastTagQueue.size(); ++i)
        {
            uint _moduleId = _chan->downstream[i]->moduleId;
            deque<bool> queue;
            _chan->lastTagQueue[i] = make_pair(_moduleId, queue);
        }
    }
}

void Registry::initChanBuffer(Channel* _chan)
{
    uint upstreamSize = _chan->upstream.size();

    if (!_chan->noUpstream)
    {
        _chan->chanBuffer.resize(upstreamSize);
        _chan->chanBufferDataCnt.resize(upstreamSize);
        _chan->keepModeDataCnt.resize(upstreamSize);
        _chan->getTheLastData.resize(upstreamSize);
    }
    else
    {
        _chan->chanBuffer.resize(1);
        _chan->chanBufferDataCnt.resize(1);
        _chan->keepModeDataCnt.resize(1);
        _chan->getTheLastData.resize(1);
    }

    // If channel is SGMF, resize each buffer of chanBuffer
    if (_chan->chanType == ChanType::Chan_SGMF)
    {
        for (auto& buffer : _chan->chanBuffer)
        {
            buffer.resize(_chan->size);
        }
    }

    //// If channel is DGSF, resize deque getTheLastData
    //if (_chan->chanType == ChanType::Chan_DGSF)
    //{
    //    _chan->getTheLastData.resize(upstreamSize);
    //    for (auto& i : _chan->getTheLastData)
    //    {
    //        i = 0;
    //    }
    //}
}

void Registry::initBp(Channel* _chan)
{
    if (!_chan->noUpstream)
    {
        uint upstreamSize = _chan->upstream.size();
        _chan->bp.resize(upstreamSize);
    }
    else
    {
        _chan->bp.resize(1);
    }
}

void Registry::initLastPopVal(Channel* _chan)
{
    if (!_chan->noUpstream)
    {
        uint upstreamSize = _chan->upstream.size();
        _chan->lastPopVal.resize(upstreamSize);
    }
    else
    {
        _chan->lastPopVal.resize(1);
    }
}

void Registry::initChannel()
{
    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            initChanBuffer(entry.chanPtr);
            initLastTagQueue(entry.chanPtr);
            initBp(entry.chanPtr);
            initLastPopVal(entry.chanPtr);
        }
    }
}

void Registry::checkConnectRule()
{
    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            Channel* chan = entry.chanPtr;
            //if ((!chan->noUpstream && chan->upstream.empty()) || (!chan->noDownstream && chan->downstream.empty()))
            //{
            //    Debug::throwError("Upstream or Downstream of this channel is empty!", __FILE__, __LINE__);
            //}
            checkChanConnect(chan);
            checkChanMode(chan);
            checkChanPartialMux(chan);
        }
    }
}

void Registry::checkChanConnect(Channel* _chan)
{
    if ((!_chan->noUpstream && _chan->upstream.empty()) || (!_chan->noDownstream && _chan->downstream.empty()))
    {
        Debug::throwError("Upstream or Downstream of this channel is empty!", __FILE__, __LINE__);
    }
}

void Registry::checkChanMode(Channel* _chan)
{
    if (_chan->keepMode && _chan->drainMode)
    {
        Debug::throwError("A channel can't be in keepMode and drainMode in the same time!", __FILE__, __LINE__);
    }
}

void Registry::checkChanPartialMux(Channel* _chan)
{
    if (_chan->chanType == ChanType::Chan_PartialMux)
    {
        if (_chan->upstream[0]->isCond != 1)
        {
            Debug::throwError("The first upstream channel of this ChanParitialMux is not in isCond mode!", __FILE__, __LINE__);
        }
    }
}

void Registry::checkLc()
{
    uint outerMostLoopNum = 0;

    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Lc)
        {
            if (entry.lcPtr->isOuterMostLoop)
            {
                ++outerMostLoopNum;
            }
        }
    }

    if (outerMostLoopNum == 0)
    {
        std::cout << "Warning: Not configure any outer-most Lc, make sure there is no loop in this application!" << std::endl;
    }

    if (outerMostLoopNum > 1)
    {
        std::cout << "Warning: Configure more than one outer-most Lc, make sure there are more than one outer-most loop in this application!" << std::endl;
    }
}


