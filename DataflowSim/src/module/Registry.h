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
    };

    class Registry
    {
    public:
        Registry();
        ~Registry();
        static int registerChan(Channel* chan);
        static int registerLc(Lc* lc);
        static int registerMux(Mux* mux);
        void tableInit();
        void pathBalance();  // Resize channel buffer size to avoid path imbalance

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

    private:
        static uint moduleId;
        static vector<RegistryTable> registryTable;
    };

}