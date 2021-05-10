#include "./control_tree.h"
#include "./Debug.h"

using namespace DFSim;

ControlTree::ControlTree()
{
}

void ControlTree::addControlRegion(const vector<pair<string, string>>& controlRegions_)
{
    for (auto& controlRegion : controlRegions_)
    {
        if (controlRegionIndexDict.count(controlRegion.first))
        {
            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
        }
        else
        {
            controlRegionIndexDict.insert(pair<string, uint>(controlRegion.first, controlRegionTable.size()));
            controlRegionTable.push_back(ControlRegion(controlRegion.first, controlRegion.second));
        }
    }
}

void ControlTree::addNodes(const string& targetCtrlRegion, const vector<string>& modules_)
{
    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
    //if (iter != controlRegionIndexDict.end())
    //{
    //    uint index = iter->second;
    //    controlRegionTable[index].nodes.insert(controlRegionTable[index].nodes.end(), modules_.begin(), modules_.end());
    //}
    //else
    //{
    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    //}

    uint index = findControlRegionIndex(targetCtrlRegion)->second;
    controlRegionTable[index].nodes.insert(controlRegionTable[index].nodes.end(), modules_.begin(), modules_.end());
}

void ControlTree::addUpperControlRegion(const string& targetCtrlRegion, const string& ctrlRegions_)
{
    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
    //if (iter != controlRegionIndexDict.end())
    //{
    //    uint index = iter->second;
    //    controlRegionTable[index].upperControlRegion = ctrlRegions_;
    //}
    //else
    //{
    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    //}

    uint index = findControlRegionIndex(targetCtrlRegion)->second;
    controlRegionTable[index].upperControlRegion = ctrlRegions_;
}

void ControlTree::addLowerControlRegion(const string& targetCtrlRegion, const vector<string>& ctrlRegions_)
{
    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
    //if (iter != controlRegionIndexDict.end())
    //{
    //    uint index = iter->second;
    //    controlRegionTable[index].lowerControlRegion.insert(controlRegionTable[index].lowerControlRegion.end(), ctrlRegions_.begin(), ctrlRegions_.end());
    //}
    //else
    //{
    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    //}

    uint index = findControlRegionIndex(targetCtrlRegion)->second;
    controlRegionTable[index].lowerControlRegion.insert(controlRegionTable[index].lowerControlRegion.end(), ctrlRegions_.begin(), ctrlRegions_.end());
}

auto ControlTree::findControlRegionIndex(const string& controlRegionName_)->unordered_map<string, uint>::iterator
{
    auto iter = controlRegionIndexDict.find(controlRegionName_);
    if (iter == controlRegionIndexDict.end())
    {
        Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    }
    else
    {
        return iter;
    }
}

vector<pair<string, uint>> ControlTree::traverseControlRegionsDfs()
{
    vector<pair<string, uint>> controlRegionsDfs = { make_pair(controlRegionTable[0].controlRegionName, 0) };  // pair<controlRegionName, level>
    //vector<string> controlRegionQueue = { control_tree[0].control_name };
    for (size_t ptr = 0; ptr < controlRegionsDfs.size(); ++ptr)
    {
        vector<string> lowerControlRegions = controlRegionTable[findControlRegionIndex(controlRegionsDfs[ptr].first)->second].lowerControlRegion;
        vector<pair<string, uint>> tmp;
        uint level = controlRegionsDfs[ptr].second + 1;
        for (auto& controlRegionName : lowerControlRegions)
        {
            tmp.push_back(make_pair(controlRegionName, level));
        }
        controlRegionsDfs.insert(controlRegionsDfs.begin() + ptr + 1, tmp.begin(), tmp.end());
    }

    return controlRegionsDfs;
}

void ControlTree::printSubgraphDot(std::fstream& fileName_, string& controlRegionName_)
{
    fileName_ << std::endl;
    fileName_ << "subgraph cluster_" << controlRegionName_ << " {" << std::endl;
    auto& controlRegion = controlRegionTable[findControlRegionIndex(controlRegionName_)->second];
    vector<string>& nodes_ = controlRegion.nodes;
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (i != nodes_.size() - 1)
        {
            fileName_ << nodes_[i] << ", ";
        }
        else
        {
            fileName_ << nodes_[i] << std::endl;
        }
    }

    if (controlRegion.controlType == "Loop")
    {
        fileName_ << "color=darkorange" << std::endl;
    }
    else if (controlRegion.controlType == "Branch")
    {
        fileName_ << "color=dodgerblue2" << std::endl;
    }

    fileName_ << "label=\"" << controlRegionName_ << "\"" << std::endl;  // Not print "}" here for nested subgraphs
}