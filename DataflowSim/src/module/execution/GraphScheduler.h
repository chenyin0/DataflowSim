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
        void addDivergenceSubgraph(uint subgraphId, vector<ChanDGSF*> commonChan, vector<ChanDGSF*> truePathChan, vector<ChanDGSF*> falsePathChan);

    private:
        void configChan(uint subgraphId);
        uint selectSubgraphInOrder(uint _currSubgraphId);
        uint selectSubgraphO3(uint _currSubgraphId);
        void resetSubgraph(uint _subgraphId);  // Clear chanDataCnt in each producer channel
        bool checkSubgraphIsOver(uint _subgraphId);  // Check whether this subgraph is over
        bool checkProducerChanIsFull(vector<ChanDGSF*> producerChans);
        bool checkProducerChanNotEmpty(vector<ChanDGSF*> producerChans);
        bool checkProducerChanAllEmpty(vector<ChanDGSF*> producerChans);
        bool checkConsumerChanNotFull(vector<ChanDGSF*> consumerChans);
        bool checkProducerChanFinish(vector<ChanDGSF*> producerChans);
        bool checkConsumerChanFinish(vector<ChanDGSF*> consumerChans);
        bool checkConsumerChanGetLastData(vector<ChanDGSF*> consumerChans);

    public:
        static uint currSubgraphId;  // Current subgraph Id, default start at 0;
       
    public:
        // <subgraphId, pair<producer channels, consumer channels>>
        unordered_map<uint, pair<vector<ChanDGSF*>, vector<ChanDGSF*>>> subgraphTable;
        unordered_map<uint, vector<vector<ChanDGSF*>>> divergenceGraph;  // Store branch-divergence subgraph
        deque<bool> subgraphIsOver;

    private:
        //uint subgraphTimeout = 0;

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        inline const deque<bool>& getSubgraphStatus() const
        {
            return subgraphIsOver;
        }
#endif // DEBUG_MODE 
    };
}