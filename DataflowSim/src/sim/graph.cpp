#include "./graph.h"
#include "./Debug.h"
#include "../sim/global.h"

using namespace DFSim;

Graph::Graph()
{
}

Graph::~Graph()
{
    for (auto& nodePtr : nodes)
    {
        delete nodePtr;
    }
}

auto Graph::findNodeIndex(const string& _nodeName)->unordered_map<string, uint>::iterator
{
    auto iter = nodeIndexDict.find(_nodeName);
    if (iter == nodeIndexDict.end())
    {
        Debug::throwError("Not find this node!", __FILE__, __LINE__);
    }
    else
    {
        return iter;
    }
}

Node* Graph::getNode(const string& _nodeName)
{
    return nodes[findNodeIndex(_nodeName)->second];
}

void Graph::addPreNodesData(const string& _nodeName, const vector<string>& _pre_nodes_data)
{
    uint index = findNodeIndex(_nodeName)->second;
    nodes[index]->pre_nodes_data.insert(nodes[index]->pre_nodes_data.end(), _pre_nodes_data.begin(), _pre_nodes_data.end());
}

void Graph::addNextNodesData(const string& _nodeName, const vector<string>& _next_nodes_data)
{
    uint index = findNodeIndex(_nodeName)->second;
    nodes[index]->next_nodes_data.insert(nodes[index]->next_nodes_data.end(), _next_nodes_data.begin(), _next_nodes_data.end());
}

void Graph::addPreNodesActive(const string& _nodeName, const vector<string>& _pre_nodes_active)
{
    uint index = findNodeIndex(_nodeName)->second;
    nodes[index]->pre_nodes_active.insert(nodes[index]->pre_nodes_active.end(), _pre_nodes_active.begin(), _pre_nodes_active.end());
}

void Graph::addNextNodesActive(const string& _nodeName, const vector<string>& _next_nodes_active)
{
    uint index = findNodeIndex(_nodeName)->second;
    nodes[index]->next_nodes_active.insert(nodes[index]->next_nodes_active.end(), _next_nodes_active.begin(), _next_nodes_active.end());
}

void Graph::completeConnect()
{
    for (auto& node : nodes)
    {
        for (auto& pre_node : node->pre_nodes_data)
        {
            nodes[findNodeIndex(pre_node)->second]->next_nodes_data.push_back(node->node_name);
        }
    }

    for (auto& node : nodes)
    {
        for (auto& next_node : node->next_nodes_data)
        {
            nodes[findNodeIndex(next_node)->second]->pre_nodes_data.push_back(node->node_name);
        }
    }

    for (auto& node : nodes)
    {
        for (auto& pre_node : node->pre_nodes_active)
        {
            nodes[findNodeIndex(pre_node)->second]->next_nodes_active.push_back(node->node_name);
        }
    }

    for (auto& node : nodes)
    {
        for (auto& next_node : node->next_nodes_active)
        {
            nodes[findNodeIndex(next_node)->second]->pre_nodes_active.push_back(node->node_name);
        }
    }
}

void Graph::removeRedundantConnect()
{
    for (auto& node : nodes)
    {
        set<string> s1(node->pre_nodes_data.begin(), node->pre_nodes_data.end());
        node->pre_nodes_data.assign(s1.begin(), s1.end());

        set<string> s2(node->next_nodes_data.begin(), node->next_nodes_data.end());
        node->next_nodes_data.assign(s2.begin(), s2.end());

        set<string> s3(node->pre_nodes_active.begin(), node->pre_nodes_active.end());
        node->pre_nodes_active.assign(s3.begin(), s3.end());

        set<string> s4(node->next_nodes_active.begin(), node->next_nodes_active.end());
        node->next_nodes_active.assign(s4.begin(), s4.end());
    }
}

void Graph::plotDot(std::fstream& fileName_, ControlTree& _controlTree)
{
    fileName_ << "digraph dfg{" << std::endl;

    printDotNodeConnect(fileName_);
    printDotNodeLabel(fileName_);
    printDotControlRegion(fileName_, _controlTree);

    fileName_ << "}" << std::endl;
    fileName_.close();
}

void Graph::printDotNodeConnect(std::fstream& fileName_)
{
    // Plot data connect
    for (auto& node : nodes)
    {
        for (size_t i = 0; i < node->next_nodes_data.size(); ++i)
        {
            fileName_ << node->node_name << "->" << node->next_nodes_data[i] << " [label=" << i << "]" << std::endl;
        }
    }

    // Plot active connect
    for (auto& node : nodes)
    {
        for (size_t i = 0; i < node->next_nodes_active.size(); ++i)
        {
            fileName_ << node->node_name << "->" << node->next_nodes_active[i] << " [label=" << i << ", style = \"dashed\"]" << std::endl;
        }
    }
}

//vector<pair<string, uint>> Graph::traverseControlRegionsDfs(ControlTree& _controlTree)
//{
//    auto& _controlRegionTable = _controlTree.controlRegionTable;
//    vector<pair<string, uint>> controlRegionsDfs = { make_pair(_controlRegionTable[0].controlRegionName, 0) };  // pair<controlRegionName, level>
//    //vector<string> controlRegionQueue = { control_tree[0].control_name };
//    for (size_t ptr = 0; ptr < controlRegionsDfs.size(); ++ptr)
//    {
//        vector<string> lowerControlRegions = _controlRegionTable[(_controlTree.findControlRegionIndex(controlRegionsDfs[ptr].first))->second].lowerControlRegion;
//        vector<pair<string, uint>> tmp;
//        uint level = controlRegionsDfs[ptr].second + 1;
//        for (auto& controlRegionName : lowerControlRegions)
//        {
//            tmp.push_back(make_pair(controlRegionName, level));
//        }
//        controlRegionsDfs.insert(controlRegionsDfs.begin() + ptr + 1, tmp.begin(), tmp.end());
//    }
//
//    return controlRegionsDfs;
//}

void Graph::printDotControlRegion(std::fstream& fileName_, ControlTree& _controlTree)
{
    vector<pair<string, uint>> controlRegions = _controlTree.traverseControlRegionsDfs();

    for (size_t i = 0; i < controlRegions.size(); ++i)
    {
        string controlRegionName = controlRegions[i].first;
        _controlTree.printSubgraphDot(fileName_, controlRegionName);

        // Print "}" for the last controlRegion
        if (i != controlRegions.size() - 1)
        {
            if (!controlRegions[i].second < controlRegions[i + 1].second)
            {
                uint levelDiff = controlRegions[i].second - controlRegions[i + 1].second;
                for (size_t i = 0; i < levelDiff + 1; ++i)
                {
                    fileName_ << "}";
                }
                fileName_ << std::endl;
            }
        }
        else
        {
            uint levelDiff = controlRegions[i].second;
            for (size_t i = 0; i < levelDiff + 1; ++i)
            {
                fileName_ << "}";
            }
            fileName_ << std::endl;
        }
    }
}

//void Graph::addNode(const string& _nodeName)
//{
//    for (auto& node : nodes)
//    {
//        if (nodeIndexDict.count(node->node_name))
//        {
//            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
//        }
//        else
//        {
//            nodeIndexDict.insert(pair<string, uint>(_nodeName, nodeIndexDict.size()));
//            auto chanNode = new Chan_Node(_nodeName);
//            nodes.push_back(chanNode);
//        }
//    }
//}

//void Graph::printSubgraphDot(std::fstream& fileName_, string& controlRegionName_, vector<string>& nodes_, string& controlType_)
//{
//    fileName_ << std::endl;
//    fileName_ << "subgraph cluster_" << controlRegionName_ << " {" << std::endl;
//    for (size_t i = 0; i < nodes_.size(); ++i)
//    {
//        if (i != nodes_.size() - 1)
//        {
//            fileName_ << nodes_[i] << ", ";
//        }
//        else
//        {
//            fileName_ << nodes_[i] << std::endl;
//        }
//    }
//
//    if (controlType_ == "Loop")
//    {
//        fileName_ << "color=darkorange" << std::endl;
//    }
//    else if (controlType_ == "Branch")
//    {
//        fileName_ << "color=dodgerblue2" << std::endl;
//    }
//
//    fileName_ << "label=\"" << controlRegionName_ << "\"" << std::endl;  // Not print "}" here for nested subgraphs
//}


//** Dfg
Dfg::Dfg()
{
    dfg_dot.open(Global::file_path + "dfg.dot", ios::out);
}

void Dfg::addNode(const string& _nodeName, const string& _nodeOp)
{
    if (nodeIndexDict.count(_nodeName))
    {
        Debug::throwError("There already has a same name node!", __FILE__, __LINE__);
    }
    else
    {
        nodeIndexDict.insert(pair<string, uint>(_nodeName, nodeIndexDict.size()));
        Dfg_Node* dfgNode = new Dfg_Node(_nodeName, _nodeOp);
        if (_nodeOp == "Loop_head")
        {
            dfgNode->node_type = "Lc";
        }
        else if (_nodeOp == "Const")
        {
            dfgNode->node_type = "Const";
        }
        nodes.push_back(dfgNode);
    }
}

void Dfg::addNode(const string& _nodeName, const string& _nodeOp, const int& _constVal)
{
    addNode(_nodeName, _nodeOp);
    dynamic_cast<Dfg_Node*>(getNode(_nodeName))->constVal = _constVal;
}

void Dfg::addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodes)
{
    addNode(_nodeName, _nodeOp);
    auto& pre_nodes_data_ = dynamic_cast<Dfg_Node*>(getNode(_nodeName))->pre_nodes_data;
    pre_nodes_data_.insert(pre_nodes_data_.end(), _preNodes.begin(), _preNodes.end());
}

void Dfg::addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodesData, const vector<string>& _preNodesActive)
{
    addNode(_nodeName, _nodeOp, _preNodesData);
    auto& pre_nodes_active_ = dynamic_cast<Dfg_Node*>(getNode(_nodeName))->pre_nodes_active;
    pre_nodes_active_.insert(pre_nodes_active_.end(), _preNodesActive.begin(), _preNodesActive.end());
}

void Dfg::addNode(const string& _nodeName, const string& _nodeOp, const vector<string>& _preNodes, vector<int>* memorySpace_, const uint& baseAddr_)
{
    if (_nodeOp == "Load" || _nodeOp == "Store")
    {
        addNode(_nodeName, _nodeOp, _preNodes);
        dynamic_cast<Dfg_Node*>(getNode(_nodeName))->memorySpace = memorySpace_;
        dynamic_cast<Dfg_Node*>(getNode(_nodeName))->baseAddr = baseAddr_;
    }
    else
    {
        Debug::throwError("This node is not a Load/Store node!", __FILE__, __LINE__);
    }
}

//void Dfg::addNode(const string& _nodeName, const string& _nodeOp, const string& _preNode)
//{
//    addNode(_nodeName, _nodeOp);
//    dynamic_cast<Dfg_Node*>(getNode(_nodeName))->pre_nodes_data.push_back(_preNode);
//}

void Dfg::printDotNodeLabel(std::fstream& fileName_)
{
    string label;
    for (auto& node : nodes)
    {
        string& nodeName = dynamic_cast<Dfg_Node*>(node)->node_name;
        string& nodeOp = dynamic_cast<Dfg_Node*>(node)->node_op;
        string& nodeType = dynamic_cast<Dfg_Node*>(node)->node_type;

        if (nodeType == "Normal" || nodeType == "Lc")
        {
            label = nodeName + " [label=\"node_" + nodeName + "\\n" + nodeOp + "\"";

            if (nodeOp == "load" || nodeOp == "store")
            {
                label += ", fillcolor=\"chartreuse2\", style=filled";
            }

            label += "]";
        }
        else
        {
            if (nodeType == "Const")
            {
                label = nodeName + " [shape=box, fillcolor=\"khaki2\", style=filled]";
            }
        }     

        fileName_ << label << std::endl;
    }
}

void Dfg::plotDot()
{
    Graph::plotDot(dfg_dot, controlTree);
}


//** ChanGraph
ChanGraph::ChanGraph()
{
    initial();
}

ChanGraph::ChanGraph(Dfg& dfg_)
{
    initial();
    genChanGraphFromDfg(dfg_);
}

void ChanGraph::initial()
{
    chan_graph_dot.open(Global::file_path + "chan_graph.dot", ios::out);
}

void ChanGraph::addNode(const string& _nodeName)
{
    if (nodeIndexDict.count(_nodeName))
    {
        Debug::throwError("There already has a same name node!", __FILE__, __LINE__);
    }
    else
    {
        nodeIndexDict.insert(pair<string, uint>(_nodeName, nodeIndexDict.size()));
        auto chanNode = new Chan_Node(_nodeName);
        nodes.push_back(chanNode);
    }
}

void ChanGraph::addNode(const string& _nodeName, const string& _nodeType, const string& _nodeOp, const string& _chanMode)
{
    addNode(_nodeName);
    dynamic_cast<Chan_Node*>(getNode(_nodeName))->node_type = _nodeType;
    dynamic_cast<Chan_Node*>(getNode(_nodeName))->node_op = _nodeOp;
    dynamic_cast<Chan_Node*>(getNode(_nodeName))->chan_mode = _chanMode;
}

void ChanGraph::printDotNodeLabel(std::fstream& fileName_)
{
    string label;
    for (auto& node : nodes)
    {
        string& nodeName = dynamic_cast<Chan_Node*>(node)->node_name;
        string& nodeOp = dynamic_cast<Chan_Node*>(node)->node_op;
        string& chanMode = dynamic_cast<Chan_Node*>(node)->chan_mode;
        string& nodeType = dynamic_cast<Chan_Node*>(node)->node_type;
        uint& cycle_ = dynamic_cast<Chan_Node*>(node)->cycle;
        uint& speedup_ = dynamic_cast<Chan_Node*>(node)->speedup;
        uint& size_ = dynamic_cast<Chan_Node*>(node)->size;

        label = nodeName + " [";
        if (nodeType == "ChanBase" || nodeType == "ChanDGSF")
        {
            label += "label=\"" + nodeName + "\\n" + 
                "Size:" + to_string(size_) + " "
                "Cyc:"+ to_string(cycle_) + " "
                "Speed:" + to_string(speedup_) + "\"";

            if (nodeType == "ChanDGSF")
            {
                label += ", fillcolor=\"cadetblue2\", style=filled";
            }
        }
        else if (nodeType == "Lc" || nodeType == "Mux")
        {
            label += "shape=box";
        }
        else if (nodeType == "Lse_ld" || nodeType == "Lse_st")
        {
            label += ", fillcolor=\"chartreuse2\", style=filled";
        }

        if (chanMode == "Keep_mode")
        {
            label += ", fillcolor=\"darkorange\", style=filled";
        }
        else if (nodeType == "Drain_mode")
        {
            label += ", fillcolor=\"bisque1\", style=filled";
        }

        label += "]";

        fileName_ << label << std::endl;
    }
}

void ChanGraph::plotDot()
{
    Graph::plotDot(chan_graph_dot, controlTree);
}

void ChanGraph::genChanGraphFromDfg(Dfg& dfg_)
{
    // Copy controlRegionHierarchy
    controlTree.controlRegionTable = dfg_.controlTree.controlRegionTable;
    controlTree.controlRegionIndexDict = dfg_.controlTree.controlRegionIndexDict;
    for (auto& controlRegion : controlTree.controlRegionTable)
    {
        controlRegion.nodes.clear();  // Clear dfgNode
    }
    
    // Construct chanNode
    vector<string> chanNodeName(dfg_.nodes.size());  // Keep chanNodeName to corresponding dfgNode
    string nodeName;
    string nodeType;
    string nodeOp;
    string chanMode;
    for (size_t i = 0; i < dfg_.nodes.size(); ++i)
    {
        const auto& node = dynamic_cast<Dfg_Node*>(dfg_.nodes[i]);
        nodeOp = node->node_op;

        if (node->node_type == "Normal")
        {
            nodeName = "Chan_" + node->node_name;
            nodeType = "ChanBase";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode);
        }
        else if (node->node_type == "Lc")
        {
            nodeName = "Lc_" + node->node_name;
            nodeType = "Lc";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode);
        }
        else if (node->node_type == "Mux")
        {
            nodeName = "Mux_" + node->node_name;
            nodeType = "Mux";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode);
        }
        else if (node->node_type == "MuxParitial")
        {
            nodeName = "MuxPartial_" + node->node_name;
            nodeType = "ChanPartialMux";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode);
        }
        else if (node->node_type == "Const")
        {
            // Skip this node
        }
        else
        {
            Debug::throwError("Undefined node type!", __FILE__, __LINE__);
        }

        chanNodeName[i] = nodeName;
    }

    // Add chanNode to controlTree and add connection
    for (auto& controlRegion : dfg_.controlTree.controlRegionTable)
    {
        for (auto& dfgNode : controlRegion.nodes)
        {
            string& chanName = chanNodeName[dfg_.findNodeIndex(dfgNode)->second];
            //auto& chanNodePtr = nodes[findNodeIndex(chanName)->second];
            const auto& dfgNodePtr = dynamic_cast<Dfg_Node*>(dfg_.getNode(dfgNode));
            if (dfgNodePtr->node_type != "Const")
            {
                controlTree.addNodes(controlRegion.controlRegionName, { chanName });

                for (auto& nodeName : dfgNodePtr->pre_nodes_data)
                {
                    string& s = chanNodeName[dfg_.findNodeIndex(nodeName)->second];
                    if (s != "")
                    {
                        addPreNodesData(chanName, { s });
                    }
                }
                for (auto& nodeName : dfgNodePtr->next_nodes_data)
                {
                    string& s = chanNodeName[dfg_.findNodeIndex(nodeName)->second];
                    if (s != "")
                    {
                        addNextNodesData(chanName, { chanNodeName[dfg_.findNodeIndex(nodeName)->second] });
                    }
                }
                for (auto& nodeName : dfgNodePtr->pre_nodes_active)
                {
                    string& s = chanNodeName[dfg_.findNodeIndex(nodeName)->second];
                    if (s != "")
                    {
                        addPreNodesActive(chanName, { chanNodeName[dfg_.findNodeIndex(nodeName)->second] });
                    }
                }
                for (auto& nodeName : dfgNodePtr->next_nodes_active)
                {
                    string& s = chanNodeName[dfg_.findNodeIndex(nodeName)->second];
                    if (s != "")
                    {
                        addNextNodesActive(chanName, { chanNodeName[dfg_.findNodeIndex(nodeName)->second] });
                    }
                }
                //addPreNodesData(chanName, dfgNodePtr->pre_nodes_data);
                //addNextNodesData(chanName, dfgNodePtr->next_nodes_data);
                //addPreNodesActive(chanName, dfgNodePtr->pre_nodes_active);
                //addNextNodesActive(chanName, dfgNodePtr->next_nodes_active);
            }
            else
            {
                for (auto& nextNodeData : dfgNodePtr->next_nodes_data)
                {
                    string& nextChanNodeName = chanNodeName[dfg_.findNodeIndex(nextNodeData)->second];
                    const auto& nextChanNodePtr = dynamic_cast<Chan_Node*>(getNode(nextChanNodeName));
                    nextChanNodePtr->constVal.push_back(dfgNodePtr->constVal);

                    auto iter = find(nextChanNodePtr->pre_nodes_data.begin(), nextChanNodePtr->pre_nodes_data.end(), dfgNode);
                    nextChanNodePtr->constLocation = iter - nextChanNodePtr->pre_nodes_data.begin();
                }
            }
        }
    }
    completeConnect();
    removeRedundantConnect();
}

void ChanGraph::addSpecialFuncChan()
{
    // Generate loop level hierarchy

    // Add relayNode

    // Add keepMode and drainMode

}
