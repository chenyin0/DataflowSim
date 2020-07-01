#include "./GraphScheduler.h"

using namespace DFSim;

GraphScheduler::GraphScheduler()
{
}

void GraphScheduler::schedulerInit()
{
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
                if (!chan->chanBuffer[bufferId].empty())
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
        bool getLast = 1;

        for (auto& chan : subgraphTable[currSubgraphId].second)
        {
            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                if (/*!chan->getTheLastData[bufferId] && */chan->chanBuffer[bufferId].size() < chan->size)
                {
                    consumerChanFinish = 0;
                    //break;
                }

                if (!chan->chanBuffer[bufferId].empty())
                {
                    if (!(chan->chanBuffer[bufferId].back().lastOuter && chan->chanBuffer[bufferId].back().last))
                    {
                        getLast = 0;  // Check whether all the channel has received the last data
                    }
                }
                else
                {
                    getLast = 0;
                }
            }

            if (getLast == 0 && consumerChanFinish == 0)
            {
                break;
            }
        }

        if (getLast)
        {
            consumerChanFinish = 1;
        }
    }
    else
    {
        consumerChanFinish = 0;
    }

    if (/*producerChanFinish && */consumerChanFinish || (subgraphTable[currSubgraphId].second.empty() && producerChanFinish))
    {
        currSubgraphId = (++currSubgraphId) % subgraphTable.size();
        configChan(currSubgraphId);
    }
}

void GraphScheduler::configChan(uint subgraphId)
{
    for (auto& chan : subgraphTable[subgraphId].first)
    {
        chan->pushBufferEnable = 0;
        chan->popChannelEnable = 1;
    }

    for (auto& chan : subgraphTable[subgraphId].second)
    {
        chan->pushBufferEnable = 1;
        chan->popChannelEnable = 0;
    }
}