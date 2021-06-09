#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "../module/execution/Channel.h"
#include "../module/execution/Lc.h"
#include "../module/execution/Mux.h"
//#include "../sim/Debug.h"
#include "../sim/graph.h"
#include "./mem/MemSystem.h"
#include "../module/execution/GraphScheduler.h"

/*
Module Register

TODO:
1. Unify chanPtr, lcPtr, muxPtr to modulePtr

*/

namespace DFSim
{
    struct RegistryTableEntry
    {
        Channel* chanPtr = nullptr;
        Lc* lcPtr = nullptr;
        Mux* muxPtr = nullptr;
        uint moduleId = 0;
        ModuleType moduleType;
        string moduleName;
    };

    class Registry
    {
    public:
        Registry(MemSystem* _memSys);
        ~Registry();
        //* Do not use anymore, only keep to compatible with the old version, remove in the future
        static int registerChan(Channel* chan);
        static int registerLc(Lc* lc);
        static int registerMux(Mux* mux);
        //**
        static int registerChan(const string& moduleName_, Channel* chan_);
        static int registerLc(const string& moduleName_, Lc* lc_);
        static int registerMux(const string& moduleName_, Mux* mux_);
        Channel* getChan(const string& moduleName_);
        Lc* getLc(const string& moduleName_);
        Mux* getMux(const string& moduleName_);
        Lse* getLse(const string& moduleName_);
        auto findRegistryEntryIndex(const string& _moduleName)->unordered_map<string, uint>::iterator;
        RegistryTableEntry& getRegistryTableEntry(const string& _moduleName);

        void init();
        void pathBalance();  // Resize channel buffer size to avoid path imbalance

        // Generate module
        void genModule(ChanGraph& _chanGraph);
        Lc* genLc(Chan_Node& _lc);
        Lc* genLcOuterMost(Chan_Node& _lc);
        Channel* genChan(Chan_Node& _chan);
        Lse* genLse(Chan_Node& _lse);
        Mux* genMux(Chan_Node& _mux);
        // Config graphScheduler
        void configGraphScheduler(GraphScheduler* _graphScheduler);
        // Generate interconnect
        void genConnect(ChanGraph& _chanGraph);
        // Gen configuration
        void genSimConfig(ChanGraph& _chanGraph);
        //void setSpeedup(ChanGraph& _chanGraph);  // Remove to ChanGraph
        void setChanSize();
        auto genDebugPrint(ChanGraph& _chanGraph) -> tuple<vector<Channel*>, vector<Lc*>>;
        // simulation
        void sim();
        void updateChanDGSF();


        /*template <typename T>
        static int registerModule(const string& moduleName_, T module_)
        {
            if (Registry::registryTable.size() != Registry::moduleId)
            {
                Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
                return -1;
            }

            auto iter = registryDict.find(moduleName_);
            if (iter != registryDict.end())
            {
                Debug::throwError("There already has a same name module in registryDict!", __FILE__, __LINE__);
                return -1;
            }

            RegistryTableEntry entry;
            if (std::is_same<decltype(module_), decltype(entry.chanPtr)>::value)
            {
                entry.chanPtr = module_;
                entry.moduleType = ModuleType::Channel;
            }
            else if (std::is_same<decltype(module_), Lc*>::value)
            {
                entry.lcPtr = module_;
                entry.moduleType = ModuleType::Lc;
            }
            else if (std::is_same<decltype(module_), Mux*>::value)
            {
                entry.muxPtr = module_;
                entry.moduleType = ModuleType::Mux;
            }
            
            entry.moduleId = Registry::moduleId; 
            entry.moduleName = moduleName_;
            registryDict.insert(pair<string, uint>(moduleName_, registryTable.size()));
            registryTable.push_back(entry);

            return Registry::moduleId++;
        }*/

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<RegistryTableEntry>& getRegistryTable() const;
#endif // DEBUG_MODE

    private:
        friend class Profiler;

        void initLastTagQueue(Channel* _chan);  // Initial last tag vec for keepMode channel
        void initChanBuffer(Channel* _chan);
        void initBp(Channel* _chan);
        void initLastPopVal(Channel* _chan);
        void initAluInput(Channel* _chan);
        void initChannel();  // Initial vector inputFifo's size and vector bp's size to the number of upstream
        void initLse(Lse* _lse);  // Initial Lse reqQueue size
        void initChanDGSFVec();  // Push chanDGSF in vecChanDGSF

        void checkConnectRule();  // Check connection rules
        void checkChanConnect(Channel* _chan);  // Check the connection of all the channels
        void checkChanMode(Channel* _chan);  // Check the mode of all the channels
        void checkChanPartialMux(Channel* _chan);  // Check the connection rule of PartialMux
        void checkLc();  // Check whether set outer-most loop
        void checkChanDGSF(Channel* _chan);

        void setSpeedup(ChanGraph& _chanGraph, const string& _controlRegion, uint _speedup);
        //uint getCtrlRegionPhysicalNodeNum(ChanGraph& _chanGraph, const string& _controlRegion);

    private:
        static uint moduleId;
        static vector<RegistryTableEntry> registryTable;
        static unordered_map<string, uint> registryDict;
        vector<ChanDGSF*> vecChanDGSF;

        MemSystem* memSys = nullptr;
    };

}