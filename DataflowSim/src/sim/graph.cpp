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
        set<string> s1(node->pre_nodes_data.begin(), node->pre_nodes_data.begin());
        node->pre_nodes_data.assign(s1.begin(), s1.end());

        set<string> s2(node->next_nodes_data.begin(), node->next_nodes_data.begin());
        node->next_nodes_data.assign(s2.begin(), s2.end());

        set<string> s3(node->pre_nodes_active.begin(), node->pre_nodes_active.begin());
        node->pre_nodes_active.assign(s3.begin(), s3.end());

        set<string> s4(node->next_nodes_active.begin(), node->next_nodes_active.begin());
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
    for (auto& node : nodes)
    {
        if (nodeIndexDict.count(node->node_name))
        {
            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
        }
        else
        {
            nodeIndexDict.insert(pair<string, uint>(_nodeName, nodeIndexDict.size()));
            auto dfgNode = new Dfg_Node(_nodeName, _nodeOp);
            nodes.push_back(dfgNode);
        }
    }
}

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

void Dfg::plotDot(ControlTree& _controlTree)
{
    Graph::plotDot(dfg_dot, _controlTree);
}


//** ChanGraph
ChanGraph::ChanGraph()
{
    chan_graph_dot.open(Global::file_path + "chan_graph.dot", ios::out);
}

void ChanGraph::addNode(const string& _nodeName)
{
    for (auto& node : nodes)
    {
        if (nodeIndexDict.count(node->node_name))
        {
            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
        }
        else
        {
            nodeIndexDict.insert(pair<string, uint>(_nodeName, nodeIndexDict.size()));
            auto chanNode = new Chan_Node(_nodeName);
            nodes.push_back(chanNode);
        }
    }
}

void ChanGraph::printDotNodeLabel(std::fstream& fileName_)
{
    string label;
    for (auto& node : nodes)
    {
        string& nodeName = dynamic_cast<Chan_Node*>(node)->node_name;
        string& nodeOp = dynamic_cast<Chan_Node*>(node)->node_op;
        string& nodeType = dynamic_cast<Chan_Node*>(node)->node_type;
        uint& cycle_ = dynamic_cast<Chan_Node*>(node)->cycle;
        uint& speedup_ = dynamic_cast<Chan_Node*>(node)->speedup;
        uint& size_ = dynamic_cast<Chan_Node*>(node)->size;

        if (nodeType == "ChanBase" || nodeType == "ChanDGSF")
        {
            label = nodeName + " [label=\"chan_" + nodeName + "\\n" + 
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
            label += ", shape=box";
        }

        if (nodeOp == "Keep_mode")
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

void ChanGraph::plotDot(ControlTree& _controlTree)
{
    Graph::plotDot(chan_graph_dot, _controlTree);
}

