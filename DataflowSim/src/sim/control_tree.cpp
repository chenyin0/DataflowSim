#include "./control_tree.h"
#include "./Debug.h"

using namespace DFSim;

ControlTree::ControlTree()
{
}

void ControlTree::addControlRegion(const vector<string>& controlRegions_)
{
    for (auto& controlRegionName : controlRegions_)
    {
        if (controlRegionIndexDict.count(controlRegionName))
        {
            Debug::throwError("There already has a same name control region!", __FILE__, __LINE__);
        }
        else
        {
            controlRegionIndexDict.insert(pair<string, uint>(controlRegionName, controlRegionTable.size()));
            controlRegionTable.push_back(ControlRegion(controlRegionName));
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