#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "../module/execution/Channel.h"
#include "../module/execution/Lc.h"
#include "../module/execution/Mux.h"

/*
Module Register

TODO:
1. Unify chanPtr, lcPtr, muxPtr to modulePtr

*/

namespace DFSim
{
    struct RegistryTable
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
        Registry();
        ~Registry();
        //* Do not use anymore, only keep to compatible with the old version, remove in the future
        static int registerChan(Channel* chan);
        static int registerLc(Lc* lc);
        static int registerMux(Mux* mux);
        //**
        static int registerChan(const string& moduleName_, const Channel* chan);
        static int registerLc(const string& moduleName_, const Lc* lc);
        static int registerMux(const string& moduleName_, const Mux* mux);
        void tableInit();
        void pathBalance();  // Resize channel buffer size to avoid path imbalance

#ifdef DEBUG_MODE  // Get private instance for debug
    public:
        const vector<RegistryTable>& getRegistryTable() const;
#endif // DEBUG_MODE

    private:
        friend class Profiler;

        void initLastTagQueue(Channel* _chan);  // Initial last tag vec for keepMode channel
        void initChanBuffer(Channel* _chan);
        void initBp(Channel* _chan);
        void initLastPopVal(Channel* _chan);
        void initChannel();  // Initial vector inputFifo's size and vector bp's size to the number of upstream

        void checkConnectRule();  // Check connection rules
        void checkChanConnect(Channel* _chan);  // Check the connection of all the channels
        void checkChanMode(Channel* _chan);  // Check the mode of all the channels
        void checkChanPartialMux(Channel* _chan);  // Check the connection rule of PartialMux
        void checkLc();  // Check whether set outer-most loop
        void checkChanDGSF(Channel* _chan);

    private:
        static uint moduleId;
        static vector<RegistryTable> registryTable;
        unordered_map<string, uint> registryDict;
    };

}