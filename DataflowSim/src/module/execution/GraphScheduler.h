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
        void configChan(uint subgraphId);
        void graphUpdate();
        void addSubgraph(uint subgraphId, vector<ChanDGSF*> producerChan, vector<ChanDGSF*> consumerChan);

    //private:
        uint currSubgraphId = 0;  // Current subgraph Id, default start at 0;
        // <subgraphId, pair<producer channels, consumer channels>>
        unordered_map<uint, pair<vector<ChanDGSF*>, vector<ChanDGSF*>>> subgraphTable;  
    };
}