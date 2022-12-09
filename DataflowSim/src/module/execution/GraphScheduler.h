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
        void addSubgraph(uint64_t subgraphId, vector<ChanDGSF*> producerChan, vector<ChanDGSF*> consumerChan);
        void addDivergenceSubgraph(uint64_t subgraphId, vector<ChanDGSF*> commonChan, vector<ChanDGSF*> truePathChan, vector<ChanDGSF*> falsePathChan);

    private:
        void configChan(uint64_t subgraphId);
        uint64_t selectSubgraphInOrder(uint64_t _currSubgraphId);
        uint64_t selectSubgraphO3(uint64_t _currSubgraphId);
        void resetSubgraph(uint64_t _subgraphId);  // Clear chanDataCnt in each producer channel
        bool checkSubgraphIsOver(uint64_t _subgraphId);  // Check whether this subgraph is over
        bool checkProducerChanIsFull(vector<ChanDGSF*> producerChans);
        bool checkProducerChanNotEmpty(vector<ChanDGSF*> producerChans);
        bool checkProducerChanAllEmpty(vector<ChanDGSF*> producerChans);
        bool checkProducerDownstreamChanAllEnable(vector<ChanDGSF*> producerChans);
        bool checkConsumerChanNotFull(vector<ChanDGSF*> consumerChans);
        bool checkConsumerChanIsEmpty(vector<ChanDGSF*> consumerChans);
        bool checkProducerChanFinish(vector<ChanDGSF*> producerChans);
        bool checkConsumerChanFinish(vector<ChanDGSF*> consumerChans);
        bool checkConsumerChanGetLastData(vector<ChanDGSF*> consumerChans);

    public:
        static uint64_t currSubgraphId;  // Current subgraph Id, default start at 0;
       
    public:
        // <subgraphId, pair<producer channels, consumer channels>>
        unordered_map<uint64_t, pair<vector<ChanDGSF*>, vector<ChanDGSF*>>> subgraphTable;
        unordered_map<uint64_t, vector<vector<ChanDGSF*>>> divergenceGraph;  // Store branch-divergence subgraph
        deque<bool> subgraphIsOver;
        uint64_t subgraphTimeout = 0;
        vector<uint64_t> subgraphActiveCnt;  // Count the active cycle of each subgraph
        bool graphSwitchO3 = 1;

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        inline const deque<bool>& getSubgraphStatus() const
        {
            return subgraphIsOver;
        }

        void switchGraphManually();
#endif // DEBUG_MODE 
    };
}