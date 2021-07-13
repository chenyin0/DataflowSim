#include "./Registry.h"
#include "../sim/Debug.h"
#include "../sim/global.h"

using namespace DFSim;

uint Registry::moduleId = 0;
vector<RegistryTableEntry> Registry::registryTable;
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
        RegistryTableEntry entry;
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
        RegistryTableEntry entry;
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
        RegistryTableEntry entry;
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
    
    RegistryTableEntry entry;
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
        RegistryTableEntry entry;
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
        RegistryTableEntry entry;
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

Lse* Registry::getLse(const string& moduleName_)
{
    const auto& ptr = getChan(moduleName_);
    if (ptr->chanType != ChanType::Chan_Lse)
    {
        Debug::throwError("This module is not a Lse!", __FILE__, __LINE__);
    }
    else
    {
        return dynamic_cast<Lse*>(ptr);
    }
}

Lc* Registry::getLc(const string& moduleName_)
{
    return registryTable[registryDict[moduleName_]].lcPtr;
}

Mux* Registry::getMux(const string& moduleName_)
{
    return registryTable[registryDict[moduleName_]].muxPtr;
}

void Registry::init()
{
    checkConnectRule();
    checkLc();

    initChannel();
    initChanDGSFVec();
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
    //    if (entry.nodePtr != nullptr)
    //    {
    //        Channel* chan = entry.nodePtr;
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

            if (entry.chanPtr->chanType == ChanType::Chan_Lse)
            {
                initLse(dynamic_cast<Lse*>(entry.chanPtr));
            }
        }
    }
}

void Registry::initLse(Lse* _lse)
{
    _lse->reqQueue.resize(_lse->size);
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

void Registry::initChanDGSFVec()
{
    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType==ChanType::Chan_DGSF)
        {
            vecChanDGSF.push_back(dynamic_cast<ChanDGSF*>(entry.chanPtr));
        }
    }
}

void Registry::checkNodeRule(ChanGraph& _chanGraph, string& _node)
{
    auto nodePtr = dynamic_cast<Chan_Node*>(_chanGraph.getNode(_node));

    // Check keepMode
    if (nodePtr->chan_mode == "Keep_mode")
    {
        if (nodePtr->node_type == "Lse_ld" || nodePtr->node_type == "Lse_st")
        {
            Debug::throwError("Lse " + nodePtr->node_name + " can not be set in keepMode, need insert a shadow channel !", __FILE__, __LINE__);
        }
        else
        {
            string nextNodeCtrlRegion;
            vector<string> nextNodes;
            nextNodes.insert(nextNodes.end(), nodePtr->next_nodes_data.begin(), nodePtr->next_nodes_data.end());
            nextNodes.insert(nextNodes.end(), nodePtr->next_nodes_active.begin(), nodePtr->next_nodes_active.end());
            for (auto nextNode : nextNodes)
            {
                if (nextNodeCtrlRegion == "")
                {
                    nextNodeCtrlRegion = _chanGraph.getNode(nextNode)->controlRegionName;
                }
                else
                {
                    if (nextNodeCtrlRegion != _chanGraph.getNode(nextNode)->controlRegionName)
                    {
                        Debug::throwError("The nextNode's ctrlRegions of " + nodePtr->node_name + " are not same, need insert a shadow channel", __FILE__, __LINE__);
                    }
                }
            }
        }
    }

    // Check drainMode
    if (nodePtr->chan_mode == "Drain_mode")
    {
        string preNodeCtrlRegion;
        vector<string> preNodes;
        preNodes.insert(preNodes.end(), nodePtr->pre_nodes_data.begin(), nodePtr->pre_nodes_data.end());
        preNodes.insert(preNodes.end(), nodePtr->pre_nodes_active.begin(), nodePtr->pre_nodes_active.end());
        for (auto preNode : preNodes)
        {
            if (preNodeCtrlRegion == "")
            {
                preNodeCtrlRegion = _chanGraph.getNode(preNode)->controlRegionName;
            }
            else
            {
                if (preNodeCtrlRegion != _chanGraph.getNode(preNode)->controlRegionName)
                {
                    Debug::throwError("The preNode's ctrlRegions of " + nodePtr->node_name + " are not same, need insert a shadow channel", __FILE__, __LINE__);
                }
            }
        }
    }
}

void Registry::genModule(ChanGraph& _chanGraph)
{
    auto _nodes = _chanGraph.nodes;
    for (auto& node : _nodes)
    {
        checkNodeRule(_chanGraph, node->node_name);

        auto controlRegion = _chanGraph.controlTree.getCtrlRegion(node->controlRegionName);
        auto controlType = controlRegion.controlType;
        auto branchPath = controlRegion.branchPath;
        auto _node_type = dynamic_cast<Chan_Node*>(node)->node_type;
        if (_node_type == "ChanBase" || _node_type == "ChanDGSF" || _node_type == "ChanPartialMux")
        {
            auto chanPtr = genChan(*dynamic_cast<Chan_Node*>(node));
            chanPtr->subgraphId = node->subgraphId;
            if (controlType == "Branch")
            {
                chanPtr->branchMode = true;
                chanPtr->channelCond = branchPath;
            }
            // Set cond for Cmp operation
            if (dynamic_cast<Chan_Node*>(node)->node_op == "Cmp")
            {
                chanPtr->isCond = true;
            }
        }
        else if (_node_type == "Lc")
        {
            auto& controlRegion = _chanGraph.controlTree.controlRegionTable[_chanGraph.controlTree.findControlRegionIndex(node->controlRegionName)->second];
            if (controlRegion.upperControlRegion.empty())
            {
                auto lcPtr = genLcOuterMost(*dynamic_cast<Chan_Node*>(node));
                lcPtr->subgraphId = node->subgraphId;
                lcPtr->loopVar->subgraphId = node->subgraphId;
                lcPtr->loopVar->speedup = dynamic_cast<Chan_Node*>(node)->speedup;
            }
            else
            {
                auto lcPtr = genLc(*dynamic_cast<Chan_Node*>(node));
                lcPtr->subgraphId = node->subgraphId;
                lcPtr->loopVar->subgraphId = node->subgraphId;
                lcPtr->loopVar->speedup = dynamic_cast<Chan_Node*>(node)->speedup;
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
            auto lsePtr = genLse(*dynamic_cast<Chan_Node*>(node));
            lsePtr->subgraphId = node->subgraphId;
            lsePtr->baseAddr = dynamic_cast<Chan_Node*>(node)->baseAddr;
            if (controlType == "Branch")
            {
                lsePtr->branchMode = true;
                lsePtr->channelCond = branchPath;
            }
        }
        else if (_node_type == "Mux")
        {
            auto muxPtr = genMux(*dynamic_cast<Chan_Node*>(node));
            muxPtr->subgraphId = node->subgraphId;
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
    // TODO: Set AluOp and const of channel!
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
        //chan->size = DGSF_INPUT_BUFF_SIZE;

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
    
    if (chan_ != nullptr)
    {
        chan_->isPhysicalChan = _chan.isPhysicalChan;
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
            if (entry.chanPtr->masterName == "None" && entry.chanPtr->moduleName != "Chan_end" && entry.chanPtr->moduleName != "Chan_begin")
            {
                const auto& nodePtr = _chanGraph.getNode(entry.chanPtr->moduleName);
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
                    entry.chanPtr->addDownstream({ chanDownstream });
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
            //set<Channel*> upstreamChans(registryEntry.nodePtr->upstream.begin(), registryEntry.nodePtr->upstream.end());
            //registryEntry.nodePtr->upstream.assign(upstreamChans.begin(), upstreamChans.end());

            //set<Channel*> downstreamChans(registryEntry.nodePtr->downstream.begin(), registryEntry.nodePtr->downstream.end());
            //registryEntry.nodePtr->downstream.assign(downstreamChans.begin(), downstreamChans.end());

            auto upstream_ = Util::removeDuplicatesKeepSequence(registryEntry.chanPtr->upstream);
            registryEntry.chanPtr->upstream.assign(upstream_.begin(), upstream_.end());

            auto downstream_ = Util::removeDuplicatesKeepSequence(registryEntry.chanPtr->downstream);
            registryEntry.chanPtr->downstream.assign(downstream_.begin(), downstream_.end());
        }
    }
}

void Registry::configGraphScheduler(GraphScheduler* _graphScheduler)
{
    for (auto& entry : registryTable)
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_DGSF)
        {
            const auto& _chanPtr = dynamic_cast<ChanDGSF*>(entry.chanPtr);
            if (_chanPtr->upstream.size() != 1)
            {
                Debug::throwError("The upstream of a chanDGSF must equal to 1!", __FILE__, __LINE__);
            }
            else
            {
                // Inherit branch mode from the upstream channel
                _chanPtr->isCond = _chanPtr->upstream[0]->isCond;
                _chanPtr->branchMode = _chanPtr->upstream[0]->branchMode;
                _chanPtr->channelCond = _chanPtr->upstream[0]->channelCond;

                // Add consumerChan
                _graphScheduler->addSubgraph(_chanPtr->subgraphId, {}, { _chanPtr });
                // Add producerChan
                for (auto& downstreamChan : _chanPtr->downstream)
                {
                    //if (downstreamChan->subgraphId != _chanPtr->subgraphId)
                    //{
                    //    _graphScheduler->addSubgraph(downstreamChan->subgraphId, { _chanPtr }, { });
                    //}

                    _graphScheduler->addSubgraph(downstreamChan->subgraphId, { _chanPtr }, { });
                }

                // For branch
                // Cond chan
                if (_chanPtr->isCond)
                {
                    _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, { _chanPtr }, {}, {});
                }

                //for (auto& downstreamChan : _chanPtr->downstream)
                //{
                //    if (downstreamChan->branchMode)
                //    {
                //        if (downstreamChan->channelCond == true)
                //        {
                //            _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, {}, {_chanPtr}, {});
                //        }
                //        else
                //        {
                //            _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, {}, {}, { _chanPtr });
                //        }
                //    }
                //}

                bool isTruePathChan = 0;
                bool isFalsePathChan = 0;
                for (auto& downstreamChan : _chanPtr->downstream)
                {
                    if (downstreamChan->branchMode)
                    {
                        if (downstreamChan->channelCond == true)
                        {
                            isTruePathChan = 1;
                        }
                        else
                        {
                            isFalsePathChan = 1;
                        }
                    }
                }

                if (isTruePathChan && isFalsePathChan)
                {
                    _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, { _chanPtr }, {}, {});  // Common chan
                }
                else
                {
                    if (isTruePathChan)
                    {
                        _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, {}, { _chanPtr }, {});  // TruePath chan
                    }

                    if (isFalsePathChan)
                    {
                        _graphScheduler->addDivergenceSubgraph(_chanPtr->subgraphId, {}, {}, { _chanPtr });  // FalsePath chan
                    }
                }
            }
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

RegistryTableEntry& Registry::getRegistryTableEntry(const string& _moduleName)
{
    return registryTable[findRegistryEntryIndex(_moduleName)->second];
}

void Registry::genSimConfig(ChanGraph& _chanGraph)
{
    vector<string> simNodes = _chanGraph.bfsTraverseNodes();
    vector<string> configNodes = _chanGraph.bfsTraverseNodesWithCtrlRegionSequence();
    
    std::ofstream _config_file;
    _config_file.open(Global::file_path + App_name_convert::toString(Global::app_name) + string("_config_") + string(xstr(ARCH)) + string(".txt"));

    // gen channel declare
    _config_file << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << "******* Below is gen channel declare *******" << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << std::endl;
    for (auto& chanName : configNodes)
    {
        _config_file << "const auto& " << chanName << " = registry->";
        auto& entry = getRegistryTableEntry(chanName);
        if (entry.moduleType == ModuleType::Channel)
        {
            _config_file << "getChan(\"" << chanName << "\");";
        }
        else if (entry.moduleType == ModuleType::Lc)
        {
            _config_file << "getLc(\"" << chanName << "\");";
        }
        else if (entry.moduleType == ModuleType::Mux)
        {
            _config_file << "getMux(\"" << chanName << "\");";
        }
        _config_file << std::endl;
    }

    // gen sim
    _config_file << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << "******* Below is gen simulation config *******" << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << std::endl;
    for (auto& chanName : simNodes)
    {
        if (chanName != "Chan_begin" && chanName != "Chan_end")
        {
            auto& entry = getRegistryTableEntry(chanName);
            if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType != ChanType::Chan_DGSF)
            {
                const auto& chanPtr = dynamic_cast<Chan_Node*>(_chanGraph.getNode(chanName));
                string& op = chanPtr->node_op;
                _config_file << chanName << "->get();" << "\t// " << op << "\t";
                for (size_t i = 0; i < chanPtr->pre_nodes_data.size(); ++i)
                {
                    _config_file << "[" << i << "]" << chanPtr->pre_nodes_data[i] << " ";
                }
                _config_file << std::endl;

                if (op != "Store")
                {
                    _config_file << chanName << "->value = ";
                    if (op == "Load")
                    {
                        _config_file << chanName << "->assign();";
                    }
                    else
                    {
                        // Assign by chanName
                        /*for (auto& preNode : _chanGraph.getNode(chanName)->pre_nodes_data)
                        {
                            _config_file << chanName << "->assign(" << preNode << ")";
                        }*/

                        // Assign by chanBufferId, to support add chanDGSF
                        for (size_t bufferId = 0; bufferId < _chanGraph.getNode(chanName)->pre_nodes_data.size(); ++bufferId)
                        {
                            _config_file << chanName << "->assign(uint(" << bufferId << "))";
                        }
                        _config_file << ";";
                    }
                }
            }
            else if (entry.moduleType == ModuleType::Lc)
            {
                _config_file << ">>> Here should add a Lc: " << chanName;
                _config_file << "\n// Lc: " << chanName;
            }
            else if (entry.moduleType == ModuleType::Mux)
            {
                _config_file << ">>> Here should add a Mux: " << chanName;
            }
            _config_file << std::endl;
            _config_file << std::endl;
        }
    }

    //// gen debugPrint
    //_config_file << std::endl;
    //_config_file << "*********************************************" << std::endl;
    //_config_file << "******* Below is gen debug print *******" << std::endl;
    //_config_file << "*********************************************" << std::endl;
    //_config_file << std::endl;

    //// Print chan
    //_config_file << "//** Print channle" << std::endl;
    //for (auto& chanName : simNodes)
    //{
    //    auto& entry = getRegistryTableEntry(chanName);
    //    if (entry.moduleType == ModuleType::Channel)
    //    {
    //        if (entry.nodePtr->chanType == ChanType::Chan_Lse)
    //        {
    //            _config_file << "debug->lsePrint(\"" << chanName << "\", " << "dynamic_cast<Lse*>(" << chanName << "));" << std::endl;
    //        }
    //        else
    //        {
    //            _config_file << "debug->chanPrint(\"" << chanName << "\", " << chanName << ");" << std::endl;
    //        }
    //    }
    //    else if (entry.moduleType == ModuleType::Lc)
    //    {
    //        _config_file << "debug->getFile() << std::endl;" << std::endl;
    //        _config_file << "debug->getFile() << \"************ Lc: \" << \"" << chanName << "\" << \"***********\" << std::endl;" << std::endl;
    //        _config_file << "debug->chanPrint(\"" << chanName << "->loopVar" << "\", " << chanName << "->loopVar" << ");" << std::endl;
    //    }
    //}

    //// Print End signal
    //_config_file << std::endl;
    //_config_file << "debug->getFile() << std::endl;" << std::endl;
    //_config_file << "debug->getFile() << \"*****************  End signal  *****************\" << std::endl;" << std::endl;
    //for (auto& chanName : simNodes)
    //{
    //    auto& entry = getRegistryTableEntry(chanName);
    //    if (entry.moduleType == ModuleType::Lc)
    //    {
    //        _config_file << "debug->chanPrint(\"" << chanName << "->getEnd" << "\", " << chanName << "->getEnd" << ");";
    //        _config_file << "debug->getFile() << \"" << chanName << " loopEnd: \" << " << chanName << "->loopEnd << std::endl;" << std::endl;
    //    }
    //}

    // gen subgraph manually partition config
    _config_file << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << "******* Below is subgraph manually partition config *******" << std::endl;
    _config_file << "*********************************************" << std::endl;
    _config_file << std::endl;
    for (auto& chanName : configNodes)
    {
        _config_file << "chanGraph.getNode(\"" << chanName << "\")->subgraphId = 0;" << std::endl;
    }
}

auto Registry::genDebugPrint(ChanGraph& _chanGraph)->tuple<vector<Channel*>, vector<Lc*>>
{
    vector<string> simNodes = _chanGraph.bfsTraverseNodesWithCtrlRegionSequence();
    //unordered_map<string, vector<string>> simNodesWithCtrlRegion;  // string: ctrlRegion; vector<string>: Nodes in each ctrlRegion
    //for (auto& nodeName : simNodes)
    //{
    //    string ctrlRegionName = _chanGraph.getNode(nodeName)->controlRegionName;
    //    auto iter = simNodesWithCtrlRegion.find(ctrlRegionName);
    //    if (iter == simNodesWithCtrlRegion.end())
    //    {
    //        simNodesWithCtrlRegion.insert(make_pair(ctrlRegionName, vector<string>{nodeName}));
    //    }
    //    else
    //    {
    //        iter->second.push_back(nodeName);
    //    }
    //}

    vector<Channel*> chans;
    vector<Lc*> lc;

    for (auto& chanName : simNodes)
    {
        auto& entry = getRegistryTableEntry(chanName);
        if (entry.moduleType == ModuleType::Channel)
        {
            chans.push_back(entry.chanPtr);
        }
        else if (entry.moduleType == ModuleType::Lc)
        {
            chans.push_back(entry.lcPtr->loopVar);
            lc.push_back(entry.lcPtr);
        }
    }

    //for (auto& chanName : simNodes)
    //{
    //    auto& entry = getRegistryTableEntry(chanName);
    //    if (entry.moduleType == ModuleType::Channel)
    //    {
    //        chans.push_back(entry.chanPtr);
    //    }
    //    else if (entry.moduleType == ModuleType::Lc)
    //    {
    //        chans.push_back(entry.lcPtr->loopVar);
    //        lc.push_back(entry.lcPtr);
    //    }
    //}

    /*for (auto iter = simNodesWithCtrlRegion.begin(); iter != simNodesWithCtrlRegion.end(); ++iter)
    {
        for (auto& chanName : iter->second)
        {
            auto& entry = getRegistryTableEntry(chanName);
            if (entry.moduleType == ModuleType::Channel)
            {
                chans.push_back(entry.chanPtr);
            }
            else if (entry.moduleType == ModuleType::Lc)
            {
                chans.push_back(entry.lcPtr->loopVar);
                lc.push_back(entry.lcPtr);
            }
        }
    }*/

    return make_tuple(chans, lc);
}

//void Registry::setSpeedup(ChanGraph& _chanGraph, const string& _controlRegion, uint _speedup)
//{
//    auto& ctrlRegion = _chanGraph.controlTree.getCtrlRegion(_controlRegion);
//    for (auto& chanNode : ctrlRegion.nodes)
//    {
//        auto& nodeName = _chanGraph.getNode(chanNode)->node_name;
//        if (nodeName != "Chan_begin" && nodeName != "Chan_end")
//        {
//            auto& registryEntry = getRegistryTableEntry(chanNode);
//            if (registryEntry.moduleType == ModuleType::Channel)
//            {
//                if (registryEntry.nodePtr->masterName == "None")
//                {
//                    registryEntry.nodePtr->speedup = _speedup;
//                }
//            }
//        }
//    }
//}

void Registry::setChanSize()
{
    for (auto& entry : registryTable)
    {
        if (entry.chanPtr != nullptr)
        {
            if (entry.chanPtr->masterName == "None"
                && (entry.chanPtr->moduleName != "Chan_begin" && entry.chanPtr->moduleName != "Chan_end")
                && entry.chanPtr->chanType != ChanType::Chan_DGSF)
            {
                entry.chanPtr->size = 10 * std::max(entry.chanPtr->cycle, uint(1)) * entry.chanPtr->speedup;

                /*if (entry.nodePtr->chanType == ChanType::Chan_Lse)
                {
                    entry.nodePtr->size = 20 * std::max(entry.nodePtr->cycle, uint(1)) * entry.nodePtr->speedup;
                }
                else
                {
                    entry.nodePtr->size = 10 * std::max(entry.nodePtr->cycle, uint(1)) * entry.nodePtr->speedup;
                }*/
            }
        }
    }
}

//uint Registry::getSubgraphPhysicalNodeNum(ChanGraph& _chanGraph, const string& _controlRegion)
//{
//    uint nodeNum = 0;
//    auto ctrlRegion = _chanGraph.controlTree.getCtrlRegion(_controlRegion);
//    for (auto& nodeName : ctrlRegion.nodes)
//    {
//        const auto& node = dynamic_cast<Chan_Node*>(_chanGraph.getNode(nodeName));
//        if (node->isPhysicalChan)
//        {
//            ++nodeNum;
//        }
//    }
//
//    return nodeNum;
//}

//void Registry::setSpeedup(ChanGraph& _chanGraph)
//{
//    unordered_map<uint, vector<Chan_Node*>> subgraphNodes;  // <uint, vector<Chan_Node*>> = <subgraphId, nodesInThisSubgraph>
//    for (auto& node : _chanGraph.nodes)
//    {
//        if (dynamic_cast<Chan_Node*>(node)->isPhysicalChan)
//        {
//            if (subgraphNodes.count(node->subgraphId) == 0)
//            {
//                vector<Chan_Node*> tmp = { dynamic_cast<Chan_Node*>(node) };
//                subgraphNodes.insert(make_pair(node->subgraphId, tmp));
//            }
//            else
//            {
//                subgraphNodes[node->subgraphId].push_back(dynamic_cast<Chan_Node*>(node));
//            }
//        }
//    }
//
//    vector<uint> subgraphSpeedupTable(subgraphNodes.size());
//    for (auto& subgraph : subgraphNodes)
//    {
//        uint nodeNum = subgraph.second.size();
//        uint speedup = ARRAY_SIZE / nodeNum;
//        subgraphSpeedupTable[subgraph.first] = speedup;  // Record speedup of each subgraph
//        if (speedup < 1)
//        {
//            Debug::throwError("[Config Error] Current subgraph size is larger than array size", __FILE__, __LINE__);
//        }
//        else
//        {
//            for (auto& node : subgraph.second)
//            {
//                auto& nodeName = node->node_name;
//                if (nodeName != "Chan_begin" && nodeName != "Chan_end")
//                {
//                    auto& registryEntry = getRegistryTableEntry(nodeName);
//                    if (registryEntry.moduleType == ModuleType::Channel)
//                    {
//                        if (registryEntry.nodePtr->masterName == "None")
//                        {
//                            registryEntry.nodePtr->speedup = speedup;
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    // Print subgraph speedup
//    std::cout << std::endl;
//    std::cout << "********* Subgraph speedup *********" << std::endl;
//    for (size_t i = 0; i < subgraphSpeedupTable.size(); ++i)
//    {
//        std::cout << "SubgraphId: " << i << "\t" << "Speedup: " << subgraphSpeedupTable[i] << std::endl;
//    }
//    std::cout << std::endl;
//}

void Registry::updateChanDGSF()
{
    for (auto& chanDGSF : vecChanDGSF)
    {
        chanDGSF->get();
        chanDGSF->value = chanDGSF->assign(uint(0));
    }
}

#ifdef DEBUG_MODE  // Get private instance for debug
const vector<RegistryTableEntry>& Registry::getRegistryTable() const
{
    return registryTable;
}
#endif // DEBUG_MODE

