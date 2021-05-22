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
        string controlRegionName;
        vector<string> pre_nodes_data;
        vector<string> next_nodes_data;
        vector<string> pre_nodes_active;
        vector<string> next_nodes_active;

        virtual void fakeFunc() {};
    };

    struct Dfg_Node : public Node
    {
        Dfg_Node(string _nodeName, string _node_op) : Node(_nodeName), node_op(_node_op) {}
        //Dfg_Node(string _nodeName, string _node_op, int _constVal) : Node(_nodeName), node_op(_node_op), constVal(_constVal) {}
        string node_type = "Normal";  // Normal, Const, Lc, Mux, MuxParitial
        string node_op;  // normal op, Loop_head, Load, Store, Const
        //tuple<string, string, string, string> loop_info;  // {init, boundary, operation, next}; Only when this node is a Lc
        
        int constVal = 0;  // For constant

        // For load/store
        uint baseAddr = 0;
        vector<int>* memorySpace = nullptr;  // For load/store node
    };

    struct Chan_Node : public Node
    {
        Chan_Node(string _nodeName) : Node(_nodeName) {}
        string node_type = "ChanBase";  // Lc, Mux, ChanPartialMux, ChanBase, ChanDGSF, Lse_ld, Lse_st
        string node_op;  // Inherit from Dfg_Node 
        string chan_mode = "Normal";  // Normal, Keep_mode, Drain_mode, Fake_mode(e.g. Relay_mode)
        
        uint size = 2;
        uint speedup = 1;
        uint cycle = 0;

        // For constant
        vector<int> constVal;
        uint constLocation = 0;  // 0 => const Sub a; 1 => a Sub const

        // For load/store
        uint baseAddr = 0;  
        vector<int>* memorySpace = nullptr;
    };

    class Graph
    {
    public:
        Graph();
        ~Graph();
        //void addNode(const string& _nodeName);
        Node* getNode(const string& _nodeName);
        void addPreNodesData(const string& _nodeName, const vector<string>& _pre_nodes_data);
        void addNextNodesData(const string& _nodeName, const vector<string>& _next_nodes_data);
        void addPreNodesActive(const string& _nodeName, const vector<string>& _pre_nodes_active);
        void addNextNodesActive(const string& _nodeName, const vector<string>& _next_nodes_active);
        void completeConnect();
        void removeRedundantConnect();
        auto findNodeIndex(const string& _nodeName)->unordered_map<string, uint>::iterator;
        bool findNode(const string& _nodeName);
        void addNodes2CtrlTree(const string& targetCtrlRegion, const vector<string>& nodes_);

    protected:
        void plotDot(std::fstream& fileName_, ControlTree& _controlTree);
        void printDotNodeConnect(std::fstream& fileName_);
        virtual void printDotNodeLabel(std::fstream& fileName_) = 0;
        //vector<pair<string, uint>> traverseControlRegionsDfs(ControlTree& _controlTree);  // pair<controlRegionName, level>
        void printDotControlRegion(std::fstream& fileName_, ControlTree& _controlTree);
        //void printSubgraphDot(std::fstream& fileName_, string& controlRegionName_, vector<string>& nodes_, string& controlType_);

    public:
        deque<Node*> nodes;
        unordered_map<string, uint> nodeIndexDict;
        ControlTree controlTree;
    };

    class Dfg : public Graph
    {
    public: 
        Dfg();
        void addNode(const string& _nodeName, const string& _nodeOp);
        void addNode(const string& _nodeName, const string& _nodeOp, const int& _constVal);
        void addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodes);
        void addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodesData, const vector<string>& _preNodesActive);
        void addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodes, vector<int>* memorySpace_, const uint& baseAddr_);
        //void addNode(const string& _nodeName, const string& _nodeOp, const string& _preNode);
        void setTheTailNode(const string& targetCtrlRegion, string nodeName);
        void plotDot();

    private:
        void printDotNodeLabel(std::fstream& fileName_) override;

    /*public:
        ControlTree controlTree;*/

    private:
        std::fstream dfg_dot;
        
    };

    class ChanGraph : public Graph
    {
    public:
        ChanGraph();
        ChanGraph(Dfg& dfg_);
        void initial();
        void addNode(const string& _nodeName);
        void addNode(const string& _nodeName, const string& _nodeType, const string& _nodeOp, const string& _chanMode);
        void addNode(const string& _nodeName, const string& _nodeType, const string& _nodeOp, const string& _chanMode, const string& _ctrlRegion);
        void plotDot();
        void genChanGraphFromDfg(Dfg& dfg_);  // Generate chanGraph from DFG
        void addSpecialModeChan();  // Add relayMode and drainMode channel
        void addNodeDelay();
        void pathBalance();
        
        vector<string> bfsTraverseControlTree(ControlTree& ctrlTree);
        vector<string> findShortestCtrlRegionPath(vector<string>& _preNodeCtrlRegionPath, vector<string>& _nextNodeCtrlRegionPath);
        vector<string> backTrackPath(string& nodeName, vector<ControlRegion>& loopHierarchy);
        vector<ControlRegion> genLoopHierarchy(ControlTree& _controlTree);
        string findNodeCtrlRegionInLoopHierarchy(string _nodeName, vector<ControlRegion> _loopHierarchy);
        void insertChanNode(Chan_Node& chanNode, vector<string> preNodes, vector<string> nextNodes);  // Add a chanNode between preNodes and nextNodes (at least one of them's size must equal to one)
        //string findCtrlRegion(string& ctrlRegionName, vector<ControlRegion>& loopHierarchy);
        vector<string> bfsTraverseNode();  // Generate simulation sequence
 
    private:
        void printDotNodeLabel(std::fstream& fileName_) override;

   /* public:
        ControlTree controlTree;*/

    private:
        std::fstream chan_graph_dot;
    };
}