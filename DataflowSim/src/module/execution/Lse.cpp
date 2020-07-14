#include "./Lse.h"
#include "../ClkSys.h"

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

Lse::~Lse()
{
    //if (memSys != nullptr)
    //{
    //    delete memSys;
    //}
    memSys = nullptr;  // Destruct memorySystem in the destructor of class MemSystem
}

void Lse::initial()
{
    reqQueue.resize(size);

    lseId = memSys->registerLse(this);  // Register Lse in MemSystem and return lseId

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
    //data.cycle = data.cycle + cycle;
    /*uint cycleTemp = data.cycle + cycle;
    data.cycle = cycleTemp > clk ? cycleTemp : clk;*/

    // Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag;
    for (auto buffer : chanBuffer)
    {
        data.last |= buffer.front().last;
        data.lastOuter |= buffer.front().lastOuter;
        data.graphSwitch |= buffer.front().graphSwitch;
    }
    //for (auto channel : upstream)
    //{
    //    data.last |= channel->channel.front().last;
    //    data.lastOuter |= channel->channel.front().lastOuter;
    //    data.graphSwitch |= channel->channel.front().graphSwitch;
    //}

    // Send lastTag to each upstream channel in keepMode
    if (data.last)
    {
        sendLastTag();
    }

    MemReq req;
    req.valid = 1;
    req.addr = chanBuffer[0].front().value;  // Address must stored in chanBuffer[0]!!!
    req.isWrite = isWrite;
    req.lseId = lseId;

    for (size_t i = 0; i < reqQueue.size(); ++i)
    {
        if (!reqQueue[i].first.valid)
        {
            req.lseReqQueueIndex = i;  // Record reqQueue Id, for callback from memory
            req.cnt = orderId;
            ++orderId;
            data.lseReqQueueIndex = i;  // Record reqQueue Id, for delete from channel
            reqQueue[i].first = req;
            reqQueue[i].second = data;

            popChanBuffer();
            break;
        }
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

bool Lse::sendReq(MemReq _req)
{
    // No memory (memory latency = 0)
    if (NO_MEMORY || noLatencyMode)
    {
        ackCallback(_req);  // Send back ack directly(Not send reqs to memSys)
        return true;
    }
    else
    {
        if (memSys->addTransaction(_req))
        {
            uint index = _req.lseReqQueueIndex;
            reqQueue[index].first.inflight = 1;  // Req has been sent to memory

            return true;
        }
        else
        {
            return false;
        }
    }
}

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

    // Send req to memSystem
    for (size_t i = 0; i < reqQueue.size(); ++i)
    {
        MemReq& req = reqQueue[sendPtr].first;
        if (req.valid && !req.inflight && !req.ready)
        {
            if (reqQueue[sendPtr].second.cycle < clk)  // Satisfy clk restriction
            {
                // If Lse in the false path, not send req to memory and sendback ack directly
                if (branchMode && reqQueue[sendPtr].second.cond != channelCond)
                {
                    ackCallback(req);
                }
                else
                {
                    if (!sendReq(req))  // Send req to MemSystem, if send failed (reqQueue is full) -> break;
                    {
                        //sendPtr = (++sendPtr) % reqQueue.size();  // Update sendPtr, round-robin
                        break;
                    }
                }
            }
        }

        sendPtr = (++sendPtr) % reqQueue.size();  // Update sendPtr, round-robin
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

    // Emulate hardware parallel loop unrolling
    if (speedup > 1)
    {
        parallelize();
    }
}

void Lse::pushChannel()
{
    uint clk = ClkDomain::getInstance()->getClk();
    uint cnt = (std::numeric_limits<uint>::max)();  // Initial the max value
    bool getValid = 0;

    if (channel.size() < size)
    {
        if (LSE_O3)  // If lse OoO
        {
            for (auto& req : reqQueue)
            {
                if (req.first.valid && req.first.ready && !req.first.hasPushChan)
                {
                    req.second.cycle = clk;  // Update cycle
                    channel.push_back(req.second);
                    req.first.hasPushChan = 1;
                }
            }
        }
        else  // If Lse in order
        {
            //while (1)
            //{
            //    // Find the earliest valid req
            //    for (size_t i = 0; i < reqQueue.size(); ++i)
            //    {
            //        if (reqQueue[i].first.valid && !reqQueue[i].first.hasPushChan)
            //        {
            //            if (reqQueue[i].first.cnt < cnt)
            //            {
            //                getValid = 1;
            //                cnt = reqQueue[i].first.cnt;
            //            }
            //        }
            //    }

            //    // If find out the earliest valid rea
            //    if (getValid)
            //    {
            //        if()
            //    }
            //}

            for(auto& req:reqQueue)
            {
                if (req.first.cnt == currReqId)
                {
                    if (req.first.valid && req.first.ready && !req.first.hasPushChan)
                    {
                        req.second.cycle = clk;  // Update cycle
                        channel.push_back(req.second);
                        req.first.hasPushChan = 1;
                        ++currReqId;
                    }
                }
            }
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
        return reqQueue[index].first.addr;
    }
    else
        return lastPopVal;
}

void Lse::ackCallback(MemReq _req)
{
    uint index = _req.lseReqQueueIndex;
    reqQueue[index].first.ready = 1;
    reqQueue[index].first.inflight = 0;
    //reqQueue[index].first.cnt = _req.cnt;
}

#ifdef DEBUG_MODE  // Get private instance for debug
const uint& Lse::getCurrReqId() const
{
    return currReqId;
}
#endif // DEBUG_MODE