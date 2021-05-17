#include "./Registry.h"
#include "../sim/Debug.h"

using namespace DFSim;

uint Registry::moduleId = 0;
vector<RegistryTable> Registry::registryTable;
unordered_map<string, uint> Registry::registryDict;

Registry::Registry(MemSystem* _memSys) : memSys(_memSys)
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

    //delete memSys;
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

int Registry::registerChan(const string& moduleName_, Channel* chan_)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }

    auto iter = registryDict.find(moduleName_);
    if (iter != registryDict.end())
    {
        Debug::throwError("There already has a same name module in registryDict!", __FILE__, __LINE__);
        return -1;
    }
    
    RegistryTable entry;
    entry.chanPtr = chan_;
    entry.moduleId = Registry::moduleId;
    entry.moduleType = ModuleType::Channel;
    entry.moduleName = moduleName_;
    registryDict.insert(pair<string, uint>(moduleName_, registryTable.size()));
    registryTable.push_back(entry);

    return Registry::moduleId++;
}

int Registry::registerLc(const string& moduleName_, Lc* lc_)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }

    auto iter = registryDict.find(moduleName_);
    if (iter != registryDict.end())
    {
        Debug::throwError("There already has a same name module in registryDict!", __FILE__, __LINE__);
        return -1;
    }

    else
    {
        RegistryTable entry;
        entry.lcPtr = lc_;
        entry.moduleId = Registry::moduleId;
        entry.moduleType = ModuleType::Lc;
        entry.moduleName = moduleName_;
        registryDict.insert(pair<string, uint>(moduleName_, registryTable.size()));
        registryTable.push_back(entry);

        return Registry::moduleId++;
    }
}

int Registry::registerMux(const string& moduleName_, Mux* mux_)
{
    if (Registry::registryTable.size() != Registry::moduleId)
    {
        Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
        return -1;
    }

    auto iter = registryDict.find(moduleName_);
    if (iter != registryDict.end())
    {
        Debug::throwError("There already has a same name module in registryDict!", __FILE__, __LINE__);
        return -1;
    }

    else
    {
        RegistryTable entry;
        entry.muxPtr = mux_;
        entry.moduleId = Registry::moduleId;
        entry.moduleType = ModuleType::Mux;
        entry.moduleName = moduleName_;
        registryDict.insert(pair<string, uint>(moduleName_, registryTable.size()));
        registryTable.push_back(entry);

        return Registry::moduleId++;
    }
}

Channel* Registry::getChan(const string& moduleName_)
{
    return registryTable[registryDict[moduleName_]].chanPtr;
}

Lc* Registry::getLc(const string& moduleName_)
{
    return registryTable[registryDict[moduleName_]].lcPtr;
}

Mux* Registry::getMux(const string& moduleName_)
{
    return registryTable[registryDict[moduleName_]].muxPtr;
}

void Registry::tableInit()
{
    initChannel();
    checkConnectRule();
    checkLc();
}

void Registry::pathBalance()
{
    // Ensure each channel buffer size greater than 2 to avoid deadlock and pipeline bubble
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

void Registry::initAluInput(Channel* _chan)
{
    if (!_chan->noUpstream)
    {
        uint upstreamSize = _chan->upstream.size();
        _chan->buffer2Alu.resize(upstreamSize);
    }
    else
    {
        _chan->buffer2Alu.resize(1);
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
            initAluInput(entry.chanPtr);
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
            checkChanDGSF(chan);
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

void Registry::checkChanDGSF(Channel* _chan)
{
    if (_chan->chanType == ChanType::Chan_DGSF)
    {
        if (!_chan->noUpstream && _chan->upstream.size() > 1)
        {
            Debug::throwError("The number of ChanDGSF's upstream can not exceed 1 !", __FILE__, __LINE__);
        }
    }
}

void Registry::genModule(ChanGraph& _chanGraph)
{
    auto _nodes = _chanGraph.nodes;
    for (auto& _module : _nodes)
    {
        auto controlRegion = _chanGraph.controlTree.getCtrlRegion(_module->controlRegionName);
        auto controlType = controlRegion.controlType;
        auto branchPath = controlRegion.branchPath;
        auto _node_type = dynamic_cast<Chan_Node*>(_module)->node_type;
        if (_node_type == "ChanBase" || _node_type == "ChanDGSF" || _node_type == "ChanPartialMux")
        {
            auto chanPtr = genChan(*dynamic_cast<Chan_Node*>(_module));
            if (controlType == "Branch")
            {
                chanPtr->branchMode = true;
                chanPtr->channelCond = branchPath;
            }
            // Set cond for Cmp operation
            if (dynamic_cast<Chan_Node*>(_module)->node_op == "Cmp")
            {
                chanPtr->isCond = true;
            }
        }
        else if (_node_type == "Lc")
        {
            auto& controlRegion = _chanGraph.controlTree.controlRegionTable[_chanGraph.controlTree.findControlRegionIndex(_module->controlRegionName)->second];
            if (controlRegion.upperControlRegion.empty())
            {
                genLcOuterMost(*dynamic_cast<Chan_Node*>(_module));
            }
            else
            {
                auto lcPtr = genLc(*dynamic_cast<Chan_Node*>(_module));
                // TODO: not sure whether lc in branchMode works well
                if (controlType == "Branch")
                {
                    lcPtr->loopVar->branchMode = true;
                    lcPtr->loopVar->channelCond = branchPath;
                }
            }
        }
        else if (_node_type == "Lse_ld" || _node_type == "Lse_st")
        {
            auto lsePtr = genLse(*dynamic_cast<Chan_Node*>(_module));
            if (controlType == "Branch")
            {
                lsePtr->branchMode = true;
                lsePtr->channelCond = branchPath;
            }
        }
        else if (_node_type == "Mux")
        {
            genMux(*dynamic_cast<Chan_Node*>(_module));
        }
    }
}

Lc* Registry::genLc(Chan_Node& _lc)
{
    Lc* lc = new Lc(_lc.node_name);
    lc->mux->addPort({ lc->loopVar }, { }, { lc->loopVar });
    return lc;
}

Lc* Registry::genLcOuterMost(Chan_Node& _lc)
{
    const auto& lc = genLc(_lc);
    lc->isOuterMostLoop = 1;
    return lc;
}

Channel* Registry::genChan(Chan_Node& _chan)
{
    Channel* chan_ = nullptr;
    if (_chan.node_name == "Chan_begin" || _chan.node_name == "Chan_end")
    {
        Channel* chan = new ChanBase(_chan.node_name, 1, 0, 1);
        chan_ = chan;
    }
    else if (_chan.node_type == "ChanBase")
    {
        ChanBase* chan = new ChanBase(_chan.node_name, _chan.size, _chan.cycle, _chan.speedup);

        if (_chan.chan_mode == "Keep_mode")
        {
            chan->keepMode = 1;
        }
        else if (_chan.chan_mode == "Drain_mode")
        {
            chan->drainMode = 1;
        }

        chan_ = chan;
    }
    else if (_chan.node_type == "ChanDGSF")
    {
        ChanDGSF* chan = new ChanDGSF(_chan.node_name, _chan.size, _chan.cycle, _chan.speedup);

        if (_chan.chan_mode == "Keep_mode")
        {
            chan->keepMode = 1;
        }
        else if (_chan.chan_mode == "Drain_mode")
        {
            chan->drainMode = 1;
        }

        chan_ = chan;
    }
    else if (_chan.node_type == "ChanPartialMux")
    {
        ChanPartialMux* chan = new ChanPartialMux(_chan.node_name, _chan.size, _chan.cycle, _chan.speedup);
        chan_ = chan;
    }

    return chan_;
}

Lse* Registry::genLse(Chan_Node& _lse)
{
    Lse* lse = nullptr;
    if (_lse.node_type == "Lse_ld")
    {
        lse = new Lse(_lse.node_name, _lse.size, _lse.cycle, false, memSys, _lse.speedup);
    }
    else if (_lse.node_type == "Lse_st")
    {
        lse = new Lse(_lse.node_name, _lse.size, _lse.cycle, true, memSys, _lse.speedup);
    }

    return lse;
}

Mux* Registry::genMux(Chan_Node& _mux)
{
    Mux* mux = new Mux(_mux.node_name);
    return mux;
}

void Registry::genConnect(ChanGraph& _chanGraph)
{
    for (auto& entry : registryTable)
    {
        // Lc
        if (entry.lcPtr != nullptr)
        {
            vector<string> getActiveName;
            vector<string> sendActiveName;
            string getEndName;
            //vector<string> sendEndName;  // auto-set in completeConnect

            auto lcChanGraphPtr = _chanGraph.getNode(entry.lcPtr->moduleName);
            getActiveName.insert(getActiveName.end(), lcChanGraphPtr->pre_nodes_data.begin(), lcChanGraphPtr->pre_nodes_data.end());
            getActiveName.insert(getActiveName.end(), lcChanGraphPtr->pre_nodes_active.begin(), lcChanGraphPtr->pre_nodes_active.end());

            sendActiveName.insert(sendActiveName.end(), lcChanGraphPtr->next_nodes_data.begin(), lcChanGraphPtr->next_nodes_data.end());
            sendActiveName.insert(sendActiveName.end(), lcChanGraphPtr->next_nodes_active.begin(), lcChanGraphPtr->next_nodes_active.end());

            getEndName = _chanGraph.controlTree.getCtrlRegion(lcChanGraphPtr->controlRegionName).theTailNode;
            if (getEndName == "")
            {
                Debug::throwError("Forget to declare the tailNode in loop control region !", __FILE__, __LINE__);
            }

            // Process begin and end node
            auto iter = find(getActiveName.begin(), getActiveName.end(), "Chan_begin");
            auto iter_ = find(sendActiveName.begin(), sendActiveName.end(), "Chan_end");
            if (iter != getActiveName.end())
            {
                Channel* Chan_begin_ptr = getChan("Chan_begin");
                entry.lcPtr->addDependence({ Chan_begin_ptr }, {});
                getActiveName.erase(iter);
            }
            if (iter_ != sendActiveName.end())
            {
                Channel* Chan_end_ptr = getChan("Chan_end");
                entry.lcPtr->addPort({ }, { }, { }, { Chan_end_ptr });
                sendActiveName.erase(iter_);
            }

            vector<Channel*> getActive;
            for (auto& i : getActiveName)
            {
                getActive.push_back(getChan(i));
            }

            vector<Channel*> sendActive;
            for (auto& i : sendActiveName)
            {
                sendActive.push_back(getChan(i));
            }

            Channel* getEnd = nullptr;
            if (getRegistryTableEntry(getEndName).lcPtr != nullptr)
            {
                getEnd = getRegistryTableEntry(getEndName).lcPtr->sendEnd;
            }
            else if (getRegistryTableEntry(getEndName).chanPtr != nullptr)
            {
                getEnd = getChan(getEndName);
            }
            else
            {
                Debug::throwError("Not find getEnd!", __FILE__, __LINE__);
            }

            //vector<Channel*> sendEnd;

            entry.lcPtr->addPort({ getActive }, { sendActive }, { getEnd }, {});  // No need sendEnd, auto-set in complete connect
        }
        // Mux should be single input for trueChan and falseChan, single output for outChan
        // Mux must connect to Channel!
        else if (entry.muxPtr != nullptr)
        {
            // Not set to a submodule mux (e.g. the mux in a Lc)
            if (entry.muxPtr->masterName == "")
            {
                auto muxChanGraphPtr = _chanGraph.getNode(entry.muxPtr->moduleName);
                if (muxChanGraphPtr->pre_nodes_data.size() != 2 && muxChanGraphPtr->next_nodes_data.size() != 1)
                {
                    Debug::throwError("Mux:: The number of pre_nodes and next_nodes not satisfy. If default, use \"NULL\" to represent!", __FILE__, __LINE__);
                }
                else
                {
                    string trueChanUpstream = muxChanGraphPtr->pre_nodes_data[0];
                    string falseChanUpstream = muxChanGraphPtr->pre_nodes_data[1];
                    string outChanDownstream = muxChanGraphPtr->next_nodes_data[0];

                    if (trueChanUpstream != "NULL")
                    {
                        entry.muxPtr->trueChan->addUpstream({ getChan(trueChanUpstream) });
                    }
                    if (falseChanUpstream != "NULL")
                    {
                        entry.muxPtr->falseChan->addUpstream({ getChan(falseChanUpstream) });
                    }
                    if (outChanDownstream != "NULL")
                    {
                        entry.muxPtr->outChan->addDownstream({ getChan(outChanDownstream) });
                    }
                }
            }
        }
        // Channel
        else if (entry.chanPtr != nullptr)  
        {
            if (entry.chanPtr->masterName == "")
            {
                auto nodePtr = _chanGraph.getNode(entry.chanPtr->moduleName);
                auto& pre_data = nodePtr->pre_nodes_data;
                auto& pre_active = nodePtr->pre_nodes_active;
                auto& next_data = nodePtr->next_nodes_data;
                auto& next_active = nodePtr->next_nodes_active;

                vector<string> upstreams;
                vector<string> downstreams;
                upstreams.insert(upstreams.end(), pre_data.begin(), pre_data.end());
                upstreams.insert(upstreams.end(), pre_active.begin(), pre_active.end());
                downstreams.insert(downstreams.end(), next_data.begin(), next_data.end());
                downstreams.insert(downstreams.end(), next_active.begin(), next_active.end());

                for (auto& upstream : upstreams)
                {
                    Channel* chanUpstream = nullptr;
                    auto& registryEntry = getRegistryTableEntry(upstream);
                    if (registryEntry.lcPtr != nullptr)
                    {
                        chanUpstream = registryEntry.lcPtr->loopVar;
                    }
                    else if (registryEntry.chanPtr != nullptr)
                    {
                        chanUpstream = registryEntry.chanPtr;
                    }
                    else if (registryEntry.muxPtr != nullptr)
                    {
                        chanUpstream = registryEntry.muxPtr->outChan;
                    }
                    entry.chanPtr->addUpstream({ chanUpstream });
                }

                for (auto& downstream : downstreams)
                {
                    Channel* chanDownstream = nullptr;
                    auto& registryEntry = getRegistryTableEntry(downstream);
                    if (registryEntry.lcPtr != nullptr)
                    {
                        chanDownstream = registryEntry.lcPtr->loopVar;
                    }
                    else if (registryEntry.chanPtr != nullptr)
                    {
                        chanDownstream = registryEntry.chanPtr;
                    }
                    else if (registryEntry.muxPtr != nullptr)
                    {
                        // Do nothing, due to hard to determine connect to trueChan or falseChan
                        // auto-set in completeConnect
                    }
                    entry.chanPtr->addUpstream({ chanDownstream });
                }
            }
        }
        else
        {
            Debug::throwError("Invalid registryTable entry!", __FILE__, __LINE__);
        }
    }

    // completeConnect
    // set noupstream and nodownstream
    for (auto& registryEntry : registryTable)
    {
        if (registryEntry.chanPtr != nullptr)
        {
            auto& chan_ = registryEntry.chanPtr;
            for (auto& upChanPtr : chan_->upstream)
            {
                upChanPtr->addDownstream({ chan_ });
            }

            for (auto& downChanPtr : chan_->downstream)
            {
                downChanPtr->addUpstream({ chan_ });
            }
        }
    }

    // Set noUpstream and noDownstream
    for (auto& registryEntry : registryTable)
    {
        if (registryEntry.chanPtr != nullptr)
        {
            if (registryEntry.chanPtr->upstream.empty())
            {
                registryEntry.chanPtr->noUpstream = true;
            }

            if (registryEntry.chanPtr->downstream.empty())
            {
                registryEntry.chanPtr->noDownstream = true;
            }

        }
    }

    // Remove redundant connect
    for (auto& registryEntry : registryTable)
    {
        if (registryEntry.chanPtr != nullptr)
        {
            set<Channel*> upstreamChans(registryEntry.chanPtr->upstream.begin(), registryEntry.chanPtr->upstream.end());
            registryEntry.chanPtr->upstream.assign(upstreamChans.begin(), upstreamChans.end());

            set<Channel*> downstreamChans(registryEntry.chanPtr->downstream.begin(), registryEntry.chanPtr->downstream.end());
            registryEntry.chanPtr->downstream.assign(downstreamChans.begin(), downstreamChans.end());
        }
    }
}

auto Registry::findRegistryEntryIndex(const string& _moduleName)->unordered_map<string, uint>::iterator
{
    auto iter = registryDict.find(_moduleName);
    if (iter == registryDict.end())
    {
        Debug::throwError("Not find this module!", __FILE__, __LINE__);
    }
    else
    {
        return iter;
    }
}

RegistryTable& Registry::getRegistryTableEntry(const string& _moduleName)
{
    return registryTable[findRegistryEntryIndex(_moduleName)->second];
}

#ifdef DEBUG_MODE  // Get private instance for debug
const vector<RegistryTable>& Registry::getRegistryTable() const
{
    return registryTable;
}
#endif // DEBUG_MODE

