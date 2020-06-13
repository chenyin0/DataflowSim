#pragma once
#include "../../src/define/Define.hpp"
#include "../../src/module/mem/MemSystem.h"
#include "../../src/module/execution/Channel.h"
#include "../../src/module/execution/Lc.h"
#include "../../src/module/execution/Mux.h"
#include "../../src/module/ClkSys.h"
#include "../../src/sim/Debug.h"
#include "../../src/module/Registry.h"

/*
Src code:

#define Q_PUSH(node) { queue[q_in==0?N_NODES-1:q_in-1]=node; q_in=(q_in+1)%N_NODES; }
#define Q_PEEK() (queue[q_out])
#define Q_POP() { q_out = (q_out+1)%N_NODES; }
#define Q_EMPTY() (q_in>q_out ? q_in==q_out+1 : (q_in==0)&&(q_out==N_NODES-1))

void bfs(node_t nodes[N_NODES], edge_t edges[N_EDGES],
    node_index_t starting_node, level_t level[N_NODES],
    edge_index_t level_counts[N_LEVELS])
{
    node_index_t queue[N_NODES];
    node_index_t q_in, q_out;
    node_index_t dummy;
    node_index_t n;
    edge_index_t e;

    /*init_levels: for( n=0; n<N_NODES; n++ )*/
    /*level[n] = MAX_LEVEL;*/
    /*init_horizons: for( i=0; i<N_LEVELS; i++ )*/
    /*level_counts[i] = 0;*/
/*
    q_in = 1;
    q_out = 0;
    level[starting_node] = 0;
    level_counts[0] = 1;
    Q_PUSH(starting_node);

loop_queue: for (dummy = 0; dummy < N_NODES; dummy++) { // Typically while(not_empty(queue)){
    if (Q_EMPTY())
        break;
    n = Q_PEEK();
    Q_POP();
    edge_index_t tmp_begin = nodes[n].edge_begin;
    edge_index_t tmp_end = nodes[n].edge_end;

loop_neighbors: for (e = tmp_begin; e < tmp_end; e++) {
    node_index_t tmp_dst = edges[e].dst;
    level_t tmp_level = level[tmp_dst];

    if (tmp_level == MAX_LEVEL) { // Unmarked
        level_t tmp_level = level[n] + 1;
        level[tmp_dst] = tmp_level;
        ++level_counts[tmp_level];
        Q_PUSH(tmp_dst);
    }
}
}

*/

namespace DFSimTest
{
    using namespace DFSim;

    struct Node_t
    {
        uint begin;
        uint end;
    };

    struct Edge_t
    {
        uint dst;
    };

    class BfsTest
    {
    public:
        static void bfs_SGMF(Debug* debug);
        static void generateData();

    private:
        static const uint initialNode;  // Traverse from this node
        static const uint edgeNum;
        static const uint nodeNum;

        static const uint edgeBaseAddr;
        static const uint nodeBaseAddr;
        static const uint levelBaseAddr;

        static vector<int> memData;
        static vector<Edge_t> edges;
        static vector<Node_t> nodes;
        static vector<int> level;
    };
}