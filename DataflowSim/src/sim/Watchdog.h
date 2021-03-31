#pragma once
#include "../define/Define.hpp"
#include "../module/execution/Channel.h"

namespace DFSim
{
    class Watchdog
    {
    public:
        Watchdog(uint _timer, uint _maxFeedTimes);
        void addCheckPointChan(vector<Channel*> _chanVec);
        void feedDog(uint _iter);
        void dogBark();

    private:
        uint timer;  // Interval time, recommend set to the power of 2 (e.g 128)
        uint maxFeedTimes;  // the maximum times of feed dog failure
        uint feedTimes = 0;
        uint iterExe = 0;
        vector<pair<Channel*, uint>> checkPointChans;  // First chanPtr, second chan_cnt
    };
}