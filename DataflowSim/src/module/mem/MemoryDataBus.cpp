#include "./MemoryDataBus.h"

using namespace DFSim;

// MemoryDataBus
MemoryDataBus::MemoryDataBus()
{
}

void MemoryDataBus::mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
{
    MemReq req;
    req.valid = 1;
    req.isWrite = false;
    req.addr = _addr;

    busDelayFifo.push_back(std::make_pair(req, busDelay));
}

void MemoryDataBus::mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
{
    MemReq req;
    req.valid = 1;
    req.isWrite = true;
    req.addr = _addr;

    busDelayFifo.push_back(std::make_pair(req, busDelay));
}

vector<MemReq> MemoryDataBus::MemoryDataBusUpdate()
{
    // Emulate bus delay
    for (auto& req : busDelayFifo)
    {
        if (req.second > 0)
        {
            --req.second;
        }
    }

    // Send ready req from memoryDataBus to reqStack in memSystem
    vector<MemReq> _reqAckStack;

    //bool traverse = 1;
    //while (traverse)
    //{
    //    if (!busDelayFifo.empty() && busDelayFifo.front().second == 0)
    //    {
    //        _reqAckStack.push_back(busDelayFifo.front().first);
    //        busDelayFifo.pop_front();
    //    }
    //    else
    //    {
    //        traverse = 0;
    //    }
    //}

    for (auto iter = busDelayFifo.begin(); iter != busDelayFifo.end(); )
    {
        if (iter->second == 0)
        {
            _reqAckStack.push_back(iter->first);
            iter = busDelayFifo.erase(iter);
        }
        else
        {
            break;
        }
    }

    return _reqAckStack;
}

//vector<MemReq> MemoryDataBus::getFromBusDelayFifo()
//{
//    vector<MemReq> reqStack;
//    bool traverse = 1;
//    while (traverse)
//    {
//        if (!busDelayFifo.empty() && busDelayFifo.front().second == 0)
//        {
//            reqStack.push_back(busDelayFifo.front().first);
//            busDelayFifo.pop_front();
//        }
//        else
//        {
//            traverse = 0;
//        }
//    }
//
//    return reqStack;
//}


#ifdef DEBUG_MODE
const deque<pair<MemReq, uint>>& MemoryDataBus::getBusDelayFifo() const
{
    return busDelayFifo;
}
#endif // DEBUG_MODE