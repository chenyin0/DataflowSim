#pragma once
#include "../define/Define.hpp"
#include "./control_tree.h"

namespace DFSim
{
    struct Node
    {
        Node(string _nodeName) : node_name(_nodeName) {}
        string node_name;
        /*string node_type;
        string node_op;*/
        vector<string> pre_nodes_data;
        vector<string> next_nodes_data;
        vector<string> pre_nodes_active;
        vector<string> next_nodes_active;

        virtual void fakeFunc() {};
    };

    struct Dfg_Node : public Node
    {
        Dfg_Node(string _nodeName, string _node_op) : Node(_nodeName), node_op(_node_op) {}
        string node_type;  // Normal, Const, Lc
        string node_op;  // normal op, loop_head, load, store, null
        tuple<string, string, string, string> loop_info;  // {init, boundary, operation, next}; Only when this node is a Lc
        double const_num = 0;
    };

    struct Chan_Node : public Node
    {
        Chan_Node(string _nodeName) : Node(_nodeName) {}
        string node_type;  // Lc, Mux, chanBase, chanDGSF
        string node_op;  // Normal, Keep_mode, Drain_mode
        
        uint size = 1;
        uint speedup = 1;
        uint cycle = 0;
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
        void plotDot(std::fstream& fileName_, ControlTree& _controlTree);
        void printDotNodeConnect(std::fstream& fileName_);
        virtual void printDotNodeLabel(std::fstream& fileName_) = 0;
        //vector<pair<string, uint>> traverseControlRegionsDfs(ControlTree& _controlTree);  // pair<controlRegionName, level>
        void printDotControlRegion(std::fstream& fileName_, ControlTree& _controlTree);
        //void printSubgraphDot(std::fstream& fileName_, string& controlRegionName_, vector<string>& nodes_, string& controlType_);

    protected:
        vector<Node*> nodes;
        unordered_map<string, uint> nodeIndexDict;
    };

    class Dfg : public Graph
    {
    public: 
        Dfg();
        void addNode(const string& _nodeName, const string& _nodeOp) override;
        void plotDot(ControlTree& _controlTree);

    private:
        void printDotNodeLabel(std::fstream& fileName_) override;

    private:
        std::fstream dfg_dot;
    };

    class ChanGraph : public Graph
    {
    public:
        ChanGraph();
        ChanGraph(Dfg& dfg_);  // Generate chanGraph from DFG
        void addNode(const string& _nodeName);
        void plotDot(ControlTree& _controlTree);

    private:
        void printDotNodeLabel(std::fstream& fileName_) override;

    private:
        std::fstream chan_graph_dot;
    };
}