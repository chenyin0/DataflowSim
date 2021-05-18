#include "./Lse.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"

using namespace DFSim;

//Lse::Lse(uint _size, uint _cycle, MemSystem* _memSys) : 
//    Channel(_size, _cycle), memSys(_memSys)
//{
//    //reqQueue.resize(_size);
//    //lseId = _memSys->registerLse(this);  // Register Lse in MemSystem and return lseId
//
//    initial();
//}
//
//Lse::Lse(uint _size, uint _cycle, MemSystem* _memSys, uint _speedup) :
//    Channel(_size, _cycle, _speedup), memSys(_memSys)
//{
//    //reqQueue.resize(_size);
//    //lseId = _memSys->registerLse(this);  // Register Lse in MemSystem and return lseId
//
//    initial();
//}

Lse::Lse(uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys) :
    Channel(_size, _cycle), isWrite(_isWrite), memSys(_memSys)
{
    //reqQueue.resize(_size);
    //lseId = _memSys->registerLse(this);  // Register Lse in MemSystem and return lseId

    initial();
}

Lse::Lse(uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys, uint _speedup) :
    Channel(_size, _cycle, _speedup), isWrite(_isWrite), memSys(_memSys)
{
    //reqQueue.resize(_size);
    //lseId = _memSys->registerLse(this);  // Register Lse in MemSystem and return lseId

    initial();
}

Lse::Lse(string _moduleName, uint _size, uint _cycle, bool _isWrite, MemSystem* _memSys, uint _speedup)
    : Channel(_moduleName, _size, _cycle, _speedup), isWrite(_isWrite), memSys(_memSys)
{
    initial();
}

Lse::~Lse()
{
    //if (memSys != nullptr)
    //{
    //    delete memSys;
    //}
    //memSys = nullptr;  // Destruct memorySystem in the destructor of class MemSystem

    delete memorySpace;
}

void Lse::initial()
{
    reqQueue.resize(size);
    lseId = memSys->registerLse(this);  // Register Lse in MemSystem and return lseId
    suspendReq.first = 0;  // Reset valid

    chanType = ChanType::Chan_Lse;
}

//void Lse::get(bool _isWrite, uint _addr)
//{
//    checkConnect();
//    pop();
//    push(_isWrite, _addr);
//    statusUpdate();
//}

//void Lse::get(vector<int> data, uint& trig_cnt)
//{
//    checkConnect();
//    pop();
//
//    // Only when trigger valid, push channel
//    // Must ensure the valid signal arrives earlier than the data, or else the data will be flushed 
//    if (trig_cnt > 0) 
//    {
//        if (push(_isWrite, _addr))
//        {
//            --trig_cnt;  // Decrease trig_cnt
//        }
//    }
//
//    statusUpdate();
//}

vector<int> Lse::pop()
{
    bool popReady = valid;
    bool popSuccess = 0;
    int popAddr = 0;

    if (popReady)
    {
        uint reqQueueIndex = channel.front().lseReqQueueIndex;
        lastPopVal = reqQueue[reqQueueIndex].first.addr;
        channel.pop_front();  // Pop channel
        if (isWrite)
        {
            valueQueue.pop_front();
        }
        reqQueue[reqQueueIndex].first.valid = 0;  // Pop reqQueue
        reqQueue[reqQueueIndex].second.valid = 0;

        popSuccess = 1;
        popAddr = reqQueue[reqQueueIndex].first.addr;
    }

    return { popSuccess, popAddr };
}

//bool Lse::checkUpstream()
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
//        if (reqQueue.size() == size)  // When reqQueue is full
//            ready = 0;
//    }
//
//    return ready;
//}

// Pop chanBuffer after push reqQueue, avoid to push a req into reqQueue many times
void Lse::popChanBuffer()
{
    for (auto& buffer : chanBuffer)
    {
        buffer.pop_front();
    }
}

void Lse::pushReqQ(/*bool _isWrite, uint _addr*/)  // chanBuffer[0] must store addr!!!
{
    //uint clk = ClkDomain::getInstance()->getClk();
    //Data data = upstream.front()->channel.front();  // Inherit cond status
    Data data = chanBuffer[0].front();  // Inherit cond status
    data.valid = 1;
    data.last = 0;  // Reset last flag
    data.cycle = ClkDomain::getClk();  // Update cycle to current clk for measuring memory access latency
    /*uint cycleTemp = data.cycle + cycle;
    data.cycle = cycleTemp > clk ? cycleTemp : clk;*/

    // Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag;
    for (auto buffer : chanBuffer)
    {
        data.last |= buffer.front().last;
        data.lastOuter |= buffer.front().lastOuter;
        data.graphSwitch |= buffer.front().graphSwitch;
    }

    // Send lastTag to each upstream channel in keepMode
    if (data.last)
    {
        sendLastTag();
    }

    MemReq req;
    req.valid = 1;
    //req.addr = chanBuffer[0].front().value;  // Address must stored in chanBuffer[0]!!!
    req.addr = chanBuffer[0].front().value + baseAddr;  // Address must stored in chanBuffer[0]!!!
    req.isWrite = isWrite;
    req.lseId = lseId;

    //for (size_t i = 0; i < reqQueue.size(); ++i)
    //{
    //    pushQueuePtr = (pushQueuePtr++) % reqQueue.size();
    //    if (!reqQueue[pushQueuePtr].first.valid)
    //    {
    //        req.lseReqQueueIndex = pushQueuePtr;  // Record reqQueue Id, for callback from memory
    //        req.cnt = orderId;
    //        ++orderId;
    //        data.lseReqQueueIndex = pushQueuePtr;  // Record reqQueue Id, for delete from channel
    //        reqQueue[pushQueuePtr].first = req;
    //        reqQueue[pushQueuePtr].second = data;

    //        popChanBuffer();
    //        break;
    //    }
    //}

    if (!reqQueue[pushQueuePtr].first.valid)
    {
        req.lseReqQueueIndex = pushQueuePtr;  // Record reqQueue Id, for callback from memory
        req.cnt = orderId;
        ++orderId;
        data.lseReqQueueIndex = pushQueuePtr;  // Record reqQueue Id, for delete from channel
        reqQueue[pushQueuePtr].first = req;
        reqQueue[pushQueuePtr].second = data;

        if (isWrite)
        {
            valueQueue.push_back(chanBuffer[1].front().value);  // Preserve value in Store mode
        }
        popChanBuffer();
        pushQueuePtr = (++pushQueuePtr) % reqQueue.size();
    }
}

//bool Lse::push(bool _isWrite, uint _addr)
//{
//    // Push data in channel
//    if (checkUpstream())
//    {
//        pushReqQ(_isWrite, _addr);
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

//bool Lse::sendReq(MemReq _req)
//{
//    // No memory (memory latency = 0)
//    if (NO_MEMORY || noLatencyMode)
//    {
//        ackCallback(_req);  // Send back ack directly(Not send reqs to memSys)
//        return true;
//    }
//    else
//    {
//        memReqCnt++;
//
//        if (memSys->addTransaction(_req))
//        {
//            uint index = _req.lseReqQueueIndex;
//            reqQueue[index].first.inflight = 1;  // Req has been sent to memory
//
//            return true;
//        }
//        else
//        {
//            memReqBlockCnt++;
//            return false;
//        }
//    }
//}

void Lse::statusUpdate()
{
    uint clk = ClkDomain::getInstance()->getClk();
    valid = 1;

    bool match = 1;
    for (auto& buffer : chanBuffer)
    {
        if (buffer.empty() || buffer.front().cycle > clk)
        {
            match = 0;
        }
    }

    if (match/* && channel.empty()*/)  // Channel size is 1
    {
        pushReqQ();
    }

    // Select a ready req
    if (!suspendReq.first)  // If suspendReq is invalid
    {
        //for (size_t i = 0; i < reqQueue.size(); ++i)
        //{
        //    auto& req = reqQueue[sendMemPtr].first;
        //    if (req.valid && !req.inflight && !req.ready)
        //    {
        //        if (reqQueue[sendMemPtr].second.cycle <= clk)  // Satisfy clk restriction
        //        {
        //            req.lseReqQueueIndex = sendMemPtr;
        //            if (NO_MEMORY || noLatencyMode)
        //            {
        //                ackCallback(req);  // Send back ack directly(Not send reqs to memSys)
        //            }
        //            else
        //            {
        //                // If Lse in the false path, not send req to memory and sendback ack directly
        //                if (branchMode && reqQueue[sendMemPtr].second.cond != channelCond)
        //                {
        //                    ackCallback(req);
        //                }
        //                else
        //                {
        //                    suspendReq = make_pair(true, req);
        //                }
        //            }

        //            sendMemPtr = (++sendMemPtr) % reqQueue.size();  // Update sendPtr, round-robin
        //            break;
        //        }
        //    }
        //    //sendMemPtr = (++sendMemPtr) % reqQueue.size();  // Update sendPtr, round-robin
        //}

        auto& req = reqQueue[sendMemPtr].first;
        if (req.valid && !req.inflight && !req.ready)
        {
            if (reqQueue[sendMemPtr].second.cycle <= clk)  // Satisfy clk restriction
            {
                req.lseReqQueueIndex = sendMemPtr;
                if (NO_MEMORY || noLatencyMode)
                {
                    reqQueue[req.lseReqQueueIndex].first.inflight = 1;  // Set to inflight virtually
                    ackCallback(req);  // Send back ack directly(Not send reqs to memSys)
                }
                else
                {
                    // If Lse in the false path, not send req to memory and sendback ack directly
                    if (branchMode && reqQueue[sendMemPtr].second.cond != channelCond)
                    {
                        reqQueue[req.lseReqQueueIndex].first.inflight = 1;  // Set to inflight virtually
                        ackCallback(req);
                    }
                    else
                    {
                        suspendReq = make_pair(true, req);
                    }
                }

                sendMemPtr = (++sendMemPtr) % reqQueue.size();  // Update sendPtr, round-robin
            }
        }
    }

    // Send ready data to channel
    pushChannel();

    if (!channel.empty())
    {
        //Data data = channel.front();
        // Check sendable
        if (!noDownstream)
        {
            for (auto& channel : downstream)
            {
                if (!channel->checkSend(this->channel.front(), this))
                {
                    valid = 0;
                    break;
                }
            }
        }
    }
    else
    {
        valid = 0;
    }

    if (valid)
    {
        ++chanDataCnt;
    }

    bpUpdate();

    //// Emulate hardware parallel loop unrolling
    //if (speedup > 1)
    //{
    //    parallelize();
    //}
}

void Lse::pushChannel()
{
    uint clk = ClkDomain::getInstance()->getClk();
    //uint cnt = (std::numeric_limits<uint>::max)();  // Initial the max value
    bool getValid = 0;

    if (channel.size() < size)
    {
        if (LSE_O3)  // If lse OoO
        {
            for (size_t i = 0; i < reqQueue.size(); ++i)
            {
                auto& req = reqQueue[sendChanPtr];
                if (req.first.valid && req.first.ready && !req.first.hasPushChan)
                {
                    req.second.cycle = clk;  // Update cycle
                    channel.push_back(req.second);
                    req.first.hasPushChan = 1;
                    sendChanPtr = (++sendChanPtr) % reqQueue.size();
                    break;
                }

                //if (channel.size() >= size)
                //{
                //    break;
                //}
            }
        }
        else  // If Lse in order
        {
            //for(size_t i = 0; i < reqQueue.size(); ++i)
            //{
            //    sendChanPtr = (sendChanPtr++) % reqQueue.size();
            //    auto req = reqQueue[sendChanPtr];
            //    if (req.first.cnt == currReqId)
            //    {
            //        if (req.first.valid && req.first.ready && !req.first.hasPushChan)
            //        {
            //            req.second.cycle = clk;  // Update cycle
            //            channel.push_back(req.second);
            //            req.first.hasPushChan = 1;
            //            ++currReqId;
            //        }
            //    }
            //    else
            //    {
            //        break;
            //    }

            //    if (channel.size() >= size)
            //    {
            //        break;
            //    }
            //}

            auto& req = reqQueue[sendChanPtr];
            if (req.first.valid && req.first.ready && !req.first.hasPushChan)
            {
                // Double check, ensure in order
                if (req.first.cnt != currReqId)
                {
                    Debug::throwError("Lse sendChanPtr not consist with currReqId!", __FILE__, __LINE__);
                }

                req.second.cycle = clk;  // Update cycle
                channel.push_back(req.second);
                req.first.hasPushChan = 1;
                ++currReqId;
                sendChanPtr = (++sendChanPtr) % reqQueue.size();
            }
        }
    }
}

void Lse::funcUpdate()
{
    if (valid)
    {
        // chanBuffer[0] stores address
        value = (*memorySpace)[channel.front().value - baseAddr];

        if (isWrite)
        {
            (*memorySpace)[channel.front().value - baseAddr] = valueQueue.front();
        }
    }
}

//bool Lse::checkSend(Data _data, Channel* upstream)
//{
//    bool sendable = 1;
//    if (bp)
//    {
//        sendable = 0;
//    }
//    return sendable;
//}

//void Lse::bpUpdate()
//{
//    /*if (reqQueue.size() < size)
//        bp = 0;
//    else
//        bp = 1;*/
//
//    bp = 1;
//    for (auto& req : reqQueue)
//    {
//        if (!req.first.valid)
//        {
//            bp = 0;
//            break;
//        }
//    }
//}

uint Lse::assign()
{
    if (!this->channel.empty())
    {
        uint index = channel.front().lseReqQueueIndex;
        return reqQueue[index].first.addr - baseAddr;
    }
    else
    {
        return lastPopVal >= baseAddr ? lastPopVal - baseAddr : 0;
    }
}

void Lse::ackCallback(MemReq _req)
{
    uint index = _req.lseReqQueueIndex;
    if (!reqQueue[index].first.inflight)
    {
        Debug::throwError("Current req is not an inflight req!", __FILE__, __LINE__);
    }
    reqQueue[index].first.ready = 1;
    reqQueue[index].first.inflight = 0;
    //reqQueue[index].first.cnt = _req.cnt;

#ifdef DEBUG_MODE 
    updateMemAccessRecord(index);  // Profiling
#endif // DEBUG_MODE
}

pair<bool, MemReq> Lse::peekReqQueue()
{
    auto req = make_pair(0, MemReq());
    if (suspendReq.first)
    {
        req.first = 1;
        req.second = suspendReq.second;
    }

    return req;
}

void Lse::setInflight(MemReq& _req)
{
//    bool sendSuccess = 0;
//    bool reqInvalid = 0;
//
//    if (suspendReq.first)
//    {
//#ifdef DEBUG_MODE 
//        memReqCnt++;
//#endif // DEBUG_MODE
//
//        auto& req = suspendReq.second;
//        if (memSys->addTransaction(req))
//        {
//            uint index = req.lseReqQueueIndex;
//            reqQueue[index].first.inflight = 1;  // Req has been sent to memory
//            suspendReq.first = 0;  // Clear current suspendReq
//            sendSuccess = 1;
//        }
//#ifdef DEBUG_MODE 
//        else
//        {
//            memReqBlockCnt++;
//        }
//#endif // DEBUG_MODE
//    }
//    else
//    {
//        reqInvalid = 1;
//    }
//
//    return sendSuccess || reqInvalid;

    if (!suspendReq.first)
    {
        Debug::throwError("Try to send an invalid request from Lse to MemSys", __FILE__, __LINE__);
    }
    else
    {
#ifdef DEBUG_MODE 
        memReqCnt++;
#endif // DEBUG_MODE

        /*auto& req = suspendReq.second;*/
        uint index = _req.lseReqQueueIndex;
        reqQueue[index].first.inflight = 1;  // Req has been sent to memory
        reqQueue[index].first.coalesced = _req.coalesced;
        suspendReq.first = 0;  // Clear current suspendReq
    }
}

void Lse::updateMemAccessRecord(uint index)
{
    uint latency = ClkDomain::getClk() - reqQueue[index].second.cycle;
    latTotal += latency;
    avgMemAccessLat = latTotal / (++memAccessCnt);
    /*std::cout << "lat: " << latency << " cnt: " << memAccessCnt << " avg: " << avgMemAccessLat << std::endl;*/
}

#ifdef DEBUG_MODE  // Get private instance for debug
const uint& Lse::getCurrReqId() const
{
    return currReqId;
}
#endif // DEBUG_MODE