#include "./GraphScheduler.h"
#include "../../sim/Debug.h"

using namespace DFSim;

GraphScheduler::GraphScheduler()
{
}

void GraphScheduler::schedulerInit()
{
    subgraphIsOver.resize(subgraphTable.size());
    configChan(currSubgraphId);
}

void GraphScheduler::addSubgraph(uint subgraphId, vector<ChanDGSF*> producerChan, vector<ChanDGSF*> consumerChan)
{
    subgraphTable.insert(make_pair(subgraphId, make_pair(producerChan, consumerChan)));
}

void GraphScheduler::addDivergenceSubgraph(uint subgraphId, vector<ChanDGSF*> commonChan, vector<ChanDGSF*> truePathChan, vector<ChanDGSF*> falsePathChan)
{
    vector<vector<ChanDGSF*>> diverGraph = { commonChan, truePathChan, falsePathChan };
    divergenceGraph.insert(make_pair(subgraphId, diverGraph));
}

void GraphScheduler::graphUpdate()
{
    uint graphSize = subgraphTable.size();
    bool producerChanFinish = 1;
    bool consumerChanFinish = 1;

    // Check whether all the producerChan is empty
    if (!subgraphTable[currSubgraphId].first.empty())
    {
        producerChanFinish = checkProducerChanFinish(subgraphTable[currSubgraphId].first);

        //for (auto& chan : subgraphTable[currSubgraphId].first)
        //{
        //    for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
        //    {
        //        //// If producer channal is empty or has sent out enough data, it is able to switch sub-graph 
        //        //if (!(chan->chanBuffer[bufferId].empty() || chan->chanDataCnt == DGSF_INPUT_BUFF_SIZE))
        //        //{
        //        //    producerChanFinish = 0;
        //        //    break;
        //        //}

        //        // If producer channal is empty or has sent out enough data, it is able to switch sub-graph 
        //        if (!subgraphTable[currSubgraphId].second.empty() && chan->chanDataCnt >= DGSF_INPUT_BUFF_SIZE)
        //        {
        //            // If producer channel has sent enough data, disable popChannel
        //            chan->popChannelEnable = 0;
        //        }
        //        else if (!chan->chanBuffer[bufferId].empty())
        //        {
        //            producerChanFinish = 0;
        //            break;
        //        }
        //    }

        //    if (!producerChanFinish)
        //    {
        //        break;
        //    }
        //}
    }
    else
    {
        producerChanFinish = 0;
    }

    // Check whether all the comsumerChan is full
    if (!subgraphTable[currSubgraphId].second.empty())
    {
        // If not a branch divergence subgraph
        if (divergenceGraph.count(currSubgraphId) == 0)
        {
            //bool consumerChanIsFull = !checkConsumerChanNotFull(subgraphTable[currSubgraphId].second);
            //bool consumerChanGetLastData = checkConsumerChanGetLastData(subgraphTable[currSubgraphId].second);

            //consumerChanFinish = consumerChanIsFull || consumerChanGetLastData;

            consumerChanFinish = checkConsumerChanFinish(subgraphTable[currSubgraphId].second);  // Debug_yin_12.26
        }
        else
        {
            //bool commonChanIsFull = !checkConsumerChanNotFull(divergenceGraph[currSubgraphId][0]);
            //bool commonChanGetLastData = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][0]);

            //bool truePathChanGetLastData = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][1]);
            //bool falsePathChanGetLastData = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][2]);

            //consumerChanFinish = commonChanIsFull || (commonChanGetLastData && (truePathChanGetLastData || falsePathChanGetLastData));

            // Debug_yin_12.26
            bool commonChanIsFinish = checkConsumerChanFinish(divergenceGraph[currSubgraphId][0]);
            bool truePathChanIsFinish = checkConsumerChanFinish(divergenceGraph[currSubgraphId][1]);
            bool falsePathChanGetIsFinish = checkConsumerChanFinish(divergenceGraph[currSubgraphId][2]);

            consumerChanFinish = commonChanIsFinish && (truePathChanIsFinish || falsePathChanGetIsFinish);
        }
    }
    else
    {
        consumerChanFinish = 0;
    }

    //// Check whether current sub-graph is over
    //subgraphIsOver[currSubgraphId] = checkSubgraphIsOver(currSubgraphId);

    if (producerChanFinish || consumerChanFinish)
    {
        // Check whether current sub-graph is over
        subgraphIsOver[currSubgraphId] = checkSubgraphIsOver(currSubgraphId);
        // Select a new subgraph
        //currSubgraphId = selectSubgraphInOrder(currSubgraphId);
        currSubgraphId = selectSubgraphO3(currSubgraphId);
        // Config all channels in new subgraph
        configChan(currSubgraphId);
        // Clear the chanDataCnt of subgraph's producer channels
        resetSubgraph(currSubgraphId);
    }

    //if (/*producerChanFinish && */consumerChanFinish || (subgraphTable[currSubgraphId].second.empty() && producerChanFinish))
    //{
    //    // Clear the chanDataCnt of subgraph's producer channels
    //    resetSubgraph(currSubgraphId);
    //    // Select a new subgraph
    //    currSubgraphId = selectSubgraphInOrder(currSubgraphId);
    //    // Config all channels in new subgraph
    //    configChan(currSubgraphId);
    //}
}

/*
Graph rule:
                Producer channels:          Consumer channels:
                pushBuffer  popChannel      pushBuffer  popChannel
Current graph:  No          Yes             Yes         No
Other graphs:   Yes         No              Yes         No
*/
void GraphScheduler::configChan(uint subgraphId)
{
    //TODO: Code is not clear, need rewriting

    // Set other subgraph
    for (size_t id = 0; id < subgraphTable.size(); ++id)
    {
        if (id != subgraphId)
        {
            if (!subgraphTable[id].first.empty())
            {
                for (auto& chan : subgraphTable[id].first)
                {
                    chan->pushBufferEnable = 1;
                    chan->popChannelEnable = 0;
                }

                for (auto& chan : subgraphTable[id].second)
                {
                    chan->pushBufferEnable = 1;
                    chan->popChannelEnable = 0;
                }
            }
            else
            {
                for (auto& chan : subgraphTable[id].second)
                {
                    chan->pushBufferEnable = 0;
                    chan->popChannelEnable = 0;
                }
            }
        }
    }

    // Set selected subgraph
    for (auto& chan : subgraphTable[subgraphId].first)
    {
        //chan->pushBufferEnable = 0;
        chan->pushBufferEnable = 1;
        chan->popChannelEnable = 1;
    }

    for (auto& chan : subgraphTable[subgraphId].second)
    {
        chan->pushBufferEnable = 1;
        chan->popChannelEnable = 0;
    }

    for (size_t id = 0; id < subgraphTable.size(); ++id)
    {
        if (id != subgraphId)
        {
            if (subgraphTable[id].first.empty())
            {
                for (auto& chan : subgraphTable[id].second)
                {
                    chan->pushBufferEnable = 0;
                }
            }
        }
    }
}

uint GraphScheduler::selectSubgraphInOrder(uint _currSubgraphId)
{
    return (++_currSubgraphId) % subgraphTable.size();
}

//uint GraphScheduler::selectSubgraphO3(uint _currSubgraphId)
//{
//    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
//    {
//        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
//        {
//            if (checkProducerChanIsFull(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
//            {
//                return subgraphId;
//            }
//        }
//    }
//
//    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
//    {
//        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
//        {
//            if (checkProducerChanNotEmpty(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
//            {
//                return subgraphId;
//            }
//        }
//    }
//
//    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
//    {
//        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
//        {
//            return subgraphId;  // Return a subgraph randomly
//        }
//    }
//
//    Debug::throwError("Can not find a subgraph to execute. Program should be already finished!", __FILE__, __LINE__);
//}

uint GraphScheduler::selectSubgraphO3(uint _currSubgraphId)
{
    for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size(); ++subgraphCnt)
    {
        uint subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
        if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
        {
            if (checkProducerChanIsFull(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }
        }
    }

    for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size(); ++subgraphCnt)
    {
        uint subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
        if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
        {
            if (checkProducerChanNotEmpty(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }
        }
    }

    // Select a subgraph to execute in round-robin 
    for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size() + 1; ++subgraphCnt)
    {
        uint subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
        if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
        {
            return subgraphId;  // Return a subgraph randomly
        }
    }

    Debug::throwError("Can not find a subgraph to execute. Program should be already finished!", __FILE__, __LINE__);
}

bool GraphScheduler::checkSubgraphIsOver(uint _subgraphId)
{
    bool isOver = 1;

    if (!subgraphTable[_subgraphId].second.empty())
    {
        if (divergenceGraph.count(currSubgraphId) == 0)
        {
            //isOver = checkConsumerChanGetLastData(subgraphTable[_subgraphId].second);

            // Debug_yin_12.26
            isOver = checkConsumerChanGetLastData(subgraphTable[_subgraphId].second) && checkProducerChanAllEmpty(subgraphTable[_subgraphId].first);
        }
        else
        {
            bool commonChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][0]);
            bool truePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][1]);
            bool falsePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][2]);

            isOver = commonChanIsOver && (truePathChanIsOver || falsePathChanIsOver);
        }

        //for (auto& chan : subgraphTable[_subgraphId].second)
        //{
        //    for (auto& i : chan->getTheLastData)
        //    {
        //        if (i != true)  // Hasn't received the last data
        //        {
        //            isOver = 0;
        //            return isOver;
        //        }
        //    }
        //}
    }
    else
    {
        isOver = 0;
    }

    return isOver;
}

bool GraphScheduler::checkProducerChanIsFull(vector<ChanDGSF*> producerChans)
{
    bool isFull = 1;

    for (auto& chan : producerChans)
    {
        for (auto& buffer : chan->chanBuffer)
        {
            if (buffer.size() < chan->size)
            {
                isFull = 0;
                return isFull;
            }
        }
    }

    return isFull;
}

bool GraphScheduler::checkProducerChanNotEmpty(vector<ChanDGSF*> producerChans)
{
    bool notEmpty = 1;

    for (auto& chan : producerChans)
    {
        for (auto& buffer : chan->chanBuffer)
        {
            if (buffer.empty())
            {
                notEmpty = 0;
                return notEmpty;
            }
        }
    }

    return notEmpty;
}

bool GraphScheduler::checkProducerChanAllEmpty(vector<ChanDGSF*> producerChans)
{
    bool allEmpty = 1;

    for (auto& chan : producerChans)
    {
        for (auto& buffer : chan->chanBuffer)
        {
            if (!buffer.empty())
            {
                allEmpty = 0;
                return allEmpty;
            }
        }
    }

    return allEmpty;
}

bool GraphScheduler::checkConsumerChanNotFull(vector<ChanDGSF*> consumerChans)
{
    //bool notFull = 1;

    //for (auto& chan : subgraphTable[_subgraphId].second)
    //{
    //    for (auto& buffer : chan->chanBuffer)
    //    {
    //        if (buffer.size() == chan->size)
    //        {
    //            notFull = 0;
    //            return notFull;
    //        }
    //    }
    //}

    //return notFull;

    bool notFull = 0;

    if (!consumerChans.empty())
    {
        for (auto& chan : consumerChans)
        {
            for (auto& buffer : chan->chanBuffer)
            {
                if (buffer.size() < chan->size)
                {
                    notFull = 1;
                    return notFull;
                }
            }
        }
    }
    else
    {
        notFull = 1;
    }

    return notFull;
}

void GraphScheduler::resetSubgraph(uint _subgraphId)
{
    for (auto& chan : subgraphTable[_subgraphId].first)
    {
        chan->chanDataCnt = 0;
    }

    for (auto& chan : subgraphTable[_subgraphId].second)
    {
        chan->chanDataCnt = 0;
    }
}

bool GraphScheduler::checkProducerChanFinish(vector<ChanDGSF*> producerChans)
{
    bool finish = 1;

    for (auto& chan : producerChans)
    {
        for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
        {
            //// If producer channal is empty or has sent out enough data, it is able to switch sub-graph 
            //if (!(chan->chanBuffer[bufferId].empty() || chan->chanDataCnt == DGSF_INPUT_BUFF_SIZE))
            //{
            //    producerChanFinish = 0;
            //    break;
            //}

            // If producer channal is empty or has sent out enough data, it is able to switch sub-graph 
            if (!subgraphTable[currSubgraphId].second.empty() && chan->chanDataCnt >= DGSF_INPUT_BUFF_SIZE)
            {
                // If producer channel has sent enough data, disable popChannel
                chan->popChannelEnable = 0;
            }
            else if (!chan->chanBuffer[bufferId].empty())
            {
                finish = 0;
                break;
            }
        }

        if (!finish)
        {
            break;
        }
    }

    return finish;
}

bool GraphScheduler::checkConsumerChanFinish(vector<ChanDGSF*> consumerChans)
{
    bool finish = 1;

    for (auto& chan : consumerChans)
    {
        for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
        {
            if (!chan->chanBuffer[bufferId].empty())
            {
                // When 1)the chanBuffer has received the last data; or 2)the chanBuffer is full, this consumer channel is finish
                if (!((chan->chanBuffer[bufferId].back().lastOuter && chan->chanBuffer[bufferId].back().last) || 
                    chan->chanBuffer[bufferId].size() >= chan->size))  
                {
                    finish = 0;
                    break;
                }
            }
            else
            {
                finish = 0;
            }
        }

        if (finish == 0)
        {
            break;
        }
    }

    return finish;
}

bool GraphScheduler::checkConsumerChanGetLastData(vector<ChanDGSF*> consumerChans)
{
    //for (auto& chan : consumerChans)
    //{
    //    for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
    //    {
    //        if (!chan->chanBuffer[bufferId].empty())
    //        {
    //            if (chan->chanBuffer[bufferId].back().lastOuter && chan->chanBuffer[bufferId].back().last)  // Check whether all the channel has received the last data
    //            {
    //                chan->getTheLastData[bufferId] = 1;  // Signify this chanBuffer has received the last data
    //            }
    //        }
    //    }
    //}

    //bool getLast = 1;
    //for (auto& chan : consumerChans)
    //{
    //    for (auto& i : chan->getTheLastData)
    //    {
    //        if (i != true)  // Hasn't received the last data
    //        {
    //            getLast = 0;
    //            return getLast;
    //        }
    //    }
    //}

    //return getLast;

    bool getLast = 1;
    for (auto& chan : consumerChans)
    {
        for (auto& i : chan->getTheLastData)
        {
            if (i == 0)
            {
                return getLast = 0;
            }
        }
    }

    return getLast;
}

#ifdef DEBUG_MODE  // Get private instance for debug
const deque<bool>& GraphScheduler::getSubgraphStatus() const
{
    return subgraphIsOver;
}
#endif // DEBUG_MODE 