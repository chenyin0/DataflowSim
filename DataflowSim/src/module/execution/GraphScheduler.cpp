#include "./GraphScheduler.h"
#include "../../sim/Debug.h"
#include "../../sim/global.h"

using namespace DFSim;

uint64_t GraphScheduler::currSubgraphId = 0;  // Current subgraph Id, default start at 0;

GraphScheduler::GraphScheduler()
{
}

void GraphScheduler::schedulerInit()
{
    subgraphIsOver.resize(subgraphTable.size());
    configChan(currSubgraphId);
    subgraphActiveCnt.resize(subgraphTable.size());
}

void GraphScheduler::addSubgraph(uint64_t subgraphId, vector<ChanDGSF*> producerChan, vector<ChanDGSF*> consumerChan)
{
    if (subgraphTable.find(subgraphId) == subgraphTable.end())
    {
        subgraphTable.insert(make_pair(subgraphId, make_pair(producerChan, consumerChan)));
    }
    else
    {
        for (auto& chan : producerChan)
        {
            if (count(subgraphTable[subgraphId].first.begin(), subgraphTable[subgraphId].first.end(), chan) == 0)
            {
                subgraphTable[subgraphId].first.push_back(chan);
            }
        }

        for (auto& chan : consumerChan)
        {
            if (count(subgraphTable[subgraphId].second.begin(), subgraphTable[subgraphId].second.end(), chan) == 0)
            {
                subgraphTable[subgraphId].second.push_back(chan);
            }
        }

        //subgraphTable[subgraphId].first.insert(subgraphTable[subgraphId].first.end(), producerChan.begin(), producerChan.end());
        //subgraphTable[subgraphId].second.insert(subgraphTable[subgraphId].second.end(), consumerChan.begin(), consumerChan.end());
    }
}

void GraphScheduler::addDivergenceSubgraph(uint64_t subgraphId, vector<ChanDGSF*> commonChan, vector<ChanDGSF*> truePathChan, vector<ChanDGSF*> falsePathChan)
{
    if (divergenceGraph.find(subgraphId) == divergenceGraph.end())
    {
        vector<vector<ChanDGSF*>> diverGraph = { commonChan, truePathChan, falsePathChan };
        divergenceGraph.insert(make_pair(subgraphId, diverGraph));
    }
    else
    {
        for (auto& chan : commonChan)
        {
            if (count(divergenceGraph[subgraphId][0].begin(), divergenceGraph[subgraphId][0].end(), chan) == 0)
            {
                divergenceGraph[subgraphId][0].push_back(chan);
            }
        }

        for (auto& chan : truePathChan)
        {
            if (count(divergenceGraph[subgraphId][1].begin(), divergenceGraph[subgraphId][1].end(), chan) == 0)
            {
                divergenceGraph[subgraphId][1].push_back(chan);
            }
        }

        for (auto& chan : falsePathChan)
        {
            if (count(divergenceGraph[subgraphId][2].begin(), divergenceGraph[subgraphId][2].end(), chan) == 0)
            {
                divergenceGraph[subgraphId][2].push_back(chan);
            }
        }

        //divergenceGraph[subgraphId][0].insert(divergenceGraph[subgraphId][0].end(), commonChan.begin(), commonChan.end());
        //divergenceGraph[subgraphId][1].insert(divergenceGraph[subgraphId][1].end(), truePathChan.begin(), truePathChan.end());
        //divergenceGraph[subgraphId][2].insert(divergenceGraph[subgraphId][2].end(), falsePathChan.begin(), falsePathChan.end());
    }
}

void GraphScheduler::graphUpdate()
{
    uint64_t graphSize = subgraphTable.size();
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

            // Debug_yin_21.06.23
            if (divergenceGraph[currSubgraphId][1].empty() && divergenceGraph[currSubgraphId][2].empty())
            {
                consumerChanFinish = commonChanIsFinish;
            }
            else
            {
                consumerChanFinish = commonChanIsFinish && (truePathChanIsFinish || falsePathChanGetIsFinish);
            }
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
        if (graphSwitchO3)
        {
            currSubgraphId = selectSubgraphO3(currSubgraphId);
        }
        else
        {
            currSubgraphId = selectSubgraphInOrder(currSubgraphId);
        }
        // Config all channels in new subgraph
        configChan(currSubgraphId);
        // Clear the chanDataCnt of subgraph's producer channels
        resetSubgraph(currSubgraphId);

        subgraphTimeout = 0;  // Reset timeout counter
    }
    else
    {
        ++subgraphTimeout;
    }

    ++subgraphActiveCnt[currSubgraphId];

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
void GraphScheduler::configChan(uint64_t subgraphId)
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
                    /*chan->popChannelEnable = 0;*/
                    if (chan->keepMode && !chan->downstream[0]->isLoopVar)
                    {
                        chan->popChannelEnable = 1;
                    }
                    else
                    {
                        chan->popChannelEnable = 0;
                    }
                }

                for (auto& chan : subgraphTable[id].second)
                {
                    /*chan->pushBufferEnable = 1;
                    chan->popChannelEnable = 0;*/

                    // Debug_yin_21.06.10
                    chan->pushBufferEnable = 1;
                    if (chan->keepMode && !chan->downstream[0]->isLoopVar)
                    {
                        chan->popChannelEnable = 1;
                    }
                    else
                    {
                        chan->popChannelEnable = 0;
                    }
                }
            }
            else
            {
                for (auto& chan : subgraphTable[id].second)
                {
                    /*chan->pushBufferEnable = 0;
                    chan->popChannelEnable = 0;*/

                    // Debug_yin_21.06.10
                    chan->pushBufferEnable = 0;
                    if (chan->keepMode && !chan->downstream[0]->isLoopVar)
                    {
                        chan->popChannelEnable = 1;
                    }
                    else
                    {
                        chan->popChannelEnable = 0;
                    }
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

uint64_t GraphScheduler::selectSubgraphInOrder(uint64_t _currSubgraphId)
{
    //return (++_currSubgraphId) % subgraphTable.size();

    // Debug_yin_21.06.30
    for (size_t i = 0; i < subgraphTable.size(); ++i)
    {
        uint64_t subgraphId = (_currSubgraphId + 1 + i) % subgraphTable.size();
        if (checkProducerDownstreamChanAllEnable(subgraphTable[subgraphId].first))
        {
            return subgraphId;
        }
    }

    Debug::throwError("Not find a enable subgraph!", __FILE__, __LINE__);
}

//uint64_t GraphScheduler::selectSubgraphO3(uint64_t _currSubgraphId)
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

uint64_t GraphScheduler::selectSubgraphO3(uint64_t _currSubgraphId)
{
    for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size(); ++subgraphCnt)
    {
        uint64_t subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
        if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
        {
           /* if (checkProducerChanIsFull(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }

            if (checkProducerChanNotEmpty(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }*/

            // Debug_yin_21.06.30
            if (checkProducerChanIsFull(subgraphTable[subgraphId].first)
                && checkProducerDownstreamChanAllEnable(subgraphTable[subgraphId].first)
                && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }

            if (checkProducerChanNotEmpty(subgraphTable[subgraphId].first) 
                && checkProducerDownstreamChanAllEnable(subgraphTable[subgraphId].first)
                && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
            {
                return subgraphId;
            }
        }
    }

    // Select a non-over subgraph
    if (subgraphIsOver[_currSubgraphId])
    {
        for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size(); ++subgraphCnt)
        {
            uint64_t subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
            if (!subgraphIsOver[subgraphId])
            {
                return subgraphId;
            }
        }
    }

    //for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size(); ++subgraphCnt)
    //{
    //    uint64_t subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
    //    if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
    //    {
    //        if (checkProducerChanNotEmpty(subgraphTable[subgraphId].first) && checkConsumerChanNotFull(subgraphTable[subgraphId].second))
    //        {
    //            return subgraphId;
    //        }
    //    }
    //}

    //if (subgraphTimeout < DGSF_INPUT_BUFF_SIZE * 20)
    //{
    //    return currSubgraphId;
    //}
    //else
    //{
    //    // Debug_yin_21.06.01 (Disable try another subgraph randomly)
    //    // Select a subgraph to execute in round-robin 
    //    for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size() + 1; ++subgraphCnt)
    //    {
    //        uint64_t subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
    //        if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
    //        {
    //            return subgraphId;  // Return a subgraph randomly
    //        }
    //    }
    //}

    return currSubgraphId;

    //// Debug_yin_21.06.01
    //// Select a subgraph to execute in round-robin 
    //for (size_t subgraphCnt = 1; subgraphCnt < subgraphTable.size() + 1; ++subgraphCnt)
    //{
    //    uint64_t subgraphId = (_currSubgraphId + subgraphCnt) % subgraphTable.size();
    //    if (!subgraphIsOver[subgraphId]/* && subgraphId != _currSubgraphId*/)
    //    {
    //        return subgraphId;  // Return a subgraph randomly
    //    }
    //}

    //Debug::throwError("Can not find a subgraph to execute. Program should be already finished!", __FILE__, __LINE__);
}

bool GraphScheduler::checkSubgraphIsOver(uint64_t _subgraphId)
{
    bool isOver = 1;

    if (!subgraphTable[_subgraphId].second.empty())
    {
        if (divergenceGraph.count(currSubgraphId) == 0)
        {
            //isOver = checkConsumerChanGetLastData(subgraphTable[_subgraphId].second);

            // Debug_yin_21.06.24
            isOver = checkConsumerChanGetLastData(subgraphTable[_subgraphId].second) && checkProducerChanAllEmpty(subgraphTable[_subgraphId].first);

            //// Debug_yin_21.06.24
            //isOver = checkConsumerChanGetLastData(subgraphTable[_subgraphId].second) 
            //    && checkConsumerChanIsEmpty(subgraphTable[_subgraphId].second)
            //    && checkProducerChanAllEmpty(subgraphTable[_subgraphId].first);
        }
        else
        {
            bool commonChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][0]);
            bool truePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][1]);
            bool falsePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][2]);

            /*bool commonChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][0]) && checkConsumerChanIsEmpty(divergenceGraph[currSubgraphId][0]);
            bool truePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][1]) && checkConsumerChanIsEmpty(divergenceGraph[currSubgraphId][1]);
            bool falsePathChanIsOver = checkConsumerChanGetLastData(divergenceGraph[currSubgraphId][2]) && checkConsumerChanIsEmpty(divergenceGraph[currSubgraphId][2]);*/

            if (divergenceGraph[currSubgraphId][1].empty() && divergenceGraph[currSubgraphId][2].empty())
            {
                isOver = commonChanIsOver;
            }
            else
            {
                isOver = commonChanIsOver && (truePathChanIsOver || falsePathChanIsOver);
            }
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
        if (!chan->getTheLastData.front())  // If has received the last data, regard as channel is full
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

bool GraphScheduler::checkProducerDownstreamChanAllEnable(vector<ChanDGSF*> producerChans)
{
    bool allEnable = 1;

    for (auto& chan : producerChans)
    {
        for (auto& downstream : chan->downstream)
        {
            if (!downstream->enable)
            {
                allEnable = 0;
                break;
            }
        }
    }

    return allEnable;
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

bool GraphScheduler::checkConsumerChanIsEmpty(vector<ChanDGSF*> consumerChans)
{
    bool isEmpty = 1;

    if (!consumerChans.empty())
    {
        for (auto& chan : consumerChans)
        {
            for (auto& buffer : chan->chanBuffer)
            {
                if (!buffer.empty())
                {
                    isEmpty = 0;
                    return isEmpty;
                }
            }
        }
    }
    else
    {
        isEmpty = 1;
    }

    return isEmpty;
}

void GraphScheduler::resetSubgraph(uint64_t _subgraphId)
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

    if (!subgraphTable[currSubgraphId].second.empty())
    {
        for (auto& chan : producerChans)
        {
            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                if (!(chan->chanBuffer[bufferId].empty() || chan->chanDataCnt < Global::DGSF_input_buffer_size))
                {
                    finish = 0;
                    break;
                }
                //else
                //{
                //    // If producer channel has sent enough data, disable popChannel
                //    chan->popChannelEnable = 0;
                //}
            }

            if (!finish)
            {
                break;
            }
        }
    }
    else
    {
        for (auto& chan : producerChans)
        {
            for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
            {
                if (!chan->chanBuffer[bufferId].empty())
                {
                    finish = 0;
                    break;
                }
                //else
                //{
                //    // If producer channel has sent enough data, disable popChannel
                //    chan->popChannelEnable = 0;
                //}
            }

            if (!finish)
            {
                break;
            }
        }
    }

    //if (subgraphTimeout > DGSF_INPUT_BUFF_SIZE * 2)
    //{
    //    for (auto& chan : producerChans)
    //    {
    //        for (size_t bufferId = 0; bufferId < chan->chanBuffer.size(); ++bufferId)
    //        {
    //            if (chan->chanBuffer[bufferId].empty() || chan->chanDataCnt >= DGSF_INPUT_BUFF_SIZE)
    //            {
    //                finish = 1;
    //                // If producer channel has sent enough data, disable popChannel
    //                chan->popChannelEnable = 0;
    //            }
    //        }
    //    }

    //    return finish;
    //}

    return finish;
}

bool GraphScheduler::checkConsumerChanFinish(vector<ChanDGSF*> consumerChans)
{
    bool finish = 1;

    if (!consumerChans.empty())
    {
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
                else if (!chan->getTheLastData.front())  // The number of ChanDGSF's upstreams is limited to 1
                {
                    finish = 0;
                    break;
                }
            }

            if (finish == 0)
            {
                break;
            }
        }
    }
    else
    {
        finish = 0;
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
    if (!consumerChans.empty())
    {
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
    }
    else
    {
        getLast = 0;
    }

    return getLast;
}

#ifdef DEBUG_MODE 
void GraphScheduler::switchGraphManually()
{
    subgraphTimeout = 0;
    subgraphIsOver[currSubgraphId] = checkSubgraphIsOver(currSubgraphId);
    // Select a new subgraph
    currSubgraphId = selectSubgraphInOrder(currSubgraphId);
    //currSubgraphId = selectSubgraphO3(currSubgraphId);
    // Config all channels in new subgraph
    configChan(currSubgraphId);
    // Clear the chanDataCnt of subgraph's producer channels
    resetSubgraph(currSubgraphId);
}
#endif