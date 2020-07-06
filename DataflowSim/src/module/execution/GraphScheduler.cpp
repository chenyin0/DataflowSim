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

void GraphScheduler::graphUpdate()
{
    uint graphSize = subgraphTable.size();
    bool producerChanFinish = 1;
    bool consumerChanFinish = 1;

    // Check whether all the producerChan is empty
    if (!subgraphTable[currSubgraphId].first.empty())
    {
        for (auto& chan : subgraphTable[currSubgraphId].first)
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
                    producerChanFinish = 0;
                    break;
                }
            }

            if (!producerChanFinish)
            {
                break;
            }
        }
    }
    else
    {
        producerChanFinish = 0;
    }

    // Check whether all the comsumerChan is full
    if (!subgraphTable[currSubgraphId].second.empty())
    {
        //bool getLast = 1;

        for (auto& chan : subgraphTable[currSubgraphId].second)
        {
            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                //if (/*!chan->getTheLastData[bufferId] && */chan->chanBuffer[bufferId].size() < chan->size)
                //{
                //    consumerChanFinish = 0;
                //    //break;
                //}

                //if (!chan->chanBuffer[bufferId].empty())
                //{
                //    if (!(chan->chanBuffer[bufferId].back().lastOuter && chan->chanBuffer[bufferId].back().last))
                //    {
                //        getLast = 0;  // Check whether all the channel has received the last data
                //    }
                //}
                //else
                //{
                //    getLast = 0;
                //}

                if (!chan->chanBuffer[bufferId].empty())
                {
                    if (!(chan->chanBuffer[bufferId].back().lastOuter && chan->chanBuffer[bufferId].back().last))  // Check whether all the channel has received the last data
                    {
                        if (chan->chanBuffer[bufferId].size() < chan->size)
                        {
                            consumerChanFinish = 0;
                            break;
                        }
                    }
                    else
                    {
                        chan->getTheLastData[bufferId] = 1;  // Signify this chanBuffer has received the last data
                    }
                }
                else
                {
                    consumerChanFinish = 0;
                }
            }

            if (consumerChanFinish == 0)
            {
                break;
            }

            //if (getLast == 0 && consumerChanFinish == 0)
            //{
            //    break;
            //}
        }

        //if (getLast)
        //{
        //    consumerChanFinish = 1;
        //}
    }
    else
    {
        consumerChanFinish = 0;
    }

    // Check whether current sub-graph is over
    subgraphIsOver[currSubgraphId] = checkSubgraphIsOver(currSubgraphId);

    if (producerChanFinish || consumerChanFinish)
    {
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

uint GraphScheduler::selectSubgraphO3(uint _currSubgraphId)
{
    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
    {
        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
        {
            if (checkProducerChanIsFull(subgraphId) && checkConsumerChanNotFull(subgraphId))
            {
                return subgraphId;
            }
        }
    }

    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
    {
        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
        {
            if (checkProducerChanNotEmpty(subgraphId) && checkConsumerChanNotFull(subgraphId))
            {
                return subgraphId;
            }
        }
    }

    for (size_t subgraphId = 0; subgraphId < subgraphTable.size(); ++subgraphId)
    {
        if (!subgraphIsOver[subgraphId] && subgraphId != _currSubgraphId)
        {
            return subgraphId;  // Return a subgraph randomly
        }
    }

    Debug::throwError("Can't find a subgraph to execute. Program should be already finished!", __FILE__, __LINE__);
}

bool GraphScheduler::checkSubgraphIsOver(uint _subgraphId)
{
    bool isOver = 1;

    if (!subgraphTable[_subgraphId].second.empty())
    {
        for (auto& chan : subgraphTable[_subgraphId].second)
        {
            for (auto& i : chan->getTheLastData)
            {
                if (i != true)  // Hasn't received the last data
                {
                    isOver = 0;
                    return isOver;
                }
            }
        }
    }
    else
    {
        isOver = 0;
    }

    return isOver;
}

bool GraphScheduler::checkProducerChanIsFull(uint _subgraphId)
{
    bool isFull = 1;

    for (auto& chan : subgraphTable[_subgraphId].first)
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

bool GraphScheduler::checkProducerChanNotEmpty(uint _subgraphId)
{
    bool notEmpty = 1;

    for (auto& chan : subgraphTable[_subgraphId].first)
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

bool GraphScheduler::checkConsumerChanNotFull(uint _subgraphId)
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

    if (!subgraphTable[_subgraphId].second.empty())
    {
        for (auto& chan : subgraphTable[_subgraphId].second)
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