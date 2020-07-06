#pragma once
#include "../../define/Define.hpp"
#include "./Channel.h"

namespace DFSim
{
    class GraphScheduler
    {
    public:
        GraphScheduler();
        void schedulerInit();
        void graphUpdate();
        void addSubgraph(uint subgraphId, vector<ChanDGSF*> producerChan, vector<ChanDGSF*> consumerChan);

    private:
        void configChan(uint subgraphId);
        uint selectSubgraphInOrder(uint _currSubgraphId);
        uint selectSubgraphO3(uint _currSubgraphId);
        void resetSubgraph(uint _subgraphId);  // Clear chanDataCnt in each producer channel
        bool checkSubgraphIsOver(uint _subgraphId);  // Check whether this subgraph is over
        bool checkProducerChanIsFull(uint _subgraphId);
        bool checkProducerChanNotEmpty(uint _subgraphId);
        bool checkConsumerChanNotFull(uint _subgraphId);

    public:
        uint currSubgraphId = 0;  // Current subgraph Id, default start at 0;
       
    private:
        // <subgraphId, pair<producer channels, consumer channels>>
        unordered_map<uint, pair<vector<ChanDGSF*>, vector<ChanDGSF*>>> subgraphTable;
        deque<bool> subgraphIsOver;
    };
}