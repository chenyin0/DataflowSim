#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
    struct ControlRegion
    {
        ControlRegion(const string& controlRegionName_) : controlRegionName(controlRegionName_) {}

        string controlRegionName;
        string upperControlRegion;
        vector<string> lowerControlRegion;
        vector<string> nodes;  // modules in this control region
    };

    class ControlTree
    {
    public:
        ControlTree();
        auto findControlRegionIndex(const string& controlRegionName_)->unordered_map<string, uint>::iterator;
        void addControlRegion(const vector<string>& controlRegions_);
        void addNodes(const string& targetCtrlRegion, const vector<string>& modules_);
        void addUpperControlRegion(const string& targetCtrlRegion, const string& ctrlRegions_);
        void addLowerControlRegion(const string& targetCtrlRegion, const vector<string>& ctrlRegions_);

    private:
        vector<ControlRegion> controlRegionTable;
        unordered_map<string, uint> controlRegionIndexDict;
    };
}