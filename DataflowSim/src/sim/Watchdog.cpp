#include "./Watchdog.h"
#include "../module/ClkSys.h"

using namespace DFSim;

Watchdog::Watchdog(uint64_t _timer, uint64_t _maxFeedTimes) : 
    timer(_timer), maxFeedTimes(_maxFeedTimes)
{
}

void Watchdog::addCheckPointChan(vector<Channel*> _chanVec)
{
    for (auto& chanPtr : _chanVec)
    {
        checkPointChans.emplace_back(std::make_pair(chanPtr, 0));
    }
}

void Watchdog::feedDog(uint64_t _iter)
{
    bool feedSuccessful = 0;
    if (_iter % timer == 0)
    {
        for (auto& chanPtr : checkPointChans)
        {
            if (chanPtr.first->chanDataCnt != chanPtr.second)
            {
                feedSuccessful = 1;
                chanPtr.second = chanPtr.first->chanDataCnt;
            }
        }

        if (feedSuccessful)
        {
            feedTimes = 0;
            iterExe = _iter;
        }
        else
        {
            feedTimes++;
        }
    }

    if (feedTimes == maxFeedTimes)
    {
        dogBark();
    }
}

void Watchdog::dogBark()
{
    std::cout << std::endl;
    std::cout << ">>>>>>>>>> Crash!!! Simulation Runaway!!! >>>>>>>>>>" << std::endl;
    std::cout << ">>> Simulation Crash @ iter = " << iterExe << std::endl;
    DEBUG_ASSERT(false);
}