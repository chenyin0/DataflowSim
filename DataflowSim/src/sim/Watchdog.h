#pragma once
#include "../define/Define.hpp"
#include "../module/execution/Channel.h"

namespace DFSim
{
    class Watchdog
    {
    public:
        Watchdog(uint64_t _timer, uint64_t _maxFeedTimes);
        void addCheckPointChan(vector<Channel*> _chanVec);
        void feedDog(uint64_t _iter);
        void dogBark();
        inline uint64_t getFeedTimes()
        {
            return feedTimes;
        }

    private:
        uint64_t timer;  // Interval time, recommend set to the power of 2 (e.g 128)
        uint64_t maxFeedTimes;  // the maximum times of feed dog failure
        uint64_t feedTimes = 0;
        uint64_t iterExe = 0;
        vector<pair<Channel*, uint64_t>> checkPointChans;  // First chanPtr, second chan_cnt
    };
}