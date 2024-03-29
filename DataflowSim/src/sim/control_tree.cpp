#include "./control_tree.h"
#include "./Debug.h"

using namespace DFSim;

ControlTree::ControlTree()
{
}

void ControlTree::addControlRegion(const vector<tuple<string, string, string>>& controlRegions_)
{
    for (auto& controlRegion : controlRegions_)
    {
        if (controlRegionIndexDict.count(std::get<0>(controlRegion)))
        {
            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
        }
        else
        {
            controlRegionIndexDict.insert(pair<string, uint64_t>(std::get<0>(controlRegion), controlRegionTable.size()));
            if (std::get<1>(controlRegion) == "Loop")
            {
                controlRegionTable.push_back(ControlRegion(std::get<0>(controlRegion), std::get<1>(controlRegion)));
            }
            else if (std::get<1>(controlRegion) == "Branch")
            {
                if (std::get<2>(controlRegion) == "truePath")
                {
                    controlRegionTable.push_back(ControlRegion(std::get<0>(controlRegion), std::get<1>(controlRegion), true));
                }
                else if (std::get<2>(controlRegion) == "falsePath")
                {
                    controlRegionTable.push_back(ControlRegion(std::get<0>(controlRegion), std::get<1>(controlRegion), false));
                }
                else
                {
                    Debug::throwError("Illegal branchPath!", __FILE__, __LINE__);
                }
            }
            else
            {
                Debug::throwError("Illegal control region!", __FILE__, __LINE__);
            }
        }
    }
}

//void ControlTree::addNodes(const string& targetCtrlRegion, const vector<string>& nodes_)
//{
//    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
//    //if (iter != controlRegionIndexDict.end())
//    //{
//    //    uint64_t index = iter->second;
//    //    controlRegionTable[index].nodes.insert(controlRegionTable[index].nodes.end(), modules_.begin(), modules_.end());
//    //}
//    //else
//    //{
//    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
//    //}
//
//    uint64_t index = findControlRegionIndex(targetCtrlRegion)->second;
//    controlRegionTable[index].nodes.insert(controlRegionTable[index].nodes.end(), nodes_.begin(), nodes_.end());
//}

void ControlTree::addUpperControlRegion(const string& targetCtrlRegion, const string& ctrlRegions_)
{
    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
    //if (iter != controlRegionIndexDict.end())
    //{
    //    uint64_t index = iter->second;
    //    controlRegionTable[index].upperControlRegion = ctrlRegions_;
    //}
    //else
    //{
    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    //}

    if (!targetCtrlRegion.empty())
    {
        uint64_t index = findControlRegionIndex(targetCtrlRegion)->second;
        controlRegionTable[index].upperControlRegion = ctrlRegions_;
    }
}

void ControlTree::addLowerControlRegion(const string& targetCtrlRegion, const vector<string>& ctrlRegions_)
{
    //auto iter = controlRegionIndexDict.find(targetCtrlRegion);
    //if (iter != controlRegionIndexDict.end())
    //{
    //    uint64_t index = iter->second;
    //    controlRegionTable[index].lowerControlRegion.insert(controlRegionTable[index].lowerControlRegion.end(), ctrlRegions_.begin(), ctrlRegions_.end());
    //}
    //else
    //{
    //    Debug::throwError("Not find this controlRegion in controlRegionIndexDict!", __FILE__, __LINE__);
    //}
    if (!targetCtrlRegion.empty())
    {
        uint64_t index = findControlRegionIndex(targetCtrlRegion)->second;
        controlRegionTable[index].lowerControlRegion.insert(controlRegionTable[index].lowerControlRegion.end(), ctrlRegions_.begin(), ctrlRegions_.end());
    }
}

ControlRegion& ControlTree::getCtrlRegion(const string& controlRegionName_)
{
    return controlRegionTable[findControlRegionIndex(controlRegionName_)->second];
}

auto ControlTree::findControlRegionIndex(const string& controlRegionName_)->unordered_map<string, uint64_t>::iterator
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

vector<pair<string, uint64_t>> ControlTree::traverseControlRegionsDfs()
{
    vector<pair<string, uint64_t>> controlRegionsDfs = { make_pair(controlRegionTable[0].controlRegionName, 0) };  // pair<controlRegionName, level>
    //vector<string> controlRegionQueue = { control_tree[0].control_name };
    for (size_t ptr = 0; ptr < controlRegionsDfs.size(); ++ptr)
    {
        vector<string> lowerControlRegions = controlRegionTable[findControlRegionIndex(controlRegionsDfs[ptr].first)->second].lowerControlRegion;
        vector<pair<string, uint64_t>> tmp;
        uint64_t level = controlRegionsDfs[ptr].second + 1;
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

void ControlTree::completeControlRegionHierarchy()
{
    for (auto& controlRegion : controlRegionTable)
    {
        // Complete upperControlRegion
        for (auto& lowerRegion : controlRegion.lowerControlRegion)
        {
            addUpperControlRegion(lowerRegion, controlRegion.controlRegionName);
        }

        // Complete lowerControlRegion
        addLowerControlRegion(controlRegion.upperControlRegion, { controlRegion.controlRegionName });
    }

    // Remove redundancy in lowerControlRegion
    for (auto& controlRegion : controlRegionTable)
    {
        set<string> s(controlRegion.lowerControlRegion.begin(), controlRegion.lowerControlRegion.end());
        controlRegion.lowerControlRegion.assign(s.begin(), s.end());
    }
}