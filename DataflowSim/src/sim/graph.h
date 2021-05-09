#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
    struct Node
    {
        Node(string _nodeName) : node_name(_nodeName) {}
        string node_name;
        vector<string> pre_nodes_data;
        vector<string> next_nodes_data;
        vector<string> pre_nodes_active;
        vector<string> next_nodes_active;
    };

    struct Dfg_Node : public Node
    {
        Dfg_Node(string _nodeName, string _node_op) : Node(_nodeName), node_op(_node_op) {}
        string node_type;  // normal, const, lc
        string node_op;
        tuple<string, string, string, string> loop_info;  // {init, boundary, operation, next}; Only when this node is a Lc
        double const_num = 0;
    };

    struct Chan_Node : public Node
    {
        Chan_Node(string _nodeName) : Node(_nodeName) {}
        string node_type;  // chanBase, chanDGSF
    };

    class Graph
    {
    public:
        Graph();
        ~Graph();
        virtual void addNode(const string& _nodeName, const string& _node_op) = 0;
        void addPreNodesData(const string& _nodeName, const vector<string>& _pre_nodes_data);
        void addNextNodesData(const string& _nodeName, const vector<string>& _next_nodes_data);
        void addPreNodesActive(const string& _nodeName, const vector<string>& _pre_nodes_active);
        void addNextNodesActive(const string& _nodeName, const vector<string>& _next_nodes_active);
        void completeConnect();
        void removeRedundantConnect();
        auto findNodeIndex(const string& _nodeName)->unordered_map<string, uint>::iterator;

    protected:
        vector<Node*> nodes;
        unordered_map<string, uint> nodeIndexDict;
    };

    class Dfg : public Graph
    {
    public: 
        Dfg();
        void addNode(const string& _nodeName, const string& _nodeOp) override;
    };

    class ChanGraph : public Graph
    {
    public:
        ChanGraph();
        void addNode(const string& _nodeName);
    };
}