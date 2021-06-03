#include "./graph.h"
#include "./Debug.h"
#include "../sim/global.h"
#include "../../lib/metis/metis.h"

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

bool Graph::findNode(const string& _nodeName)
{
    auto iter = nodeIndexDict.find(_nodeName);
    if (iter == nodeIndexDict.end())
    {
        return false;
    }
    else
    {
        return true;
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
        //set<string> s1(node->pre_nodes_data.begin(), node->pre_nodes_data.end());
        //node->pre_nodes_data.assign(s1.begin(), s1.end());

        //set<string> s2(node->next_nodes_data.begin(), node->next_nodes_data.end());
        //node->next_nodes_data.assign(s2.begin(), s2.end());

        //set<string> s3(node->pre_nodes_active.begin(), node->pre_nodes_active.end());
        //node->pre_nodes_active.assign(s3.begin(), s3.end());

        //set<string> s4(node->next_nodes_active.begin(), node->next_nodes_active.end());
        //node->next_nodes_active.assign(s4.begin(), s4.end());

        auto s1 = Util::removeDuplicatesKeepSequence(node->pre_nodes_data);
        node->pre_nodes_data.assign(s1.begin(), s1.end());

        auto s2 = Util::removeDuplicatesKeepSequence(node->next_nodes_data);
        node->next_nodes_data.assign(s2.begin(), s2.end());

        auto s3 = Util::removeDuplicatesKeepSequence(node->pre_nodes_active);
        node->pre_nodes_active.assign(s3.begin(), s3.end());

        auto s4 = Util::removeDuplicatesKeepSequence(node->next_nodes_active);
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
        for (size_t i = 0; i < node->pre_nodes_data.size(); ++i)
        {
            fileName_ << node->pre_nodes_data[i] << "->" << node->node_name << " [label=" << i << "]" << std::endl;
        }
    }

    // Plot active connect
    for (auto& node : nodes)
    {
        for (size_t i = 0; i < node->pre_nodes_active.size(); ++i)
        {
            fileName_ << node->pre_nodes_active[i] << "->" << node->node_name << " [label=" << i << ", style = \"dashed\"]" << std::endl;
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

void Graph::addNodes2CtrlTree(const string& targetCtrlRegion, const vector<string>& nodes_)
{
    uint index = controlTree.findControlRegionIndex(targetCtrlRegion)->second;
    controlTree.controlRegionTable[index].nodes.insert(controlTree.controlRegionTable[index].nodes.end(), nodes_.begin(), nodes_.end());

    for (auto& node : nodes_)
    {
        nodes[findNodeIndex(node)->second]->controlRegionName = targetCtrlRegion;
    }
}

uint Graph::genEdgeWeight(Node* node, Node* nextNode, vector<ControlRegion>& _loopHierarchy)
{
    /*
    * If span loop ctrlRegion: a) span loop:
    *                           -> loop1_rank + loop2_rank (Inner loop prior)  [loopRankFactor]
    *                           -> loop1_size + loop2_size (Larger loop region prior)  [loopSizeFactor]
    *
    * If not span loop ctrlRegion:  a) span branch  [branchFactor]
    *                               b) between memory access  [memoryAccessFactor]
    *                               c) normal edge [baseWgt]
    */

    uint baseWgt = 200;
    uint loopRankFactor = 30;
    uint loopSizeFactor = 5;
    uint branchFactor = 20;
    uint memoryAccessFactor = 5;
    int edgeWgt = baseWgt;

    uint nodeLoopRank = 0;
    uint nextNodeLoopRank = 0;
    vector<uint> loopNodeNum;
    for (size_t i = 0; i < _loopHierarchy.size(); ++i)
    {
        if (find(_loopHierarchy[i].nodes.begin(), _loopHierarchy[i].nodes.end(), node->node_name) != _loopHierarchy[i].nodes.end())
        {
            nodeLoopRank = i;
        }
        if (find(_loopHierarchy[i].nodes.begin(), _loopHierarchy[i].nodes.end(), nextNode->node_name) != _loopHierarchy[i].nodes.end())
        {
            nextNodeLoopRank = i;
        }

        uint nodeNum = 0;
        for (auto& node : _loopHierarchy[i].nodes)
        {
            if (dynamic_cast<Chan_Node*>(getNode(node))->isPhysicalChan)
            {
                ++nodeNum;
            }
        }
        loopNodeNum.push_back(nodeNum);
    }

    if (nodeLoopRank != nextNodeLoopRank)
    {
        uint loopRank = (nodeLoopRank + nextNodeLoopRank) * loopRankFactor;
        uint loopSize = (loopNodeNum[nodeLoopRank] + loopNodeNum[nextNodeLoopRank]) * loopSizeFactor;
        edgeWgt -= (loopRank + loopSize);
    }
    else if (node->controlRegionName != nextNode->controlRegionName)
    {
        edgeWgt = baseWgt - branchFactor;
    }

    if (/*dynamic_cast<Chan_Node*>(node)->node_op == "Load" ||*/
        dynamic_cast<Chan_Node*>(node)->node_op == "Store" ||
        dynamic_cast<Chan_Node*>(nextNode)->node_op == "Load" /*||
        dynamic_cast<Chan_Node*>(nextNode)->node_op == "Store"*/)
    {
        edgeWgt -= memoryAccessFactor;
    }

    return std::max(edgeWgt, 1);  // Ensure edgeWgt is positive
}

auto Graph::genAdjacentMatrix()->vector<vector<uint>>
{
    vector<ControlRegion> loopHierarchy_ = dynamic_cast<ChanGraph*>(this)->genLoopHierarchy(controlTree);

    // Generate adjacent matrix
    uint dim = nodes.size();
    vector<vector<uint>> matrix(dim, vector<uint>(dim, 0));
    for (uint i = 0; i < dim; ++i)
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            vector<string> adjNodes;
            adjNodes.insert(adjNodes.end(), nodes[i]->pre_nodes_data.begin(), nodes[i]->pre_nodes_data.end());
            adjNodes.insert(adjNodes.end(), nodes[i]->pre_nodes_active.begin(), nodes[i]->pre_nodes_active.end());
            adjNodes.insert(adjNodes.end(), nodes[i]->next_nodes_data.begin(), nodes[i]->next_nodes_data.end());
            adjNodes.insert(adjNodes.end(), nodes[i]->next_nodes_active.begin(), nodes[i]->next_nodes_active.end());
            for (auto& adjNode : adjNodes)
            {
                uint j = findNodeIndex(adjNode)->second;
                //if (nodes[i]->controlRegionName == nodes[j]->controlRegionName)
                //{
                //    matrix[i][j] = _edgeWeightWithinCtrlRegion;  // Weight of edges within a same ctrlRegion
                //}
                //else
                //{
                //    matrix[i][j] = 1;  // Weight of edges inter two different ctrlRegion
                //}
                matrix[i][j] = genEdgeWeight(nodes[i], nodes[j], loopHierarchy_);
            }
        }
    }

    return matrix;
}

auto Graph::csrFormat(uint _edgeWeightWithinCtrlRegion)->tuple<vector<int64_t>, vector<int64_t>, vector<int64_t>>
{
    auto matrix = genAdjacentMatrix();

    // Generate CSR format by adjacent matrix
    vector<int64_t> indPtr(0);
    vector<int64_t> ind(0);
    vector<int64_t> val(0);
    uint dim = nodes.size();
    for (int i = 0; i < dim; i++)
    {
        indPtr.push_back(ind.size());
        for (uint j = 0; j < dim; ++j)
        {
            if (matrix[i][j] > 0)
            {
                ind.push_back(j);
                val.push_back(matrix[i][j]);
            }
        }
    }
    indPtr.push_back(ind.size());

    return make_tuple(indPtr, ind, val);
}

void Graph::subgraphPartition(uint _subgraphNum, uint _edgeWeightWithinCtrlRegion)
{
    uint divideNum = _subgraphNum;
    if (_subgraphNum > 1)
    {
        auto dfgCsr = csrFormat(_edgeWeightWithinCtrlRegion);
        vector<idx_t> cluster;
        vector<idx_t> xadj = std::get<0>(dfgCsr);
        vector<idx_t> adjncy = std::get<1>(dfgCsr);
        vector<idx_t> adjwgt = std::get<2>(dfgCsr);

        idx_t nVertices = xadj.size() - 1;  // Vertex number
        idx_t nEdges = adjncy.size() / 2;  // Edge number
        idx_t nWeights = 1;
        idx_t nParts = static_cast<idx_t>(divideNum);  // Subgraph number
        idx_t objval;
        std::vector<idx_t> part(nVertices, 0);
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_CONTIG] = 1;

        int ufactor = 4096;
        bool validPartition = 1;
        while (1)  // Ensure each subgraph at least has one node
        {
            validPartition = 1;
            options[METIS_OPTION_UFACTOR] = ufactor;  // Don't care of task balancing
            // TODO: select the best graph split function!
            if (divideNum > 8)
            {
                int ret = METIS_PartGraphKway(&nVertices, &nWeights, xadj.data(), adjncy.data(), NULL, NULL, adjwgt.data(), &nParts, NULL, NULL, options, &objval, part.data());
            }
            else
            {
                int ret = METIS_PartGraphRecursive(&nVertices, &nWeights, xadj.data(), adjncy.data(), NULL, NULL, adjwgt.data(), &nParts, NULL, NULL, options, &objval, part.data());
            }

            for (size_t i = 0; i < divideNum; ++i)
            {
                if (find(part.begin(), part.end(), i) == part.end())
                {
                    validPartition = 0;
                    ufactor >>= 1;
                    break;
                }
            }

            if (validPartition)
            {
                break;
            }
        }

        //int ret = METIS_PartGraphKway(&nVertices, &nWeights, xadj.data(), adjncy.data(), NULL, NULL, adjwgt.data(), &nParts, NULL, NULL, options, &objval, part.data());

        for (size_t i = 0; i < part.size(); ++i)
        {
            nodes[i]->subgraphId = part[i];
        }
    }
    else
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i]->subgraphId = 0;
        }
    }


    sortSubgraphId(nodes, divideNum);

    //// Print each subgraph
    //for (size_t i = 0; i < devideNum; ++i)
    //{
    //    std::cout << "SubgraphId: " << i << std::endl;
    //    for (auto& node : nodes)
    //    {
    //        if (node->subgraphId == i)
    //        {
    //            std::cout << "\t" << node->node_name << std::endl;
    //        }
    //    }
    //}

    uint adjacentEdgeNum = 0;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        uint partId = nodes[i]->subgraphId;
        vector<string> preNodes;
        preNodes.insert(preNodes.end(), nodes[i]->pre_nodes_data.begin(), nodes[i]->pre_nodes_data.end());
        preNodes.insert(preNodes.end(), nodes[i]->pre_nodes_active.begin(), nodes[i]->pre_nodes_active.end());
        for (auto& preNode : preNodes)
        {
            if (getNode(preNode)->subgraphId != partId)
            {
                adjacentEdgeNum++;
            }
        }
    }

    uint actualNodeNum = 0;
    uint memNormAccessNum = 0;
    for (auto& node : nodes)
    {
        auto nodePtr = dynamic_cast<Chan_Node*>(node);
        if (nodePtr->isPhysicalChan && nodePtr->node_op != "Nop")
        {
            ++actualNodeNum;
        }

        if (nodePtr->node_op == "Load" || nodePtr->node_op == "Store")
        {
            ++memNormAccessNum;
        }
    }

    uint arraySize = 64;
    uint maxCoalesceRate = BANK_BLOCK_SIZE / DATA_PRECISION;
    // TODO: Figure out actual coalesceRate of each subgraph
    uint coalesceRate = std::min(arraySize / (actualNodeNum / divideNum), maxCoalesceRate);
    float memAccessInterNum = static_cast<float>(adjacentEdgeNum) / static_cast<float>(coalesceRate);
    float memAccessNormalNum = static_cast<float>(memNormAccessNum) / static_cast<float>(coalesceRate);
    float memAccessTotalNum = memAccessInterNum + memAccessNormalNum;

    std::cout << std::endl;
    std::cout << ">>> Part_num: " << _subgraphNum << std::endl;
    std::cout << "totalActualNodeNum: " << actualNodeNum << std::endl;
    std::cout << "normalMemAccessNum: " << memNormAccessNum << std::endl;
    std::cout << "adjEdgeNum: " << adjacentEdgeNum << std::endl;
    std::cout << std::endl;
    std::cout << "coalesce_rate: " << coalesceRate << std::endl;
    std::cout << "mem_access_num_inter: " << memAccessInterNum << std::endl;
    std::cout << "mem_access_num_normal: " << memAccessNormalNum << std::endl;
    std::cout << "total_mem_access_num: " << memAccessTotalNum << std::endl;
    std::cout << std::endl;
}

void Graph::sortSubgraphId(deque<Node*>& nodes, uint subgraphNum)
{
    //auto simNodes = bfsTraverseNode();
    vector<vector<Node*>> subgraphNodes(subgraphNum);
    vector<uint> subgraphQueue;

    //if (!simNodes.empty())
    //{
    //    subgraphIdVal.insert(make_pair(getNode(simNodes.front())->subgraphId, 0));
    //}

    //// Generate a rough sort according to BFS
    //for (auto& node : simNodes)
    //{
    //    const auto& nodePtr = getNode(node);
    //    subgraphNodes[nodePtr->subgraphId].push_back(nodePtr);

    //    if (find(subgraphQueue.begin(), subgraphQueue.end(), nodePtr->subgraphId) == subgraphQueue.end())
    //    {
    //        subgraphQueue.push_back(nodePtr->subgraphId);
    //    }
    //}

    for (auto& node : nodes)
    {
        subgraphNodes[node->subgraphId].push_back(node);

        if (find(subgraphQueue.begin(), subgraphQueue.end(), node->subgraphId) == subgraphQueue.end())
        {
            subgraphQueue.push_back(node->subgraphId);
        }
    }

    // Sort
    for (auto iter = subgraphQueue.begin(); iter != subgraphQueue.end(); ++iter)
    {
        for (auto& node : subgraphNodes[*iter])
        {
            vector<string> nextNodes;
            nextNodes.insert(nextNodes.end(), node->next_nodes_data.begin(), node->next_nodes_data.end());
            nextNodes.insert(nextNodes.end(), node->next_nodes_active.begin(), node->next_nodes_active.end());
            for (auto& nextNode : nextNodes)
            {
                auto iter_ = find(subgraphQueue.begin(), subgraphQueue.end(), getNode(nextNode)->subgraphId);
                if (iter_ < iter)
                {
                    swap(*iter, *iter_);
                }
            }
        }
    }

    for (size_t i = 0; i < subgraphQueue.size(); ++i)
    {
        for (auto* node : subgraphNodes[subgraphQueue[i]])
        {
            node->subgraphId = i;  // Update subgraphId
        }
    }
}

void Graph::printSubgraphPartition(const uint& divideNum, Debug* debug)
{
    // Print each subgraph
    debug->getFile() << "********** Subgraph partitiion scheme ************ " << std::endl;
    for (size_t i = 0; i < divideNum; ++i)
    {
        debug->getFile() << "SubgraphId: " << i << std::endl;
        for (auto& node : nodes)
        {
            if (node->subgraphId == i)
            {
                debug->getFile() << "\t" << node->node_name << std::endl;
            }
        }
    }

    uint adjacentEdgeNum = 0;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        uint partId = nodes[i]->subgraphId;
        vector<string> preNodes;
        preNodes.insert(preNodes.end(), nodes[i]->pre_nodes_data.begin(), nodes[i]->pre_nodes_data.end());
        preNodes.insert(preNodes.end(), nodes[i]->pre_nodes_active.begin(), nodes[i]->pre_nodes_active.end());
        for (auto& preNode : preNodes)
        {
            if (getNode(preNode)->subgraphId != partId)
            {
                adjacentEdgeNum++;
            }
        }
    }

    uint actualNodeNum = 0;
    uint memNormAccessNum = 0;
    for (auto& node : nodes)
    {
        auto nodePtr = dynamic_cast<Chan_Node*>(node);
        if (nodePtr->isPhysicalChan && nodePtr->node_op != "Nop")
        {
            ++actualNodeNum;
        }

        if (nodePtr->node_op == "Load" || nodePtr->node_op == "Store")
        {
            ++memNormAccessNum;
        }
    }

    uint arraySize = 64;
    uint maxCoalesceRate = BANK_BLOCK_SIZE / DATA_PRECISION;
    // TODO: Figure out actual coalesceRate of each subgraph
    uint coalesceRate = std::min(arraySize / (actualNodeNum / divideNum), maxCoalesceRate);
    float memAccessInterNum = static_cast<float>(adjacentEdgeNum) / static_cast<float>(coalesceRate);
    float memAccessNormalNum = static_cast<float>(memNormAccessNum) / static_cast<float>(coalesceRate);
    float memAccessTotalNum = memAccessInterNum + memAccessNormalNum;

    debug->getFile() << std::endl;
    debug->getFile() << "********** Partition statistic ************* " << std::endl;
    debug->getFile() << ">>> Part_num: " << divideNum << std::endl;
    debug->getFile() << "totalActualNodeNum: " << actualNodeNum << std::endl;
    debug->getFile() << "normalMemAccessNum: " << memNormAccessNum << std::endl;
    debug->getFile() << "adjEdgeNum: " << adjacentEdgeNum << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "coalesce_rate: " << coalesceRate << std::endl;
    debug->getFile() << "mem_access_num_inter: " << memAccessInterNum << std::endl;
    debug->getFile() << "mem_access_num_normal: " << memAccessNormalNum << std::endl;
    debug->getFile() << "total_mem_access_num: " << memAccessTotalNum << std::endl;
    debug->getFile() << std::endl;
}

vector<string> Graph::bfsTraverseNode()
{
    vector<string> bfsNodeTree;
    deque<string> queue;
    for (auto& node : nodes)
    {
        if (node->pre_nodes_data.empty() && node->pre_nodes_active.empty())
        {
            queue.push_back(node->node_name);
        }
    }

    while (!queue.empty())
    {
        vector<string> nextNodes;
        nextNodes.insert(nextNodes.end(), getNode(queue.front())->next_nodes_data.begin(), getNode(queue.front())->next_nodes_data.end());
        nextNodes.insert(nextNodes.end(), getNode(queue.front())->next_nodes_active.begin(), getNode(queue.front())->next_nodes_active.end());
        for (auto& nextNode : nextNodes)
        {
            queue.push_back(nextNode);
        }

        bfsNodeTree.push_back(queue.front());
        queue.pop_front();
    }

    // Remove redundant node
    reverse(bfsNodeTree.begin(), bfsNodeTree.end());
    vector<string> hasFound;
    for (auto iter = bfsNodeTree.begin(); iter != bfsNodeTree.end(); )
    {
        if (find(hasFound.begin(), hasFound.end(), *iter) != hasFound.end())
        {
            iter = bfsNodeTree.erase(iter);
        }
        else
        {
            hasFound.push_back(*iter);
            ++iter;
        }
    }
    reverse(bfsNodeTree.begin(), bfsNodeTree.end());

    return bfsNodeTree;
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
    dfg_dot.open(Global::file_path + App_name_convert::toString(Global::app_name) + "_dfg.dot", ios::out);
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
        else if (_nodeOp == "Sel")
        {
            dfgNode->node_type = "Mux";
        }
        else if (_nodeOp == "SelPartial")
        {
            dfgNode->node_type = "MuxPartial";
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

void Dfg::setTheTailNode(const string& targetCtrlRegion, string nodeName)
{
    auto& nodeType_ = dynamic_cast<Dfg_Node*>(getNode(nodeName))->node_type;
    if (nodeType_ == "Normal")
    {
        auto& nodeOp_ = dynamic_cast<Dfg_Node*>(getNode(nodeName))->node_op;
        if (nodeOp_ == "Load" || nodeOp_ == "Store")
        {
            nodeName = "Lse_" + nodeName;
        }
        else
        {
            nodeName = "Chan_" + nodeName;
        }
    }
    else if (nodeType_ == "Lc")
    {
        nodeName = "Lc_" + nodeName;
    }
    else if (nodeType_ == "Mux")
    {
        nodeName = "Mux_" + nodeName;
    }

    controlTree.getCtrlRegion(targetCtrlRegion).theTailNode = nodeName;
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
    chan_graph_dot.open(Global::file_path + App_name_convert::toString(Global::app_name) + "_chan_graph.dot", ios::out);
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

void ChanGraph::addNode(const string& _nodeName, const string& _nodeType, const string& _nodeOp, const string& _chanMode, const string& _ctrlRegion)
{
    addNode(_nodeName, _nodeType, _nodeOp, _chanMode);
    dynamic_cast<Chan_Node*>(getNode(_nodeName))->controlRegionName = _ctrlRegion;
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
        if (nodeType == "ChanBase" || nodeType == "ChanDGSF" || nodeType == "Lse_ld" || nodeType == "Lse_st")
        {
            label += "label=\"" + nodeName + "\\n" +
                "Size:" + to_string(size_) + " "
                "Cyc:" + to_string(cycle_) + " "
                "Speed:" + to_string(speedup_) + "\\n" +
                nodeType + " "
                "Graph: " + to_string(node->subgraphId) + 
                "\"";

            if (nodeType == "ChanDGSF")
            {
                /*label += ", fillcolor=\"cadetblue2\", style=filled";*/
                label += ", shape=octagon";
            }
            else if (nodeType == "Lse_ld" || nodeType == "Lse_st")
            {
                label += ", fillcolor=\"chartreuse2\", style=filled";
            }
        }
        else if (nodeType == "Lc" || nodeType == "Mux")
        {
            label += "shape=box";
        }

        if (chanMode == "Keep_mode")
        {
            label += ", fillcolor=\"darkorange\", style=filled";
        }
        else if (chanMode == "Drain_mode")
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
    string controlRegionName;
    for (size_t i = 0; i < dfg_.nodes.size(); ++i)
    {
        const auto& node = dynamic_cast<Dfg_Node*>(dfg_.nodes[i]);
        nodeOp = node->node_op;
        controlRegionName = node->controlRegionName;

        if (node->node_type == "Normal")
        {
            if (node->node_op == "Load")
            {
                nodeType = "Lse_ld";
                nodeName = "Lse_" + node->node_name;
            }
            else if (node->node_op == "Store")
            {
                nodeType = "Lse_st";
                nodeName = "Lse_" + node->node_name;
            }
            else
            {
                nodeType = "ChanBase";
                nodeName = "Chan_" + node->node_name;
            }
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode, controlRegionName);
            dynamic_cast<Chan_Node*>(getNode(nodeName))->baseAddr = node->baseAddr;  // Only for load/store chanNode
        }
        else if (node->node_type == "Lc")
        {
            nodeName = "Lc_" + node->node_name;
            nodeType = "Lc";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode, controlRegionName);
        }
        else if (node->node_type == "Mux")
        {
            nodeName = "Mux_" + node->node_name;
            nodeType = "Mux";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode, controlRegionName);
        }
        else if (node->node_type == "MuxPartial")
        {
            nodeName = "MuxPartial_" + node->node_name;
            nodeType = "ChanPartialMux";
            chanMode = "Normal";
            addNode(nodeName, nodeType, nodeOp, chanMode, controlRegionName);
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
                addNodes2CtrlTree(controlRegion.controlRegionName, { chanName });

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

void ChanGraph::addSpecialModeChan()
{
    // Generate loop level hierarchy
    vector<ControlRegion> loopHierarchy = genLoopHierarchy(controlTree);

    //** Add relayNode
    // Find the shortest path between two nodes in two different controlRegions
    for (auto& node : nodes)
    {
        //std::cout << node->node_name << std::endl;
        //if (node->node_name == "Chan_i_row")
        //{
        //    std::cout << node->node_name << std::endl;
        //}
        vector<string> nextNodes;
        nextNodes.insert(nextNodes.end(), node->next_nodes_data.begin(), node->next_nodes_data.end());
        nextNodes.insert(nextNodes.end(), node->next_nodes_active.begin(), node->next_nodes_active.end());

        for (auto& nextNodeName : nextNodes)
        {
            auto& nextNode = nodes[findNodeIndex(nextNodeName)->second];
            if (node->controlRegionName != nextNode->controlRegionName)
            {
                // Construct ctrlRegionPath
                //vector<string> ctrlRegionPath;
                vector<string> s = backTrackPath(node->node_name, loopHierarchy);
                vector<string> s_next = backTrackPath(nextNode->node_name, loopHierarchy);
                vector<string> ctrlRegionPath = findShortestCtrlRegionPath(s, s_next);

                // Insert relay nodes according to ctrlRegionPath
                if (ctrlRegionPath.size() > 2)
                {
                    auto it = ctrlRegionPath.begin();
                    ctrlRegionPath.erase(it);  // Remove node's ctrlRegion
                    ctrlRegionPath.pop_back();  // Remove nextNode's ctrlRegion

                    vector<string> nodeVec;
                    // Create relay node
                    for (auto& ctrlRegion : ctrlRegionPath)
                    {
                        string nodeName = node->node_name + "_relay_" + ctrlRegion;
                        if (!findNode(nodeName))
                        {
                            addNode(nodeName, "ChanBase", "Nop", "Normal", ctrlRegion);  // Add relayMode node
                            dynamic_cast<Chan_Node*>(getNode(nodeName))->isPhysicalChan = false;
                            addNodes2CtrlTree(ctrlRegion, { nodeName });

                            // Add active connect between relay node and Lc
                            for (auto& node_ : controlTree.controlRegionTable[controlTree.findControlRegionIndex(ctrlRegion)->second].nodes)
                            {
                                auto nodePtr = dynamic_cast<Chan_Node*>(nodes[findNodeIndex(node_)->second]);
                                if (nodePtr->node_type == "Lc")
                                {
                                    nodes[findNodeIndex(nodeName)->second]->pre_nodes_active.push_back(nodePtr->node_name);
                                    break;
                                }
                            }
                        }

                        nodeVec.push_back(nodeName);
                    }

                    // Connect each level relay nodes
                    for (auto iter = nodeVec.begin(); iter != nodeVec.end(); ++iter)
                    {
                        if (iter == nodeVec.begin())
                        {
                            nodes[findNodeIndex(*iter)->second]->pre_nodes_data.push_back(node->node_name);
                        }
                        if (iter == nodeVec.end() - 1)
                        {
                            auto iter_data = find(nextNode->pre_nodes_data.begin(), nextNode->pre_nodes_data.end(), node->node_name);
                            auto iter_active = find(nextNode->pre_nodes_active.begin(), nextNode->pre_nodes_active.end(), node->node_name);
                            if (iter_data != nextNode->pre_nodes_data.end())
                            {
                                nodes[findNodeIndex(*iter)->second]->next_nodes_data.push_back(nextNode->node_name);
                            }
                            else if (iter_active != nextNode->pre_nodes_active.end())
                            {
                                nodes[findNodeIndex(*iter)->second]->next_nodes_active.push_back(nextNode->node_name);
                            }
                        }
                        if (iter != nodeVec.begin() && iter != nodeVec.end() - 1)
                        {
                            nodes[findNodeIndex(*iter)->second]->pre_nodes_data.push_back(*(iter - 1));
                            nodes[findNodeIndex(*iter)->second]->next_nodes_data.push_back(*(iter + 1));
                        }
                    }

                    // Delete original node -> nextNode connection
                    // Must ensure there has inserted a relay node
                    auto iter_data = find(node->next_nodes_data.begin(), node->next_nodes_data.end(), nextNode->node_name);
                    auto iter_active = find(node->next_nodes_active.begin(), node->next_nodes_active.end(), nextNode->node_name);
                    auto iter_next_data = find(nextNode->pre_nodes_data.begin(), nextNode->pre_nodes_data.end(), node->node_name);
                    auto iter_next_active = find(nextNode->pre_nodes_active.begin(), nextNode->pre_nodes_active.end(), node->node_name);

                    if (iter_data != node->next_nodes_data.end())
                    {
                        node->next_nodes_data.erase(iter_data);
                    }

                    if (iter_active != node->next_nodes_active.end())
                    {
                        node->next_nodes_active.erase(iter_active);
                    }

                    if (iter_next_data != nextNode->pre_nodes_data.end())
                    {
                        nextNode->pre_nodes_data.erase(iter_next_data);
                    }

                    if (iter_next_active != nextNode->pre_nodes_active.end())
                    {
                        nextNode->pre_nodes_active.erase(iter_next_active);
                    }
                }

                //// Delete original node -> nextNode connection
                //// Only when ctrlRegionPath not empty, signify there has inserted a relay node
                //if (/*!ctrlRegionPath.empty()*/ctrlRegionPath.size() >= 2)
                //{
                //    auto iter_data = find(node->next_nodes_data.begin(), node->next_nodes_data.end(), nextNode->node_name);
                //    auto iter_active = find(node->next_nodes_active.begin(), node->next_nodes_active.end(), nextNode->node_name);
                //    auto iter_next_data = find(nextNode->pre_nodes_data.begin(), nextNode->pre_nodes_data.end(), node->node_name);
                //    auto iter_next_active = find(nextNode->pre_nodes_active.begin(), nextNode->pre_nodes_active.end(), node->node_name);

                //    if (iter_data != node->next_nodes_data.end())
                //    {
                //        node->next_nodes_data.erase(iter_data);
                //    }

                //    if (iter_active != node->next_nodes_active.end())
                //    {
                //        node->next_nodes_active.erase(iter_active);
                //    }

                //    if (iter_next_data != nextNode->pre_nodes_data.end())
                //    {
                //        nextNode->pre_nodes_data.erase(iter_next_data);
                //    }

                //    if (iter_next_active != nextNode->pre_nodes_active.end())
                //    {
                //        nextNode->pre_nodes_active.erase(iter_next_active);
                //    }
                //}
            }
        }
    }
    // Delete below in the final version
    completeConnect();
    removeRedundantConnect();

    // TODO: Strict rule: The controlRegion of all the nextNodes of a chanNode in keepMode or drainMode must be the same. Remove this restriction in the future 
    // Set keepMode and drainMode
    vector<ControlRegion> loopHierarchy_ = genLoopHierarchy(controlTree);
    for (auto& ctrlRegion : loopHierarchy_)
    {
        for (auto& node : ctrlRegion.nodes)
        {
            // KeepMode
            auto nodePtr = nodes[findNodeIndex(node)->second];
            vector<string> next_nodes = nodePtr->next_nodes_data;
            next_nodes.insert(next_nodes.end(), nodePtr->next_nodes_active.begin(), nodePtr->next_nodes_active.end());

            string lastLowerCtrlRegion;
            vector<string> nextNodesInLowerCtrlRegion;
            for (auto& nextNode : next_nodes)
            {
                auto iter = find(ctrlRegion.lowerControlRegion.begin(), ctrlRegion.lowerControlRegion.end(), findNodeCtrlRegionInLoopHierarchy(nextNode, loopHierarchy_));
                if (iter != ctrlRegion.lowerControlRegion.end())
                {
                    if (lastLowerCtrlRegion == "")
                    {
                        lastLowerCtrlRegion = *iter;
                    }
                    else
                    {
                        if (lastLowerCtrlRegion != *iter)
                        {
                            Debug::throwError("The lowerCtrlRegions are not same!", __FILE__, __LINE__);
                        }
                    }

                    if (dynamic_cast<Chan_Node*>(getNode(nextNode))->node_type != "Lc")
                    {
                        nextNodesInLowerCtrlRegion.push_back(nextNode);
                    }
                }
            }
            if (lastLowerCtrlRegion != "")
            {
                dynamic_cast<Chan_Node*>(nodePtr)->chan_mode = "Keep_mode";
                //for (auto& nextNodeData : nodePtr->next_nodes_data)
                //{
                //    const auto& nextNodeDataPtr = getNode(nextNodeData);
                //    for (auto iter = nextNodeDataPtr->pre_nodes_data.begin(); iter != nextNodeDataPtr->pre_nodes_data.end();)
                //    {
                //        if (*iter == nodePtr->node_name)
                //        {
                //            iter = nextNodeDataPtr->pre_nodes_data.erase(iter);
                //        }
                //        else
                //        {
                //            ++iter;
                //        }
                //    }
                //    getNode(nextNodeData)->pre_nodes_active.push_back(nodePtr->node_name);
                //}

                //for (auto& nextNodeActive : nodePtr->next_nodes_active)
                //{
                //    const auto& nextNodeActivePtr = getNode(nextNodeActive);
                //    for (auto iter = nextNodeActivePtr->pre_nodes_active.begin(); iter != nextNodeActivePtr->pre_nodes_active.end();)
                //    {
                //        if (*iter == nodePtr->node_name)
                //        {
                //            iter = nextNodeActivePtr->pre_nodes_active.erase(iter);
                //        }
                //        else
                //        {
                //            ++iter;
                //        }
                //    }
                //}


                // If the next node number in the lower loop region is more than one, create a fake chan in the lower loop as a shadow channel
                // TODO: If it exist a relayMode channel in the nextNodes, it can set the relayMode channel in Scatter mode rather than create a new one
                // Note: Compulsorily add a scatterMode channel for each keepMode channel to ensure correctness. Remove this rule in the future
                if (nodePtr->next_nodes_active.empty() || 
                    (!nodePtr->next_nodes_active.empty() && dynamic_cast<Chan_Node*>(getNode(nodePtr->next_nodes_active.front()))->node_type != "Lc")/*nextNodesInLowerCtrlRegion.size() > 1*/)
                {
                    string nodeName = nodePtr->node_name + "_scatter_" + lastLowerCtrlRegion;
                    addNode(nodeName, "ChanBase", "Nop", "Scatter_mode", lastLowerCtrlRegion);  // Add scatter mode
                    dynamic_cast<Chan_Node*>(getNode(nodeName))->isPhysicalChan = false;
                    addNodes2CtrlTree(lastLowerCtrlRegion, { nodeName });

                    // Add active connect between Scatter_mode node and Lc
                    for (auto& node_ : controlTree.getCtrlRegion(lastLowerCtrlRegion).nodes)
                    {
                        auto nodePtr = dynamic_cast<Chan_Node*>(nodes[findNodeIndex(node_)->second]);
                        if (nodePtr->node_type == "Lc")
                        {
                            nodes[findNodeIndex(nodeName)->second]->pre_nodes_active.push_back(nodePtr->node_name);
                            break;
                        }
                    }

                    // Gen connect
                    for (auto& nextNodeData : nodePtr->next_nodes_data)
                    {
                        auto iter = find(getNode(nextNodeData)->pre_nodes_data.begin(), getNode(nextNodeData)->pre_nodes_data.end(), nodePtr->node_name);
                        if (iter != getNode(nextNodeData)->pre_nodes_data.end())
                        {
                            *iter = nodeName;
                            getNode(nodeName)->next_nodes_data.push_back(nextNodeData);
                        }
                    }
                    for (auto& nextNodeActive : nodePtr->next_nodes_active)
                    {
                        auto iter = find(getNode(nextNodeActive)->pre_nodes_active.begin(), getNode(nextNodeActive)->pre_nodes_active.end(), nodePtr->node_name);
                        if (iter != getNode(nextNodeActive)->pre_nodes_active.end())
                        {
                            *iter = nodeName;
                            getNode(nodeName)->next_nodes_active.push_back(nextNodeActive);
                        }
                    }

                    nodePtr->next_nodes_data.clear();
                    nodePtr->next_nodes_active.clear();
                    nodePtr->next_nodes_data.push_back(nodeName);

                    getNode(nodeName)->pre_nodes_data.push_back(nodePtr->node_name);
                }
            }

            // DrainMode
            vector<string> pre_nodes = nodePtr->pre_nodes_data;
            pre_nodes.insert(pre_nodes.end(), nodePtr->pre_nodes_active.begin(), nodePtr->pre_nodes_active.end());
            string lastUpperCtrlRegion;

            for (auto& preNode : pre_nodes)
            {
                auto iter_ = find(ctrlRegion.lowerControlRegion.begin(), ctrlRegion.lowerControlRegion.end(), findNodeCtrlRegionInLoopHierarchy(preNode, loopHierarchy_));
                if (iter_ != ctrlRegion.lowerControlRegion.end())
                {
                    if (lastUpperCtrlRegion == "")
                    {
                        lastUpperCtrlRegion = *iter_;
                    }
                    else
                    {
                        if (lastUpperCtrlRegion != *iter_)
                        {
                            Debug::throwError("The lowerCtrlRegions are not same!", __FILE__, __LINE__);
                        }
                    }
                }
            }
            if (lastUpperCtrlRegion != "")
            {
                dynamic_cast<Chan_Node*>(nodePtr)->chan_mode = "Drain_mode";
            }
        }
    }

    completeConnect();
    removeRedundantConnect();
}

void ChanGraph::insertChanNode(Chan_Node& chanNode, vector<string> preNodes, vector<string> nextNodes)
{
    //if (preNodes.empty() || nextNodes.empty())
    //{
    //    Debug::throwError("PreNodes or nextNods is empty!", __FILE__, __LINE__);
    //}
    if (preNodes.size() > 1 && nextNodes.size() > 1)
    {
        Debug::throwError("At least one of them's size must equal to one!", __FILE__, __LINE__);
    }
    else if (chanNode.controlRegionName == "")
    {
        Debug::throwError("The controlRegionName of this chanNode did not declare!", __FILE__, __LINE__);
    }
    else
    {
        if (nextNodes.size() == 1)
        {
            auto& nextNodePtr = nodes[findNodeIndex(nextNodes.front())->second];
            for (auto& preNode : preNodes)
            {
                auto& preNodePtr = nodes[findNodeIndex(preNode)->second];
                auto iter_data = find(preNodePtr->next_nodes_data.begin(), preNodePtr->next_nodes_data.end(), nextNodes.front());
                auto iter_active = find(preNodePtr->next_nodes_active.begin(), preNodePtr->next_nodes_active.end(), nextNodes.front());

                if (iter_data != preNodePtr->next_nodes_data.end())
                {
                    *iter_data = chanNode.node_name;
                }
                if (iter_active != preNodePtr->next_nodes_active.end())
                {
                    *iter_active = chanNode.node_name;
                }

                chanNode.pre_nodes_data = nextNodePtr->pre_nodes_data;
                chanNode.pre_nodes_active = nextNodePtr->pre_nodes_active;

                chanNode.next_nodes_data.push_back(nextNodePtr->node_name);

                nextNodePtr->pre_nodes_active.clear();
                nextNodePtr->pre_nodes_data.clear();
                nextNodePtr->pre_nodes_data.push_back(chanNode.node_name);
            }
        }

        if (preNodes.size() == 1)
        {
            auto& preNodePtr = nodes[findNodeIndex(preNodes.front())->second];
            for (auto& nextNode : nextNodes)
            {
                auto& nextNodePtr = nodes[findNodeIndex(nextNode)->second];
                auto iter_data = find(nextNodePtr->pre_nodes_data.begin(), nextNodePtr->pre_nodes_data.end(), preNodes.front());
                auto iter_active = find(nextNodePtr->pre_nodes_active.begin(), nextNodePtr->pre_nodes_active.end(), preNodes.front());

                if (iter_data != nextNodePtr->pre_nodes_data.end())
                {
                    *iter_data = chanNode.node_name;
                }
                if (iter_active != nextNodePtr->pre_nodes_active.end())
                {
                    *iter_active = chanNode.node_name;
                }

                chanNode.next_nodes_data = nextNodePtr->next_nodes_data;
                chanNode.next_nodes_active = nextNodePtr->next_nodes_active;

                chanNode.pre_nodes_data.push_back(preNodePtr->node_name);

                preNodePtr->next_nodes_active.clear();
                preNodePtr->next_nodes_data.clear();
                preNodePtr->next_nodes_data.push_back(chanNode.node_name);
            }
        }

        // Insert chanNode to nodes and nodeIndexDict
        nodeIndexDict.insert(pair<string, uint>(chanNode.node_name, nodeIndexDict.size()));
        Chan_Node* chanNodePtr = new Chan_Node(chanNode.node_name);
        *chanNodePtr = chanNode;
        nodes.push_back(chanNodePtr);
    }
}

string ChanGraph::findNodeCtrlRegionInLoopHierarchy(string _nodeName, vector<ControlRegion> _loopHierarchy)
{
    for (auto& ctrlRegion : _loopHierarchy)
    {
        for (auto& node : ctrlRegion.nodes)
        {
            if (_nodeName == node)
            {
                return ctrlRegion.controlRegionName;
            }
        }
    }

    string nullString;
    return nullString;
}

//string ChanGraph::findCtrlRegion(string& ctrlRegionName, vector<ControlRegion>& loopHierarchy)
//{
//    for (auto& ctrlRegion : loopHierarchy)
//    {
//        if (ctrlRegionName == ctrlRegion.controlRegionName)
//        {
//            return 
//        }
//    }
//}

vector<ControlRegion> ChanGraph::genLoopHierarchy(ControlTree& _controlTree)
{
    vector<ControlRegion> loopHierarchy = _controlTree.controlRegionTable;
    auto bfsCtrlRegion = bfsTraverseControlTree(_controlTree);
    reverse(bfsCtrlRegion.begin(), bfsCtrlRegion.end());

    for (auto& ctrlRegionName : bfsCtrlRegion)
    {
        //auto ctrlRegionName = bfsCtrlRegion[i];
        auto& ctrlRegion = loopHierarchy[_controlTree.findControlRegionIndex(ctrlRegionName)->second];
        if (ctrlRegion.controlType != "Loop")
        {
            auto& upperCtrlRegion = loopHierarchy[_controlTree.findControlRegionIndex(ctrlRegion.upperControlRegion)->second];
            upperCtrlRegion.nodes.insert(upperCtrlRegion.nodes.end(), ctrlRegion.nodes.begin(), ctrlRegion.nodes.end());
            upperCtrlRegion.lowerControlRegion.insert(upperCtrlRegion.lowerControlRegion.end(), ctrlRegion.lowerControlRegion.begin(), ctrlRegion.lowerControlRegion.end());

            //for (auto& lowerCtrlRegionName : ctrlRegion.lowerControlRegion)
            //{
            //    auto& lowerCtrlRegion = loopHierarchy[_controlTree.findControlRegionIndex(lowerCtrlRegionName)->second];
            //    lowerCtrlRegion.upperControlRegion = ctrlRegion.upperControlRegion;
            //}
        }
    }

    for (auto& ctrlRegion : loopHierarchy)
    {
        if (ctrlRegion.controlType == "Loop")
        {
            for (auto& lowerCtrlRegion : ctrlRegion.lowerControlRegion)
            {
                loopHierarchy[_controlTree.findControlRegionIndex(lowerCtrlRegion)->second].upperControlRegion = ctrlRegion.controlRegionName;
            }

            // Delete non-loop controlRegion in each lowerCtrlRegion
            for (auto iter = ctrlRegion.lowerControlRegion.begin(); iter != ctrlRegion.lowerControlRegion.end(); )
            {
                if (loopHierarchy[_controlTree.findControlRegionIndex(*iter)->second].controlType != "Loop")
                {
                    iter = ctrlRegion.lowerControlRegion.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }
    }

    // Delete non-loop controlRegion
    for (auto iter = loopHierarchy.begin(); iter != loopHierarchy.end();)
    {
        if (iter->controlType != "Loop")
        {
            iter = loopHierarchy.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    return loopHierarchy;
}

vector<string> ChanGraph::findShortestCtrlRegionPath(vector<string>& _preNodeCtrlRegionPath, vector<string>& _nextNodeCtrlRegionPath)
{
    vector<string> ctrlRegionPath;
    auto s = _preNodeCtrlRegionPath;
    auto s_next = _nextNodeCtrlRegionPath;

    auto iter = s.end();
    if (_nextNodeCtrlRegionPath.empty())  // If empty, signify nextNode locates at the outer-most loop, so just return preNodeCtrlRegionPath
    {
        return _preNodeCtrlRegionPath;
    }
    else
    {
        iter = find(s.begin(), s.end(), _nextNodeCtrlRegionPath.front());
    }

    auto iter_next = s_next.end();
    if (_preNodeCtrlRegionPath.empty())  // If empty, signify preNode locates at the outer-most loop, so just return nextNodeCtrlRegionPath
    {
        return _nextNodeCtrlRegionPath;
    }
    else
    {
        iter_next = find(s_next.begin(), s_next.end(), _preNodeCtrlRegionPath.front());
    }

    //auto iter = find(s.begin(), s.end(), _nextNodeCtrlRegionPath.front());
    //auto iter_next = find(s_next.begin(), s_next.end(), _preNodeCtrlRegionPath.front());

    if (iter != s.end())
    {
        for (auto iter_ = s.begin(); iter_ != iter; ++iter_)
        {
            ctrlRegionPath.push_back(*iter_);
        }
        ctrlRegionPath.push_back(*iter);
    }
    else if (iter_next != s_next.end())
    {
        for (auto iter_ = s_next.begin(); iter_ != iter_next; ++iter_)
        {
            ctrlRegionPath.push_back(*iter_);
        }
        ctrlRegionPath.push_back(*iter_next);
        reverse(ctrlRegionPath.begin(), ctrlRegionPath.end());  // Reverse to make node -> node_next
    }
    else
    {
        vector<string> s_tmp;
        vector<string> s_next_tmp;
        auto iter_tmp = s.begin();
        auto iter_next_tmp = s_next.begin();
        string nodeCtrlRegion = *iter_tmp;
        string nextNodeCtrlRegion = *iter_next_tmp;

        while (nodeCtrlRegion != nextNodeCtrlRegion)
        {
            s_tmp.push_back(nodeCtrlRegion);
            s_next_tmp.push_back(nextNodeCtrlRegion);
            nodeCtrlRegion = *(++iter_tmp);
            nextNodeCtrlRegion = *(++iter_next_tmp);
        }

        s_tmp.push_back(*iter_tmp);  // Add the shared root ctrlRegion
        reverse(s_next_tmp.begin(), s_next_tmp.end());
        ctrlRegionPath.insert(ctrlRegionPath.end(), s_tmp.begin(), s_tmp.end());
        ctrlRegionPath.insert(ctrlRegionPath.end(), s_next_tmp.begin(), s_next_tmp.end());
    }

    return ctrlRegionPath;
}

vector<string> ChanGraph::bfsTraverseControlTree(ControlTree& ctrlTree)
{
    vector<string> bfsCtrlTree;
    deque<string> queue;
    if (!ctrlTree.controlRegionTable.empty())
    {
        queue.push_back(ctrlTree.controlRegionTable[0].controlRegionName);
    }

    while (!queue.empty())
    {
        for (auto& lowerCtrlRegion : ctrlTree.controlRegionTable[ctrlTree.findControlRegionIndex(queue.front())->second].lowerControlRegion)
        {
            queue.push_back(lowerCtrlRegion);
        }

        bfsCtrlTree.push_back(queue.front());
        queue.pop_front();
    }

    return bfsCtrlTree;
}

//vector<string> ChanGraph::bfsTraverseNode()
//{
//    vector<string> bfsNodeTree;
//    deque<string> queue = { "Chan_begin" };
//
//    while (!queue.empty())
//    {
//        vector<string> nextNodes;
//        nextNodes.insert(nextNodes.end(), getNode(queue.front())->next_nodes_data.begin(), getNode(queue.front())->next_nodes_data.end());
//        nextNodes.insert(nextNodes.end(), getNode(queue.front())->next_nodes_active.begin(), getNode(queue.front())->next_nodes_active.end());
//        for (auto& nextNode : nextNodes)
//        {
//            queue.push_back(nextNode);
//        }
//
//        bfsNodeTree.push_back(queue.front());
//        queue.pop_front();
//    }
//
//    // Remove redundant node
//    reverse(bfsNodeTree.begin(), bfsNodeTree.end());
//    vector<string> hasFound;
//    for (auto iter = bfsNodeTree.begin(); iter != bfsNodeTree.end(); )
//    {
//        if (find(hasFound.begin(), hasFound.end(), *iter) != hasFound.end())
//        {
//            iter = bfsNodeTree.erase(iter);
//        }
//        else
//        {
//            hasFound.push_back(*iter);
//            ++iter;
//        }
//    }
//    reverse(bfsNodeTree.begin(), bfsNodeTree.end());
//
//    return bfsNodeTree;
//}

vector<string> ChanGraph::backTrackPath(string& nodeName, vector<ControlRegion>& loopHierarchy)
{
    vector<string> backTrackPath;
    bool findNode = 0;
    for (auto& ctrlRegion : loopHierarchy)
    {
        for (auto& node : ctrlRegion.nodes)
        {
            if (node == nodeName)
            {
                backTrackPath.push_back(ctrlRegion.controlRegionName);
                findNode = 1;
                break;
            }
        }

        if (findNode)
        {
            break;
        }
    }

    deque<string> queue;
    if (!backTrackPath.empty())
    {
        queue.push_back(backTrackPath.front());
    }
    while (!queue.empty())
    {
        for (auto& ctrlRegion : loopHierarchy)
        {
            if (ctrlRegion.controlRegionName == queue.front())
            {
                queue.push_back(ctrlRegion.upperControlRegion);
                backTrackPath.push_back(ctrlRegion.upperControlRegion);
                break;
            }
        }
        queue.pop_front();
    }

    return backTrackPath;
}

void ChanGraph::addNodeDelay()
{
    for (auto& chanNode : nodes)
    {
        const auto& chanNodePtr = dynamic_cast<Chan_Node*>(chanNode);
        if (chanNodePtr->node_op == "Nop")
        {
            chanNodePtr->cycle = 0;
        }
        else if (chanNodePtr->node_op == "Mul")
        {
            chanNodePtr->cycle = MUL;
        }
        else if (chanNodePtr->node_op == "Add")
        {
            chanNodePtr->cycle = ADD;
        }
        else if (chanNodePtr->node_op == "Sub")
        {
            chanNodePtr->cycle = SUB;
        }
        else if (chanNodePtr->node_op == "Div")
        {
            chanNodePtr->cycle = DIV;
        }
        else
        {
            chanNodePtr->cycle = 1;
        }
    }
}

void ChanGraph::addChanDGSF()
{
    unordered_map<uint, vector<Node*>> subgraphNodes;
    for (auto& node : nodes)
    {
        if (subgraphNodes.count(node->subgraphId) == 0)
        {
            vector<Node*> tmp = { node };
            subgraphNodes.insert(make_pair(node->subgraphId, tmp));
        }
        else
        {
            subgraphNodes[node->subgraphId].push_back(node);
        }
    }

    for (auto& subgraph : subgraphNodes)
    {
        for (auto& node : subgraph.second)
        {
            bool insertDGSF = false;
            vector<string> nextNodes;
            nextNodes.insert(nextNodes.end(), node->next_nodes_data.begin(), node->next_nodes_data.end());
            nextNodes.insert(nextNodes.end(), node->next_nodes_active.begin(), node->next_nodes_active.end());
            for (auto& nextNode : nextNodes)
            {
                if (getNode(nextNode)->subgraphId != node->subgraphId)
                {
                    insertDGSF = true;
                    break;
                }
            }

            if (insertDGSF)
            {
                string nodeName = node->node_name + "_DGSF";
                addNode(nodeName, "ChanDGSF", "Nop", "Normal", node->controlRegionName);
                dynamic_cast<Chan_Node*>(getNode(nodeName))->isPhysicalChan = false;
                addNodes2CtrlTree(node->controlRegionName, { nodeName });
                getNode(nodeName)->subgraphId = node->subgraphId;
                const auto& chanDGSFNode = dynamic_cast<Chan_Node*>(getNode(nodeName));
                const auto& chanNode = dynamic_cast<Chan_Node*>(node);

                // Gen connect
                for (auto it = chanNode->next_nodes_data.begin(); it != chanNode->next_nodes_data.end(); /*auto& *it : chanNode->next_nodes_data*/)
                {
                    if (getNode(*it)->subgraphId != chanNode->subgraphId)
                    {
                        auto& preNodesData = getNode(*it)->pre_nodes_data;
                        auto iter = find(preNodesData.begin(), preNodesData.end(), chanNode->node_name);
                        if (iter != preNodesData.end())
                        {
                            *iter = chanDGSFNode->node_name;
                            chanDGSFNode->next_nodes_data.push_back(*it);
                            it = chanNode->next_nodes_data.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    else
                    {
                        ++it;
                    }
                }

                for (auto it = chanNode->next_nodes_active.begin(); it != chanNode->next_nodes_active.end(); )
                {
                    if (getNode(*it)->subgraphId != chanNode->subgraphId)
                    {
                        auto& preNodesActive = getNode(*it)->pre_nodes_active;
                        auto iter = find(preNodesActive.begin(), preNodesActive.end(), chanNode->node_name);
                        if (iter != preNodesActive.end())
                        {
                            *iter = chanDGSFNode->node_name;
                            chanDGSFNode->next_nodes_active.push_back(*it);
                            it = chanNode->next_nodes_active.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    else
                    {
                        ++it;
                    }
                }

                chanNode->next_nodes_data.push_back(chanDGSFNode->node_name);

                chanDGSFNode->pre_nodes_data.push_back(chanNode->node_name);

                // Check whether preNode is in keepMode
                if (chanNode->chan_mode == "Keep_mode")
                {
                    chanNode->chan_mode = "Normal";
                    chanDGSFNode->chan_mode = "Keep_mode";
                }

                // Note: Branch setting and graphScheduler configuration are in the Registry!

            }
        }
    }
}