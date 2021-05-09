#include "./graph.h"
#include "./Debug.h"

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
        set<string> s(node->pre_nodes_data.begin(), node->pre_nodes_data.begin());
        node->pre_nodes_data.assign(s.begin(), s.end());

        set<string> s(node->next_nodes_data.begin(), node->next_nodes_data.begin());
        node->next_nodes_data.assign(s.begin(), s.end());

        set<string> s(node->pre_nodes_active.begin(), node->pre_nodes_active.begin());
        node->pre_nodes_active.assign(s.begin(), s.end());

        set<string> s(node->next_nodes_active.begin(), node->next_nodes_active.begin());
        node->next_nodes_active.assign(s.begin(), s.end());
    }
}


//** Dfg
Dfg::Dfg()
{
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


//** ChanGraph
ChanGraph::ChanGraph()
{
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