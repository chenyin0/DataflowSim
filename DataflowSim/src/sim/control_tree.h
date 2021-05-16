#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
    struct ControlRegion
    {
        ControlRegion(const string& controlRegionName_, const string& controlRegionType_) 
            : controlRegionName(controlRegionName_), controlType(controlRegionType_) {}
        ControlRegion(const string& controlRegionName_, const string& controlRegionType_, const bool& branchPath_)
            : controlRegionName(controlRegionName_), controlType(controlRegionType_), branchPath(branchPath_) {}

        string controlRegionName;
        string upperControlRegion;
        vector<string> lowerControlRegion;
        vector<string> nodes;  // modules in this control region
        string controlType;  // <Loop, Branch>
        bool branchPath = true;  // true = true path; false = false path
    };

    class ControlTree
    {
    public:
        ControlTree();
        auto findControlRegionIndex(const string& controlRegionName_)->unordered_map<string, uint>::iterator;
        void addControlRegion(const vector<tuple<string, string, string>>& controlRegions_);  // pair<controlRegionName, controlRegionType>
        //void addNodes(const string& targetCtrlRegion, const vector<string>& nodes_);
        void addUpperControlRegion(const string& targetCtrlRegion, const string& ctrlRegions_);
        void addLowerControlRegion(const string& targetCtrlRegion, const vector<string>& ctrlRegions_);
        void completeControlRegionHierarchy();
        vector<pair<string, uint>> traverseControlRegionsDfs();
        void printSubgraphDot(std::fstream& fileName_, string& controlRegionName_);

    public:
        vector<ControlRegion> controlRegionTable;
        unordered_map<string, uint> controlRegionIndexDict;
    };
}