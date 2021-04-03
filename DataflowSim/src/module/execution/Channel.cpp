#include "Channel.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"
#include "../Registry.h"

using namespace DFSim;

Channel::Channel(uint _size, uint _cycle) : size(_size), cycle(_cycle)
{
    speedup = 1;  // Default speedup = 1, signify no speedup
    //currId = 1;  // Begin at 1
    initial();
}

Channel::Channel(uint _size, uint _cycle, uint _speedup) :
    size(_size), cycle(_cycle), speedup(_speedup)
{
    initial();
}

void Channel::initial()
{
    // Register itself in the registerTable
    moduleId = Registry::registerChan(this);
}

Channel::~Channel()
{
    for (auto& chan : upstream)
    {
        //delete chan;
        chan = nullptr;
    }

    for (auto& chan : downstream)
    {
        //delete chan;
        chan = nullptr;
    }
}

void Channel::addUpstream(const vector<Channel*>& _upStream)
{
    for (auto& chan : _upStream)
    {
        upstream.push_back(chan);
    }
}

void Channel::addDownstream(const vector<Channel*>& _downStream)
{
    for (auto& chan : _downStream)
    {
        downstream.push_back(chan);
    }
}

//void Channel::checkConnect()
//{
//    if ((!noUpstream && upstream.empty()) || (!noDownstream && downstream.empty()))
//    {
//        Debug::throwError("Upstream/Downstream is empty!", __FILE__, __LINE__);
//    }
//
//    //if (inputData.size() != upstream.size())
//    //{
//    //    Debug::throwError("InputData's size is not equal to upstream's size!", __FILE__, __LINE__);
//    //}
//}

void Channel::parallelize()
{
    //// Push clkStall in parallel execution mode
    //if (currId != speedup && !channel.empty())  // If the parallel execution dosen't finish, stall the system clock;
    //{
    //    ClkDomain::getInstance()->addClkStall();
    //}
    //currId = currId % speedup + 1;

    // Push clkStall in parallel execution mode
    if (currId < speedup && !channel.empty())  // If the parallel execution dosen't finish, stall the system clock;
    {
        ClkDomain::getInstance()->addClkStall();
        ++currId;
    }
    else if (ClkDomain::checkClkAdd())  // If system clk has updated, reset currId
    {
        currId = 1;

        //if (chanClk != ClkDomain::getClk())  // If system clk has updated
        //{
        //    currId = 1;  // Reset currId
        //    chanClk = ClkDomain::getClk();
        //}

        // If system clk has updated, reset currId
        //if (ClkDomain::checkClkAdd())
        //{
        //    currId = 1;
        //}
    }
}

void Channel::sendLastTag()
{
    for (auto& chan : upstream)
    {
        if (chan->keepMode)
        {
            for (auto& queue : chan->lastTagQueue)
            {
                if (queue.first == this->moduleId)
                {
                    queue.second.push_back(1);
                }
            }
        }
    }
}

int Channel::getChanId(Channel* chan)
{
    for (size_t i = 0; i < upstream.size(); ++i)
    {
        if (upstream[i] == chan)
        {
            return i;
        }
    }

    Debug::throwError("Not find this channle in upstream vector!", __FILE__, __LINE__);
    return -1;
}

// Channel get data from the program variable 
vector<int> Channel::get(const vector<int>& data)
{
    vector<int> pushState(2);
    vector<int> popState(2);

    //checkConnect();
    popState = pop(); // Data lifetime in nested loop

    for (size_t i = 0; i < data.size(); ++i)
    {
        // Note: the sequence of data in data vector should consistent with corresponding upstream and buffer in chanBuffer
        pushState = push(data[i], i);
    }

    statusUpdate(); // Set valid according to the downstream channels' status
    //bpUpdate(); 

    parallelize();

    return { pushState[0], pushState[1], popState[0], popState[1] };
}

// Channel get data from the program variable 
vector<int> Channel::get()
{
    vector<int> pushState(2);
    vector<int> popState(2);

    if (isLoopVar || currId <= speedup)
    {
        popState = pop(); // Data lifetime in nested loop

        for (size_t i = 0; i < upstream.size(); ++i)
        {
            // Note: the sequence of data in data vector should consistent with corresponding upstream and buffer in chanBuffer
            pushState = push(upstream[i]->value, i);
        }

        statusUpdate(); // Set valid according to the downstream channels' status
    }
       
    //bpUpdate();
    parallelize();

    return { pushState[0], pushState[1], popState[0], popState[1] };
}

// Assign channel value to program varieties
int Channel::assign(uint bufferId)
{
    //if (!this->channel.empty())
    //{
    //    Data data = channel.front();
    //    return data.value;
    //}
    //else
    //    return lastPopVal;

    if (!this->chanBuffer[bufferId].empty())
    {
        return chanBuffer[bufferId].front().value;
    }
    else
        return lastPopVal[bufferId];
}

// Assign value according to upstream channel's pointer
int Channel::assign(Channel* chan)
{
    uint bufferId = getChanId(chan);

    return this->assign(bufferId);
}

vector<int> Channel::push(int data, uint bufferId)
{
    // Push data in channel
    if (checkUpstream(bufferId))
    {
        pushBuffer(data, bufferId);
        return { 1, data };
    }
    else
        return { 0, data };
}

bool Channel::checkUpstream(uint bufferId)
{
    bool ready = 1;
    if (!noUpstream)
    {
        //for (auto channel : upstream)
        //{
        //    if (!channel->valid)
        //    {
        //        ready = 0;
        //        break;
        //    }
        //}
        uint chanId = bufferId;
        if (!upstream[chanId]->valid)
        {
            ready = 0;
        }
        else if (drainMode)
        {
            // In drainMode, outer-loop channels only receive the last data of the inner-loop; 
            // Other data will be drained by the outer-loop channel
            if (!upstream[chanId]->channel.front().last)
            {
                ready = 0;
            }
        }
    }
    else
    {
        //if (channel.size() == size)  // When channel is full
        //    ready = 0;
        if (chanBuffer[bufferId].size() == size)  // When channel is full
            ready = 0;
    }

    return ready;
}

void Channel::pushBuffer(int _data, uint _bufferId)
{
    if (!noUpstream)
    {
        uint upstreamId = _bufferId;
        Data data = upstream[upstreamId]->channel.front();
        data.value = _data;
        //data.cycle = ClkDomain::getInstance()->getClk() + cycle;  // Update cycle when push into chanBuffer

        if (upstream[upstreamId]->keepMode)
        {
            data.cycle = ClkDomain::getInstance()->getClk();
        }
        else
        {
            if (branchMode)
            {
                if (data.cond == channelCond)
                {
                    data.cycle = ClkDomain::getInstance()->getClk() + cycle;
                }
                else
                {
                    data.cycle = ClkDomain::getInstance()->getClk();  // If in the falsePath, no need to update cycle
                }
            }
            else
            {
                data.cycle = ClkDomain::getInstance()->getClk() + cycle;
            }
        }

        // Avoid receive extra poison data when upstream is in keepMode
        if (!upstream[_bufferId]->keepMode)
        {
            chanBuffer[_bufferId].push_back(data);
            ++chanBufferDataCnt[_bufferId];
        }
        else
        {
            if (chanBuffer[_bufferId].empty())
            {
                chanBuffer[_bufferId].push_back(data);
                ++chanBufferDataCnt[_bufferId];
            }
        }
    }
    else
    {
        Data data;
        data.valid = 1;
        data.value = _data;
        data.cycle = ClkDomain::getInstance()->getClk() + cycle;  // Update cycle when push into chanBuffer
        chanBuffer[_bufferId].push_back(data);
        ++chanBufferDataCnt[_bufferId];
    }
}

bool Channel::checkSend(Data _data, Channel* _upstream)
{
    bool sendable = 1;
    uint bufferId = getChanId(_upstream);
    //// Find corresponding bufferId of upstream
    //for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
    //{
    //    if (upstream[chanId] == _upstream)
    //    {
    //        bufferId = chanId;
    //        break;
    //    }
    //}

    if (bp[bufferId] == 1)
    {
        sendable = 0;
    }
    return sendable;
}

void Channel::bpUpdate()
{
    //if (channel.size() < size)
    //    bp = 0;
    //else
    //    bp = 1;

    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
    {
        if (chanBuffer[bufferId].size() < size)
        {
            bp[bufferId] = 0;
        }
        else
        {
            bp[bufferId] = 1;
        }
    }
}

//bool Channel::checkUpstream()
//{
//    bool ready = 1;
//    if (!noUpstream)
//    {
//        for (auto channel : upstream)
//        {
//            if (!channel->valid)
//            {
//                ready = 0;
//                break;
//            }
//        }
//    }
//    else
//    {
//        if (channel.size() == size)  // When channel is full
//            ready = 0;
//    }
//
//    return ready;
//}

//bool Channel::checkSend(Data _data, Channel* upstream)
//{
//    bool sendable = 1;
//    if (bp)
//    {
//        sendable = 0;
//    }
//    return sendable;
//}

//void Channel::bpUpdate()
//{
//    if (channel.size() < size)
//        bp = 0;
//    else
//        bp = 1;
//}


// class ChanBase
ChanBase::ChanBase(uint _size, uint _cycle) : 
    Channel(_size, _cycle)
{
    initial();
}

ChanBase::ChanBase(uint _size, uint _cycle, uint _speedup) : 
    Channel(_size, _cycle, _speedup)
{
    initial();
}

void ChanBase::initial()
{
    enable = 1;
    //bp = 0;

    // Default branch setting
    branchMode = 0;
    isCond = 0;
    channelCond = 1;

    noUpstream = 0;
    noDownstream = 0;

    chanType = ChanType::Chan_Base;
}

//bool ChanBase::checkUpstream(uint bufferId)
//{
//    bool ready = 1;
//    if (!noUpstream)
//    {
//        //for (auto channel : upstream)
//        //{
//        //    if (!channel->valid)
//        //    {
//        //        ready = 0;
//        //        break;
//        //    }
//        //}
//        uint chanId = bufferId;
//        if (!upstream[chanId]->valid)
//        {
//            ready = 0;
//        }
//    }
//    else
//    {
//        //if (channel.size() == size)  // When channel is full
//        //    ready = 0;
//        if (chanBuffer[bufferId].size() == size)  // When channel is full
//            ready = 0;
//    }
//
//    return ready;
//}

//void ChanBase::pushBuffer(int _data, uint _bufferId)
//{
//    uint upstreamId = _bufferId;
//    Data data = upstream[upstreamId]->channel.front();
//    data.value = _data;
//    data.cycle = ClkDomain::getInstance()->getClk() + cycle;  // Update cycle when push into chanBuffer
//    chanBuffer[_bufferId].push_back(data);
//}

void ChanBase::pushChannel()
{
    if (!noUpstream)
    {
        //Data data = upstream.front()->channel.front();
        Data data;
        data.valid = 1;
        data.tag = chanBuffer[0].front().tag;  // Inherit tag
        //data.last = 0;  // Reset last flag
        //data.value = _data;  // No need data value
        //data.cycle = data.cycle + cycle;
        //uint cycleTemp = data.cycle + cycle;
        //data.cycle = cycleTemp > clk ? cycleTemp : clk;

        // Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
        updateDataStatus(data);

        //// Push getLast
        //if (data.last)
        //    getLast.push_back(1);

        // Send lastTag to each upstream channel in keepMode
        // 1) Normal channel send last tag to upstream channels in keepMode when pushChannel;
        // 2) But LoopVar channel send last tag when popChannel!
        if (data.last && !isLoopVar)
        {
            //if (isLoopVar)
            //{
            //    data.last = 0;  //Reset data last flag, due to loopVar not receive last, only receive lastOuter
            //    getLast.push_back(1);
            //}
            //else
            //{
            //    sendLastTag();
            //}

            sendLastTag();
        }    

        bool getIsCond = 0;  // Signify has got a cond
        if (branchMode)
        {
            // Update data cond when a upstream is in isCond mode
            for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
            {
                if (upstream[chanId]->isCond)
                {
                    if (getIsCond)
                    {
                        if (data.cond != chanBuffer[chanId].front().cond)
                        {
                            Debug::throwError("The cond flag of each iscond channel is not consistent!", __FILE__, __LINE__);
                        }
                    }
                    else
                    {
                        data.cond = chanBuffer[chanId].front().cond;
                        getIsCond = 1;
                    }
                }
            }

            //if (data.cond == channelCond)
            //{
            //    channel.push_back(data);
            //}
            //else
            //{
            //    // Pop out data in the chanBuffer directly
            //    for (auto& buffer : chanBuffer)
            //    {
            //        buffer.pop_front();
            //    }
            //}

            //if (!upstream.front()->isCond)
            //{
            //    // The cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
            //    Debug::throwError("The cond channel must be in the first element of the upstream vector!", __FILE__, __LINE__);
            //}
            //else if (data.cond == channelCond)
            //{
            //    channel.push_back(data);
            //}

            //if (data.graphSwitch)
            //{
            //    channel.back().graphSwitch = 1;  // In branch mode, if upstream send a graphSwitch flag, all the downstreams(branch paths) must receive it.
            //}
        }
        //else
        //{
        //    channel.push_back(data);
        //}

        // Inherit data cond from upstream channel
        if (!getIsCond)
        {
            if (!upstream[0]->isCond)
            {
                bool getCond = 0;
                for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
                {
                    if (getCond)
                    {
                        if (data.cond != chanBuffer[chanId].front().cond)
                        {
                            Debug::throwError("The cond flag of each upstream channel is not consistent!", __FILE__, __LINE__);
                        }
                    }
                    else
                    {
                        data.cond = chanBuffer[chanId].front().cond;
                        getCond = 1;
                    }
                }
            }
            else
            {
                data.cond = chanBuffer[0].front().cond;
            }
        }

        channel.push_back(data);
    }
    else
    {
        Data data = Data();
        data.valid = 1;
        //data.value = _data;
        data.cycle = ClkDomain::getInstance()->getClk();
        channel.push_back(data);
    }
}

void ChanBase::updateDataStatus(Data& data)
{
    //// Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
    //for (auto channel : upstream)
    //{
    ////// loopVar not receive last, only receive lastOuter
    ////// Due to a channel in keepMode may repeatly send a data with a last for many times
    ////if (/*!isCond*/ !isLoopVar && channel->keepMode == 0)
    ////{
    ////    data.last |= channel->channel.front().last;
    ////}

    //// Due to a channel in keepMode may repeatly send a data with a last for many times
    //if (channel->keepMode == 0)
    //{
    //    data.last |= channel->channel.front().last;
    //}
    //data.lastOuter |= channel->channel.front().lastOuter;
    //data.graphSwitch |= channel->channel.front().graphSwitch;
    //}

    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
    {
        // Due to a channel in keepMode may repeatly send a data with a last for many times
        // The data received by a drainMode channel is always with a last flag, so ignore it
        if (upstream[bufferId]->keepMode == 0 && this->drainMode == 0)  // upstreamId is equal to bufferId
        {
            data.last |= chanBuffer[bufferId].front().last;
            data.lastOuter |= chanBuffer[bufferId].front().lastOuter;
        }
        else
        {
            //if (chanBuffer[bufferId].front().last && isLoopVar)
            //{
            //    data.lastOuter = 1;  // Signify inner loop has received a last from outer loop
            //}

            if (chanBuffer[bufferId].front().last && chanBuffer[bufferId].front().lastOuter && isLoopVar)
            {
                data.lastOuter = 1;  // Signify inner loop has received a last from outer loop
            }
        }

        //data.lastOuter |= chanBuffer[bufferId].front().lastOuter;
        data.graphSwitch |= chanBuffer[bufferId].front().graphSwitch;
    }
}

bool ChanBase::popLastCheck()
{
    bool popLastReady = 1;

    if (!noDownstream && keepMode)
    {
        //// If the only downstream is inner loop's loopVar, pop data when the loopVar produce last tag
        //if(downstream.size() == 1 && downstream[0]->isLoopVar)  
        //{
        //    if (downstream[0]->produceLast.empty())
        //    {
        //        popLastReady = 0;
        //    }
        //}
        //else
        //{
        //    for (auto channel : downstream)
        //    {
        //        // If only one of the downstream channel set last, the data can not be poped
        //        // lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
        //        if (/*!channel->isCond*/ !channel->isLoopVar && channel->getLast.empty())
        //        {
        //            popLastReady = 0;
        //            break;
        //        }
        //    }
        //}

        //for (auto channel : downstream)
        //{
        //    // If only one of the downstream channel set last, the data can not be poped
        //    // If the channel is loopVar, check produceLast queue; else check getLast queue;
        //    if ((channel->isLoopVar && channel->produceLast.empty()) || (!channel->isLoopVar && channel->getLast.empty()))
        //    {
        //        popLastReady = 0;
        //        break;
        //    }
        //}

        for (auto& queue : lastTagQueue)
        {
            if (queue.second.empty())
            {
                popLastReady = 0;
                break;
            }
        }
    }

    return popLastReady;
}

vector<int> ChanBase::popChannel(bool popReady, bool popLastReady)
{
    int popSuccess = 0;
    int popData = 0;

    if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
    {
        //Data data = channel.front();
        channel.pop_front();
        popSuccess = 1;
        //popData = data.value;
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            lastPopVal[bufferId] = chanBuffer[bufferId].front().value;  // For LC->loopVar, record last pop data when the channel pop empty
            chanBuffer[bufferId].pop_front();
        }

        //lastPopVal = data.value;  // For LC->loopVar, record last pop data when the channel pop empty

        //// Clear the last flags of downstreams
        //if (keepMode)
        //{
        //    for (auto& channel : downstream)
        //    {
        //        // lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
        //        if (!channel->isLoopVar)
        //        {
        //            channel->getLast.pop_front();
        //        }
        //        else
        //        {
        //            // If it is a loopVar, pop produceLast queue
        //            if (!channel->produceLast.empty())
        //            {
        //                channel->produceLast.pop_front();
        //            }
        //        }
        //    }
        //}

        // Clear the last tag of lastTagQueue
        if (keepMode)
        {
            for (auto& queue : lastTagQueue)
            {
                queue.second.pop_front();  // Pop out a lastTag
            }
        }
    }

    return { popSuccess , popData };
}

vector<int> ChanBase::pop()
{
    bool popReady = valid;

    // popLastReady only used in keepMode
    bool popLastReady = popLastCheck();
    vector<int> popState = popChannel(popReady, popLastReady);
    //updateCycle(popReady, popLastReady);

    return { popState[0], popState[1] };  // For debug
}

//// Update cycle in keepMode
//void ChanBase::updateCycle(bool popReady, bool popLastReady)
//{
//    // Update cycle in keepMode, only when the system clk updates successfully
//    if (ClkDomain::getInstance()->checkClkAdd())
//    {
//        if (popReady && keepMode && !popLastReady)  // Update data cycle in keepMode
//        {
//            for (auto& data : channel)
//            {
//                ++data.cycle;
//            }
//        }
//    }
//}

//vector<int> ChanBase::push(int data, uint bufferId)
//{
//    // Push data in channel
//    if (checkUpstream(bufferId))
//    {
//        pushBuffer(data, bufferId);
//        return { 1, data };
//    }
//    else
//        return { 0, data };
//}

void ChanBase::statusUpdate()
{
    uint clk = ClkDomain::getInstance()->getClk();
    // Set valid
    valid = 1;

    //bool match = 1;
    //for (auto& buffer : chanBuffer)
    //{
    //    if (buffer.empty() || buffer.front().cycle > clk)
    //    {
    //        match = 0;
    //    }
    //}

    bool match = checkDataMatch();

    if (match && channel.empty())  // Channel size is 1
    {
        pushChannel();
    }

    if (channel.empty() || !enable)
    {
        valid = 0;
    }
    else
    {
        ////uint clk = ClkDomain::getInstance()->getClk();
        //Data data = channel.front();

        //if (data.cycle > clk)
        //{
        //    valid = 0;
        //}

        // Check sendable
        if (!noDownstream)
        {
            if (!keepMode)
            {
                for (auto& chan : downstream)
                {
                    if (!chan->checkSend(this->channel.front(), this))
                    {
                        valid = 0;
                        break;
                    }
                }
            }

            //for (auto& chan : downstream)
            //{
            //    if (!chan->checkSend(this->channel.front(), this))
            //    {
            //        valid = 0;
            //        break;
            //    }
            //}
        }
    }

    if (valid)
    {
        channel.front().cycle = clk;  // Update cycle when pop data
        ++chanDataCnt;
    }

    bpUpdate();

    //if (speedup > 1)
    //{
    //    parallelize();
    //}
}

bool ChanBase::checkDataMatch()
{
    uint clk = ClkDomain::getInstance()->getClk();
    bool match = 1;

    for (auto& buffer : chanBuffer)
    {
        if (buffer.empty() || buffer.front().cycle > clk)
        {
            match = 0;
            break;
        }
    }

    return match;
}

//bool ChanBase::checkSend(Data _data, Channel* _upstream)
//{
//    bool sendable = 1;
//    uint bufferId = getChanId(_upstream);
//    //// Find corresponding bufferId of upstream
//    //for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
//    //{
//    //    if (upstream[chanId] == _upstream)
//    //    {
//    //        bufferId = chanId;
//    //        break;
//    //    }
//    //}
//
//    if (bp[bufferId] == 1)
//    {
//        sendable = 0;
//    }
//    return sendable;
//}

//void ChanBase::bpUpdate()
//{
//    //if (channel.size() < size)
//    //    bp = 0;
//    //else
//    //    bp = 1;
//
//    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
//    {
//        if (chanBuffer[bufferId].size() < size)
//        {
//            bp[bufferId] = 0;
//        }
//        else
//        {
//            bp[bufferId] = 1;
//        }
//    }
//}

//// Channel get data from the program variable 
//vector<int> ChanBase::get(vector<int> data)
//{
//    vector<int> pushState(2);
//    vector<int> popState(2);
//
//    checkConnect();
//    popState = pop(); // Data lifetime in nested loop
//
//    for (size_t i = 0; i < data.size(); ++i)
//    {
//        // Note: the sequence of data in data vector should consistent with corresponding upstream and buffer in chanBuffer
//        pushState = push(data[i], i);  
//    }
//
//    statusUpdate(); // Set valid according to the downstream channels' status
//    //bpUpdate();
//
//    return { pushState[0], pushState[1], popState[0], popState[1] };
//}
//
//// Assign channel value to program varieties
//int ChanBase::assign(uint bufferId)
//{
//    //if (!this->channel.empty())
//    //{
//    //    Data data = channel.front();
//    //    return data.value;
//    //}
//    //else
//    //    return lastPopVal;
//
//    if (!this->chanBuffer[bufferId].empty())
//    {
//        return chanBuffer[bufferId].front().value;
//    }
//    else
//        return lastPopVal[bufferId];
//}


// class ChanDGSF
ChanDGSF::ChanDGSF(uint _size, uint _cycle, uint _speedup)
    : ChanBase(_size, _cycle, _speedup)
{
    //enable = 1;
    //currId = 1; // Id begins at 1
    //sendActiveMode = 0;  // Default set to false

    pushBufferEnable = 0;
    popChannelEnable = 0;

    chanType = ChanType::Chan_DGSF;
}

ChanDGSF::~ChanDGSF()
{
    for (auto& chan : activeStream)
    {
        //delete chan;
        chan = nullptr;
    }
}

vector<int> ChanDGSF::push(int data, uint bufferId)
{
    // Push data in channel
    if (pushBufferEnable)
    {
        if (checkUpstream(bufferId))
        {
            // If in branch mode, only push data in the same condition
            if ((branchMode && upstream[bufferId]->channel.front().cond == channelCond) || !branchMode)
            {
                pushBuffer(data, bufferId);
                return { 1, data };
            }
        }
    }

    return { 0, data };
}

void ChanDGSF::pushBuffer(int data, uint _bufferId)
{
    // Push chanBuffer only when this buffer has not gotten the last data
    if (!getTheLastData[_bufferId])
    {
        Channel::pushBuffer(data, _bufferId);
        getTheLastData[_bufferId] = checkGetLastData(_bufferId);  // Update getTheLastData
    }
}

void ChanDGSF::bpUpdate()
{
    if (pushBufferEnable)
    {
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            if (chanBuffer[bufferId].size() < size)
            {
                bp[bufferId] = 0;
            }
            else
            {
                bp[bufferId] = 1;
            }
        }
    }
    else
    {
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            bp[bufferId] = 1;  // When pushBufferDisable, set all the bp to 1
        }
    }
}

bool ChanDGSF::checkGetLastData(uint bufferId)
{
    bool getLastData = 0;
    if (!chanBuffer[bufferId].empty())
    {
        if (chanBuffer[bufferId].back().lastOuter && chanBuffer[bufferId].back().last)
        {
            getLastData = 1;
        }
    }

    return getLastData;
}

//vector<int> ChanDGSF::popChannel(bool popReady, bool popLastReady)
//{
//    int popSuccess = 0;
//    int popData = 0;
//
//    if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
//    {
//        Data data = channel.front();
//        channel.pop_front();
//        popSuccess = 1;
//        //popData = data.value;
//        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
//        {
//            lastPopVal[bufferId] = chanBuffer[bufferId].front().value;  // For LC->loopVar, record last pop data when the channel pop empty
//            chanBuffer[bufferId].pop_front();
//        }
//        //lastPopVal = data.value;
//
//        //// Clear the last flags of downstreams
//        //if (keepMode)
//        //{
//        //    for (auto& channel : downstream)
//        //    {
//        //        // lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
//        //        if (/*!channel->isCond*/ !channel->isLoopVar)
//        //        {
//        //            channel->getLast.pop_front();
//        //        }
//        //        else
//        //        {
//        //            // If it is a loopVar, pop produceLast queue
//        //            if (!channel->produceLast.empty())
//        //            {
//        //                channel->produceLast.pop_front();
//        //            }
//        //        }
//        //    }
//        //}
//
//        // Clear the last tag of lastTagQueue
//        if (keepMode)
//        {
//            for (auto& queue : lastTagQueue)
//            {
//                queue.second.pop_front();  // Pop out a lastTag
//            }
//        }
//
//        // Active downstream and disable itself
//        if (data.graphSwitch == 1 && sendActiveMode == 1)
//        {
//            sendActive();
//            enable = 0;  // Disable current channel for graph switch
//        }
//    }
//
//    return { popSuccess , popData };
//}

//void ChanDGSF::statusUpdate()
//{
//    uint clk = ClkDomain::getInstance()->getClk();
//    // Set valid
//    valid = 1;
//
//    bool match = 1;
//    for (auto& buffer : chanBuffer)
//    {
//        if (buffer.empty() || buffer.front().cycle > clk)
//        {
//            match = 0;
//        }
//    }
//
//    if (match && channel.empty())  // Channel size is 1
//    {
//        pushChannel();
//    }
//
//    if (channel.empty() || !enable)
//    {
//        valid = 0;
//    }
//    else
//    {
//        ////uint clk = ClkDomain::getInstance()->getClk();
//        //Data data = channel.front();
//
//        //if (data.cycle > clk)
//        //{
//        //    valid = 0;
//        //}
//
//        // Check sendable
//        if (!noDownstream)
//        {
//            if (!keepMode)
//            {
//                for (auto& chan : downstream)
//                {
//                    if (!chan->checkSend(this->channel.front(), this))
//                    {
//                        valid = 0;
//                        break;
//                    }
//                }
//            }
//        }
//    }
//
//    if (valid)
//    {
//        channel.front().cycle = clk;  // Update cycle when pop data
//        ++chanDataCnt;
//    }
//
//    bpUpdate();
//
//    if (speedup > 1)
//    {
//        parallelize();
//    }
//
//    checkGraphSwitch();
//}

//void ChanDGSF::checkGraphSwitch()
//{
//    bool graphSwitch = 1;
//    for (size_t i = 0; i < chanBuffer.size(); ++i)
//    {
//        if (chanBufferDataCnt[i] - chanBufferDataCntLast[i] < size)
//        {
//            graphSwitch = 0;
//            break;
//        }
//    }
//}

//void ChanDGSF::sendActive()
//{
//    if (sendActiveMode)
//    {
//        if (activeStream.empty())
//        {
//            Debug::throwError("ActiveStream is empty!", __FILE__, __LINE__);
//        }
//        else
//        {
//            for (auto& channel : activeStream)
//            {
//                channel->enable = 1;
//            }
//        }
//    }
//}

//void ChanDGSF::parallelize()
//{
//    // Push clkStall in parallel execution mode
//    if (currId != speedup && !channel.empty() && channel.front().graphSwitch == 0)  // If the parallel execution dosen't finish, stall the system clock;
//    {
//        ClkDomain::getInstance()->addClkStall();
//    }
//    currId = currId % speedup + 1;
//}

//void ChanDGSF::statusUpdate()
//{
//    uint clk = ClkDomain::getInstance()->getClk();
//    // set valid
//    valid = 1;
//
//    if (channel.empty() || !enable)
//    {
//        valid = 0;
//    }
//    else
//    {
//        //uint clk = ClkDomain::getInstance()->getClk();
//        Data data = channel.front();
//
//        if (data.cycle > clk)
//        {
//            valid = 0;
//        }
//
//        // Check sendable
//        if (!noDownstream)
//        {
//            for (auto& channel : downstream)
//            {
//                if (!channel->checkSend(data, this))
//                {
//                    valid = 0;
//                    break;
//                }
//            }
//        }
//    }
//
//    if (valid)
//    {
//        channel.front().cycle = clk;  // Update cycle when pop data
//    }
//
//    //if (!noDownstream)
//    //{
//    //    for (auto channel : downstream)
//    //    {
//    //        if (channel->bp)  // _modify 2.27
//    //        {
//    //            valid = 0;
//    //            break;
//    //        }
//    //    }
//    //}
//
//    //if (!noDownstream)
//    //{
//    //    for (auto& channel : downstream)
//    //    {
//    //        if (!channel->checkSend(Data(), this))
//    //        {
//    //            valid = 0;
//    //            break;
//    //        }
//    //    }
//    //}
//
//    bpUpdate();
//
//    //// Push clkStall in parallel execution mode
//    //if (currId != speedup && !channel.empty() && channel.front().graphSwitch == 0)  // If the parallel execution dosen't finish, stall the system clock;
//    //{
//    //    ClkDomain::getInstance()->addClkStall();
//    //}
//    //currId = currId % speedup + 1;
//
//    if (speedup > 1)
//    {
//        parallelize();
//    }
//
//}


// class ChanSGMF
ChanSGMF::ChanSGMF(uint _size, uint _cycle) : 
    ChanBase(_size, _cycle)/*, chanSize(_size)*/
{
    init();
}

//ChanSGMF::ChanSGMF(uint _size, uint _cycle, uint _bundleSize) : 
//    ChanBase(_size, _cycle)/*, chanSize(_size)*//*, chanBundleSize(_bundleSize)*/
//{
//    init();
//}

ChanSGMF::~ChanSGMF()
{
    //for (auto& upstream : upstreamBundle)
    //{
    //    for (auto& chan : upstream)
    //    {
    //        //delete chan;
    //        chan = nullptr;
    //    }
    //}

    for (auto& chan : upstream)
    {
        chan = nullptr;
    }
}

void ChanSGMF::init()
{
    chanType = ChanType::Chan_SGMF;

    //chanBundle.resize(chanBundleSize);  // Default channel number(chanBundleSize) = 2;
    //upstreamBundle.resize(chanBundleSize);
    //downstream.resize(chanBundleSize);

    //for (auto& chan : chanBundle)
    //{
    //    //chan.resize(chanSize);
    //    chan.resize(chanSize * std::max(cycle, static_cast<uint>(1)));  // Avoid tag conflict stall in multi-cycle channel
    //}

    //matchQueue.resize(chanSize);
}

//void ChanSGMF::checkConnect()
//{
//    bool upstreamEmpty = 0;
//    for (auto& upstream : upstreamBundle)
//    {
//        upstreamEmpty |= upstream.empty();
//    }
//
//    if ((!noUpstream && upstreamEmpty) || (!noDownstream && downstream.empty()))
//    {
//        Debug::throwError("Upstream/Downstream is empty!", __FILE__, __LINE__);
//    }
//}

//void ChanSGMF::addUpstream(const vector<vector<Channel*>>& _upstreamBundle)
//{
//    for (size_t i = 0; i < _upstreamBundle.size(); ++i)
//    {
//        for (auto& chan : _upstreamBundle[i])
//        {
//            upstreamBundle[i].push_back(chan);
//        }
//    }
//}

//// addUpstream for single channel chanSGMF
//void ChanSGMF::addUpstream(const vector<Channel*>& _upstream)
//{
//    if (chanBundle.size() != 1)
//    {
//        Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
//    }
//    else
//    {
//        for (auto& chan : _upstream)
//        {
//            upstreamBundle[0].push_back(chan);
//            upstream.push_back(chan);  // For single channel
//        }
//    }
//}

//void ChanSGMF::addDownstream(const vector<vector<ChanSGMF*>>& _downstreamBundle)
//{
//    for (size_t i = 0; i < _downstreamBundle.size(); ++i)
//    {
//        for (auto& chan : _downstreamBundle[i])
//        {
//            downstreamBundle[i].push_back(chan);
//        }
//    }
//}
void ChanSGMF::pushBuffer(int _data, uint _bufferId, uint _tag)
{
    Data data;
    data.valid = 1;
    if (!noUpstream)
    {
        uint upstreamId = _bufferId;
        Data data = upstream[upstreamId]->channel.front();
    }
    data.value = _data;
    //data.cycle = ClkDomain::getInstance()->getClk() + cycle;  // Update cycle when push into chanBuffer
    if (branchMode)
    {
        if (data.cond == channelCond)
        {
            data.cycle = ClkDomain::getInstance()->getClk() + cycle;
        }
        else
        {
            data.cycle = ClkDomain::getInstance()->getClk();  // If in the falsePath, no need to update cycle
        }
    }
    else
    {
        data.cycle = ClkDomain::getInstance()->getClk() + cycle;
    }

    // Tag 
    uint tag = 0;
    if (!noUpstream)
    {
        if (!upstream[_bufferId]->keepMode)
        {
            tag = upstream[_bufferId]->channel.front().tag;
        }
    }
    else
    {
        tag = _tag;  // For no upstream, tag = _tag;
    }
    /*bool tagBind = 0;
    for (auto& chan : upstream)
    {
        if (!chan->keepMode)
        {
            if (!chan->channel.empty())
            {
                tag = chan->channel.front().tag;
                tagBind = 1;
                break;
            }
        }
    }
    if (!tagBind)
    {
        Debug::throwError("Need more upstream channels except the one in keepMode!", __FILE__, __LINE__);
    }*/

    if (tagUpdateMode)
    {
        tag = (tag + 1) % tagSize;  // Update tag in tagUpdateMode
        //data.tag = tag;
    }

    data.tag = tag;

    if (!noUpstream && upstream[_bufferId]->keepMode)
    {
        if (!chanBuffer[_bufferId][0].valid)
        {
            chanBuffer[_bufferId][0] = data;  // If upstream is keepMode, only store data in the [0]; Only store once!
            ++chanBufferDataCnt[_bufferId];
        }
    }
    else
    {
        chanBuffer[_bufferId][tag] = data;  // chanBuffer index 0 ~ #tag to store received data
        ++chanBufferDataCnt[_bufferId];
    }
}

//vector<int> ChanSGMF::get(vector<int> data)
//{
//    //vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
//    vector<int> pushState(chanBuffer.size());  // Push into Din1 and Din2
//    vector<int> popState(2);
//
//    checkConnect();
//    popState = pop(); // Data lifetime in nested loop
//    
//    for (size_t i = 0; i < chanBuffer.size(); ++i)
//    {
//        vector<int> tmp(2);
//        //tmp = push(data[i], i, 0);  // If no upstream, default tag = 0
//        push(data[i], i, 0);
//        pushState[i] = tmp[0];
//        pushState[i + 1] = tmp[1];
//    }
//    
//    statusUpdate(); // Set valid according to the downstream channels' status
//    //bpUpdate();
//
//    pushState.insert(pushState.end(), popState.begin(), popState.end());
//    return pushState;
//}

vector<int> ChanSGMF::get()
{
    //vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
    vector<int> pushState(chanBuffer.size());  // Push into Din1 and Din2
    vector<int> popState(2);

    //checkConnect();
    popState = pop(); // Data lifetime in nested loop

    for (size_t i = 0; i < upstream.size(); ++i)
    {
        vector<int> tmp(2);
        //tmp = push(data[i], i, 0);  // If no upstream, default tag = 0
        tmp = push(upstream[i]->value, i, 0);
        //pushState[i] = tmp[0];
        //pushState[i + 1] = tmp[1];
    }

    statusUpdate(); // Set valid according to the downstream channels' status
    //bpUpdate();

    pushState.insert(pushState.end(), popState.begin(), popState.end());
    return pushState;
}

//// For single channel
//vector<int> ChanSGMF::get(int data)
//{
//    if (chanBundleSize != 1)
//    {
//        Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
//    }
//
//    vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
//    vector<int> popState(2);
//
//    checkConnect();
//    popState = pop(); // Data lifetime in nested loop
//    pushState = push(data, 0, 0);  // For single channel, chanId = 0 defaultly; If no upstream, tag = 0 defaultly;
//    statusUpdate(); // Set valid according to the downstream channels' status
//    //bpUpdate();
//
//    pushState.insert(pushState.end(), popState.begin(), popState.end());
//    return pushState;
//}

// For no upstream channel
vector<int> ChanSGMF::get(vector<int> data, uint tag)
{
    //if (chanBundleSize != 1)
    //{
    //    Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
    //}

    if (!noUpstream)
    {
        Debug::throwError("It is not a no upstream channel!", __FILE__, __LINE__);
    }

    vector<int> pushState(chanBuffer.size() * 2);  // Push into Din1 and Din2
    vector<int> popState(2);

    //checkConnect();
    popState = pop(); // Data lifetime in nested loop

    for (size_t i = 0; i < chanBuffer.size(); ++i)
    {
        vector<int> tmp(2);
        //tmp = push(data[i], i, 0);  // If no upstream, default tag = 0
        push(data[i], i, tag);
        pushState[i] = tmp[0];
        pushState[i + 1] = tmp[1];
    }

    statusUpdate(); // Set valid according to the downstream channels' status
    //bpUpdate();

    pushState.insert(pushState.end(), popState.begin(), popState.end());
    return pushState;
}

vector<int> ChanSGMF::popChannel(bool popReady, bool popLastReady)
{
    int popSuccess = 0;
    int popData = 0;

    if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
    {
        //Data data = channel.front();
        uint tag = channel.front().tag;
        uint round = size / tagSize;
        uint addr = (round - 1) * tagSize + tag;

        channel.pop_front();  // Pop channel(sendQueue)
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            if (!noUpstream && upstream[bufferId]->keepMode)
            {
                chanBuffer[bufferId][0].valid = 0;
                lastPopVal[bufferId] = chanBuffer[bufferId][0].value;
            }
            else
            {
                chanBuffer[bufferId][addr].valid = 0;  // Pop corrsponding data in each channel, set valid to 0
                lastPopVal[bufferId] = chanBuffer[bufferId][addr].value;  // For LC->cond, record last pop data when the channel pop empty
            }
        }

        //for (auto& data : matchQueue)
        //{
        //    if (data.tag == tag)  // Pop matchQueue
        //    {
        //        data.valid = 0;
        //    }
        //}

        for (auto ptr = matchQueue.begin(); ptr != matchQueue.end();/* ++ptr*/)
        {
            if (ptr->tag == tag)
            {
                ptr = matchQueue.erase(ptr);  // Pop matchQueue
            }
            else
            {
                ++ptr;
            }
        }

        popSuccess = 1;
        //popData = data.value;
        //lastPopVal = data.value;  // For LC->cond, record last pop data when the channel pop empty

        //// Clear the last flags of downstreams
        //if (keepMode)
        //{
        //    for (auto& channel : downstream)
        //    {
        //        // lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
        //        if (/*!channel->isCond*/ !channel->isLoopVar)
        //        {
        //            channel->getLast.pop_front();
        //        }
        //        else
        //        {
        //            // If it is a loopVar, pop produceLast queue
        //            if (!channel->produceLast.empty())
        //            {
        //                channel->produceLast.pop_front();
        //            }
        //        }
        //    }
        //}

        // Clear the last tag of lastTagQueue
        if (keepMode)
        {
            for (auto& queue : lastTagQueue)
            {
                queue.second.pop_front();  // Pop out a lastTag
            }
        }
    }

    return { popSuccess , popData };
}

//void ChanSGMF::updateCycle(bool popReady, bool popLastReady)
//{
//    if (popReady && keepMode && !popLastReady)  // Update data cycle in keepMode
//    {
//        for (auto& chan : chanBundle)  // Update data cycle in channel
//        {
//            for (auto& data : chan)
//            {
//                ++data.cycle;
//            }
//        }
//
//        for (auto& data : matchQueue)  // Update data cycle in popFifo
//        {
//            ++data.cycle;
//        }
//
//        for (auto& data : channel)  // Update data cycle in channel(sendQueue)
//        {
//            ++data.cycle;
//        }
//    }
//}

vector<int> ChanSGMF::push(int data, uint chanId, uint tag)
{
    // Push data in channel
    if (checkUpstream(chanId, tag))
    {
        pushBuffer(data, chanId, tag);
        return { 1, data };
    }
    else
        return { 0, data };
}

bool ChanSGMF::checkUpstream(uint bufferId, uint tag)
{
    bool ready = 1;
    if (!noUpstream)
    {
        uint chanId = bufferId;
        if (!upstream[chanId]->valid)
        {
            ready = 0;
        }
        else if (drainMode)
        {
            // In drainMode, outer-loop channels only receive the last data of the inner-loop; 
            // Other data will be drained by the outer-loop channel
            if (!upstream[chanId]->channel.front().last)
            {
                ready = 0;
            }
        }
    }
    else
    {
        if (chanBuffer[bufferId][tag].valid)  // When corresponding tag is not empty
            ready = 0;
    }

    return ready;
}

void ChanSGMF::pushChannel(uint tag)
{
    if (!noUpstream)
    {
        Data data;
        data.valid = 1;
        data.tag = tag;
        //data.value = _data;
        //data.cycle = data.cycle + cycle;
        //uint cycleTemp = data.cycle + cycle;
        //data.cycle = cycleTemp > clk ? cycleTemp : clk;

        //// Bind tags for data; (Ignore the tag of upstream channel in keepMode)
        //uint tag;
        //bool tagBind = 0;
        //for (auto& chan : upstreamBundle[chanId])
        //{
        //    if (!chan->keepMode)
        //    {
        //        tag = data.tag;
        //        tagBind = 1;
        //        break;
        //    }
        //}
        //if (!tagBind)
        //{
        //    Debug::throwError("Need more upstream channels except the one in keepMode!", __FILE__, __LINE__);
        //}

        //if (tagUpdateMode)
        //{
        //    tag = (tag + 1) % chanSize;  // Update tag in tagUpdateMode
        //    data.tag = tag;
        //}

        //// loopVar not receive last, only receive lastOuter
        //// Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
        //for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
        //{
        //    //if (/*!isCond*/ !isLoopVar && channel->keepMode == 0)
        //    //{
        //    //    data.last |= channel->channel.front().last;
        //    //}

        //    if (upstream[chanId]->keepMode == 0)
        //    {
        //        data.last |= channel->channel.front().last;
        //    }
        //    data.lastOuter |= channel->channel.front().lastOuter;
        //    //data.graphSwitch |= channel->channel.front().graphSwitch;
        //}

        // Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            // Due to a channel in keepMode may repeatly send a data with a last for many times
            // The data received by a drainMode channel is always with a last flag, so ignore it
            if (upstream[bufferId]->keepMode == 0 && this->drainMode == 0)  // upstreamId is equal to bufferId
            {
                data.last |= chanBuffer[bufferId][tag].last;
                data.lastOuter |= chanBuffer[bufferId][tag].lastOuter;
            }
            else
            {
                //if (chanBuffer[bufferId].front().last && isLoopVar)
                //{
                //    data.lastOuter = 1;  // Signify inner loop has received a last from outer loop
                //}

                if (chanBuffer[bufferId].front().last && chanBuffer[bufferId].front().lastOuter && isLoopVar)
                {
                    data.lastOuter = 1;  // Signify inner loop has received a last from outer loop
                }

                //data.lastOuter |= chanBuffer[bufferId][0].lastOuter;  // If a buffer's corresponding upstream is in keepMode, only store its data in [0]
            }
            //data.graphSwitch |= chanBuffer[bufferId][tag].graphSwitch;
        }

        //// Push getLast
        //if (data.last)
        //    getLast.push_back(1);
        //if (!keepMode && data.last)
        //{
        //    getLast.push_back(1);
        //}

        // Send lastTag to each upstream channel in keepMode
        if (data.last)
        {
            //if (isLoopVar)
            //{
            //    data.last = 0;  //Reset data last flag, due to loopVar not receive last, only receive lastOuter
            //    getLast.push_back(1);
            //}
            //else
            //{
            //    sendLastTag();
            //}

            sendLastTag();
        }

        //if (branchMode)
        //{
        //    if (!upstream.front()->isCond)
        //    {
        //        // The cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
        //        Debug::throwError("The cond channel must be in the first element of the upstream vector!", __FILE__, __LINE__);
        //    }
        //    else if (data.cond == channelCond)
        //    {
        //        chanBundle[chanId][tag] = data;
        //    }
        //}
        //else
        //{
        //    chanBundle[chanId][tag] = data;
        //}

        bool getIsCond = 0;  // Signify has got a cond
        if (branchMode)
        {
            // Update data cond when a upstream is in isCond mode
            for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
            {
                if (upstream[chanId]->isCond)
                {
                    if (getIsCond)
                    {
                        if (data.cond != chanBuffer[chanId][tag].cond)
                        {
                            Debug::throwError("The cond flag of each iscond channel is not consistent!", __FILE__, __LINE__);
                        }
                    }
                    else
                    {
                        data.cond = chanBuffer[chanId][tag].cond;
                        getIsCond = 1;
                    }
                }
            }

            //if (data.cond == channelCond)
            //{
            //    channel.push_back(data);
            //}
            //else
            //{
            //    // Clear data in the chanBuffer directly
            //    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
            //    {
            //        if (!upstream[bufferId]->keepMode)
            //        {
            //            uint round = size / tagSize;
            //            uint addr = (round - 1) * tagSize + tag;
            //            chanBuffer[bufferId][addr].valid = 0;  // Clear data
            //        }
            //        else
            //        {
            //            chanBuffer[bufferId][0].valid = 0;
            //        }
            //    }
            //}

            //if (!upstream.front()->isCond)
            //{
            //    // The cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
            //    Debug::throwError("The cond channel must be in the first element of the upstream vector!", __FILE__, __LINE__);
            //}
            //else if (data.cond == channelCond)
            //{
            //    channel.push_back(data);
            //}
        }
        //else
        //{
        //    channel.push_back(data);
        //}

        // Inherit data cond from upstream channel
        if (!getIsCond)
        {
            if (!upstream[0]->isCond)
            {
                bool getCond = 0;
                for (size_t chanId = 0; chanId < upstream.size(); ++chanId)
                {
                    if (getCond)
                    {
                        if (data.cond != chanBuffer[chanId][tag].cond)
                        {
                            Debug::throwError("The cond flag of each upstream channel is not consistent!", __FILE__, __LINE__);
                        }
                    }
                    else
                    {
                        data.cond = chanBuffer[chanId][tag].cond;
                        getCond = 1;
                    }
                }
            }
            else
            {
                data.cond = chanBuffer[0][tag].cond;
            }
        }

        channel.push_back(data);
    }
    else
    {
        Data data = Data();
        data.tag = tag;
        data.valid = 1;
        //data.value = _data;
        data.cycle = ClkDomain::getInstance()->getClk();
        channel.push_back(data);  
        //for (size_t i = _tag; i < chanBundle[chanId].size(); i = i + chanSize)
        //{
        //    if (!chanBundle[chanId][i].valid)
        //    {
        //        //data.chanBundleAddr = i;
        //        chanBundle[chanId][i] = data;
        //        break;
        //    }
        //}
    }
}

void ChanSGMF::statusUpdate()
{
    // Reset valid
    valid = 0;

    // Shift data in multi-cycle chanBuffer
    shiftDataInChanBuffer();
    // Check tag match among channels, and send match ready data into matchQueue
    checkTagMatch();
    
    uint clk = ClkDomain::getInstance()->getClk();
    // Check whether downstream channel avaliable to receive data
    if (!noDownstream && channel.empty())
    {
        for (auto& data : matchQueue)
        {
            if (data.cycle <= clk)
            {
                bool sendable = 1;
                // Regard the data in keepMode channel as a constant, always can be send out if the channel is valid;
                //if (keepMode)
                //{
                //    if (!channel.empty())  // Only when the channel is empty, a new data can be pushed in it
                //    {
                //        sendable = 0;
                //    }
                //}
                if(!keepMode)
                {
                    for (auto& channel : downstream)
                    {
                        if (!channel->checkSend(data, this))  // Must guarantee "this" points to current upstream channel!
                        {
                            sendable = 0;
                            break;
                        }
                    }
                }

                if (sendable)
                {
                    //channel.push_back(data);  // Push data sendable into channel(sendQueue)
                    pushChannel(data.tag);  // Only when channel is empty still execute current "for loop", no worry push a data in channel many times
                    //valid = 1;  // Set valid = 1, signify the data in channel(sendQueue) is valid for all the downstream
                    break;  // Only one data can be pushed in each cycle
                }
            }
        }
    }

    if (!channel.empty())
    {
        valid = 1;
        channel.front().cycle = clk;  // Update cycle when pop data
        ++chanDataCnt;
    }

    //bpUpdate();
}

void ChanSGMF::shiftDataInChanBuffer()
{
    // Shift data in multi-cycle chanBuffer
    uint round = size / tagSize;
    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)  // Traverse each buffernel in bufferBundle
    {
        // Only shift buffer whose corresponding upstream is not in keepMode! or not has any upstream
        if (noUpstream || (!noUpstream && !upstream[bufferId]->keepMode))
        {
            for (size_t tag = 0; tag < tagSize; ++tag)  // Traverse tag
            {
                for (uint i = 0; i < round - 1; ++i)  // Shift from back to front in chanBuffer
                {
                    uint addrBase = (round - 2 - i) * tagSize;
                    if (!chanBuffer[bufferId][addrBase + tagSize + tag].valid)  // Next round addr
                    {
                        if (chanBuffer[bufferId][addrBase + tag].valid)
                        {
                            chanBuffer[bufferId][addrBase + tagSize + tag] = chanBuffer[bufferId][addrBase + tag];  // Shift data
                            chanBuffer[bufferId][addrBase + tag].valid = 0;  // Clear after shift
                        }
                    }
                }
            }
        }
    }
}

void ChanSGMF::checkTagMatch()
{
    uint round = size / tagSize;
    // Check tag match among channels
    for (size_t tag = 0; tag < tagSize; ++tag)
    {
        //uint tag = i;
        Data data;
        data.valid = 1;
        bool match = 1;
        //uint addr = (std::max(cycle, uint(1)) - 1) * chanSize + i;  // For multi-cycle channel, only check the bottom section's tag
        uint addr = (round - 1) * tagSize + tag;  // For multi-cycle channel, only check the bottom section's tag

        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            if (!noUpstream && upstream[bufferId]->keepMode)
            {
                if (!chanBuffer[bufferId][0].valid)
                {
                    match = 0;
                    break;
                }

                data.cycle = std::max(data.cycle, chanBuffer[bufferId][0].cycle);  // Update the cycle as the max among each channel
            }
            else
            {
                if (!chanBuffer[bufferId][addr].valid)
                {
                    match = 0;
                    break;
                }

                data.cycle = std::max(data.cycle, chanBuffer[bufferId][addr].cycle);  // Update the cycle as the max among each channel
            }

            //data.cycle = std::max(data.cycle, chanBuffer[bufferId][addr].cycle);  // Update the cycle as the max among each channel

            //else
            //{
            //    //// loopVar not receive last, only receive lastOuter
            //    //// Due to a buffernel in keepMode may repeatly send a data with a last for many times
            //    //if (!isLoopVar && keepMode == 0)
            //    //{
            //    //    data.last |= buffer[addr].last;
            //    //}
            //    //data.lastOuter |= buffer[addr].lastOuter;

            //    data.cycle = std::max(data.cycle, buffer[addr].cycle);  // Update the cycle as the max among each channel
            //}
        }

        if (match)
        {
            //data.valid = 1;
            data.tag = tag;
            bool tagConflict = 0;

            // Avoid a same data being pushed into matchQueue twice
            for (auto& _data : matchQueue)
            {
                if (_data.tag == tag)
                {
                    tagConflict = 1;
                    break;
                }
            }

            if (!tagConflict)
            {
                matchQueue.push_back(data);
            }
        }
    }
}

bool ChanSGMF::checkSend(Data _data, Channel* _upstream)
{
    uint tag = _data.tag;
    bool sendable = 0;
    //bool upstreamMatch = 0;
    int chanId = getChanId(_upstream);

    if (chanId != -1)
    {
        if (!bp[chanId])  // In ChanSGMF, bp is only updated by downstream if needed.(e.g. in trueChan & falseChan in Mux)
        {
            if (tagUpdateMode)
            {
                tag = (tag + 1) % tagSize;  // Update tag in tagUpdateMode
            }

            // If this tag's data is invalid, or this tag's data will be poped out in the next cycle, it is sendable;
            if (!chanBuffer[chanId][tag].valid || (!keepMode && valid && channel.front().tag == tag))
            {
                sendable = 1;
            }
        }
        else
        {
            sendable = 0;
        }
    }

    //// Search current upstream is in which upstreamBundle
    //for (size_t i = 0; i < upstream.size(); ++i)
    //{
    //    if (upstream[i] == _upstream)
    //    {
    //        upstreamMatch = 1;
    //        chanId = i;
    //        break;
    //    }

    //    //for (auto& chan : upstream[i])
    //    //{
    //    //    if (chan == _upstream)
    //    //    {
    //    //        upstreamMatch = 1;
    //    //        chanId = i;
    //    //        break;
    //    //    }
    //    //}

    //    // Check same tag conflict
    //    if (upstreamMatch)
    //    {
    //        if (!bp[chanId])  // In ChanSGMF, bp is only updated by downstream if needed.(e.g. in trueChan & falseChan in Mux)
    //        {
    //            if (tagUpdateMode)
    //            {
    //                tag = (tag + 1) % tagSize;  // Update tag in tagUpdateMode
    //            }

    //            if (!chanBuffer[chanId][tag].valid)
    //            {
    //                sendable = 1;
    //            }
    //        }
    //        else
    //        {
    //            sendable = 0;
    //        }

    //        break;
    //    }
    //}

    return sendable;
}

//void ChanSGMF::sendLastTag()
//{
//    for (auto& chan : upstream)
//    {
//        if (chan->keepMode)
//        {
//            for (auto& queue : chan->lastTagQueue)
//            {
//                if (queue.first == this->moduleId)
//                {
//                    queue.second.push_back(1);
//                }
//            }
//        }
//        /*for (auto& chan : upstream)
//        {
//            if (chan->keepMode)
//            {
//                for (auto& queue : chan->lastTagQueue)
//                {
//                    if (queue.first == this->moduleId)
//                    {
//                        queue.second.push_back(1);
//                    }
//                }
//            }
//        }*/
//    }
//}

// In SGMF mode, interface func assign is to get value of corresponding channel by chanId (e.g. Din1, Din2)
int ChanSGMF::assign(uint bufferId)
{
    if (!channel.empty())
    {
        if (!upstream[bufferId]->keepMode)
        {
            uint tag = channel.front().tag;
            uint round = size / tagSize;
            uint addr = (round - 1) * tagSize + tag;
            return chanBuffer[bufferId][addr].value;
        }
        else
        {
            return chanBuffer[bufferId][0].value;
        }
    }
    else
    {
        //return MAX;  // Indicate current value is invalid
        return lastPopVal[bufferId];
    }
}

int ChanSGMF::assign(Channel* chan)
{
    uint bufferId = getChanId(chan);

    return this->assign(bufferId);  
}


// ChanPartialMux
ChanPartialMux::ChanPartialMux(uint _size, uint _cycle) :
    ChanBase(_size, _cycle)
{
    initial();
}

ChanPartialMux::ChanPartialMux(uint _size, uint _cycle, uint _speedup) :
    ChanBase(_size, _cycle, _speedup)
{
    initial();
}

void ChanPartialMux::initial()
{
    chanType = ChanType::Chan_PartialMux;
}

bool ChanPartialMux::checkDataMatch()
{
    uint clk = ClkDomain::getInstance()->getClk();
    bool match = 1;
    
    if (!chanBuffer[0].empty())
    {
        bool cond = chanBuffer[0].front().cond;
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            // Select corresponding chanBuffer which upstream is in the same condition
            if ((upstream[bufferId]->branchMode && upstream[bufferId]->channelCond == cond) || !upstream[bufferId]->branchMode)
            {
                if (chanBuffer[bufferId].empty() || chanBuffer[bufferId].front().cycle > clk)
                {
                    match = 0;
                    break;
                }
            }
        }
    }
    else
    {
        match = 0;
    }

    return match;
}

vector<int> ChanPartialMux::popChannel(bool popReady, bool popLastReady)
{
    int popSuccess = 0;
    int popData = 0;

    if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
    {
        bool cond = channel.front().cond;
        channel.pop_front();
        popSuccess = 1;
        for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
        {
            // Select corresponding chanBuffer which upstream is in the same condition
            if ((upstream[bufferId]->branchMode && upstream[bufferId]->channelCond == cond) || !upstream[bufferId]->branchMode)
            {
                lastPopVal[bufferId] = chanBuffer[bufferId].front().value;  // For LC->loopVar, record last pop data when the channel pop empty
                chanBuffer[bufferId].pop_front();         
            }
        }

        // Clear the last tag of lastTagQueue
        if (keepMode)
        {
            for (auto& queue : lastTagQueue)
            {
                queue.second.pop_front();  // Pop out a lastTag
            }
        }
    }

    return { popSuccess , popData };
}

void ChanPartialMux::updateDataStatus(Data& data)
{
    bool cond = chanBuffer[0].front().cond;  // chanBuffer[0] must in isCond mode

    for (size_t bufferId = 0; bufferId < chanBuffer.size(); ++bufferId)
    {
        // Select corresponding chanBuffer which upstream is in the same condition
        if ((upstream[bufferId]->branchMode && upstream[bufferId]->channelCond == cond) || !upstream[bufferId]->branchMode)
        {
            // Due to a channel in keepMode may repeatly send a data with a last for many times
            // The data received by a drainMode channel is always with a last flag, so ignore it
            if (upstream[bufferId]->keepMode == 0 && this->drainMode == 0)  // upstreamId is equal to bufferId
            {
                data.last |= chanBuffer[bufferId].front().last;
                data.lastOuter |= chanBuffer[bufferId].front().lastOuter;
            }
            else
            {
                if (chanBuffer[bufferId].front().last && chanBuffer[bufferId].front().lastOuter && isLoopVar)
                {
                    data.lastOuter = 1;  // Signify inner loop has received a last from outer loop
                }
            }
        }

        //data.graphSwitch |= chanBuffer[bufferId].front().graphSwitch;
    }
}