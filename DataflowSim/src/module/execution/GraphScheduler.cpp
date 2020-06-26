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
            //if (!chan->chanBuffer[0].empty())
            //{
            //    producerChanFinish = 0;
            //    break;
            //}

            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                //if (!chan->upstream[bufferId]->keepMode)
                //{
                //    if (chan->chanBuffer[bufferId].empty())
                //    {
                //        producerChanFinish = 1;
                //        break;
                //    }
                //}

                //if (chan->chanBuffer[bufferId].empty())
                //{
                //    producerChanFinish = 1;
                //    break;
                //}

                if (!chan->chanBuffer[bufferId].empty())
                {
                    producerChanFinish = 0;
                    break;
                }
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
        for (auto& chan : subgraphTable[currSubgraphId].second)
        {
            //if (chan->chanBuffer[0].size() != chan->size)
            //{
            //    consumerChanFinish = 0;
            //    break;
            //}

            //if (!chan->keepMode)
            //{
            //    for (auto& buffer : chan->chanBuffer)
            //    {
            //        if (buffer.size() >= chan->size)
            //        {
            //            consumerChanFinish = 1;
            //            break;
            //        }
            //    }
            //}

            //for (auto& buffer : chan->chanBuffer)
            //{
            //    if (buffer.size() >= chan->size)
            //    {
            //        consumerChanFinish = 1;
            //        break;
            //    }
            //}

            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                if (!chan->upstream[bufferId]->keepMode)
                {
                    if (chan->chanBuffer[bufferId].size() < chan->size)
                    {
                        consumerChanFinish = 0;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        consumerChanFinish = 0;
    }

    if (producerChanFinish || consumerChanFinish)
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