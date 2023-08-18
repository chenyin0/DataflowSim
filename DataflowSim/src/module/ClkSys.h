#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"

namespace DFSim
{
    /**
     * Implement clk with global static variable
     * Call clk by class ClkDomain
     *
     * Initial at first: ClkDomain::initClk()
     * Call selfAdd each cycle to update clk: ClkDomain::selfAdd()
     * Set the initial value manually if need: ClkDomain::setClk(xxx)
     * Get current clk: ClkDomain::getClk()
     *
     * 2019.10.6
     */
    class ClkDomain : public Singleton<ClkDomain>
    {
    private:
        friend Singleton<ClkDomain>;

        ClkDomain() = default;

    public:
        static uint64_t getClk();

        static void selfAdd();

        static void setClk(uint64_t clk_);

        static void initClk();

        static void clkUpdate();

        static void addClkStall();

        static bool checkClkAdd();

        static const string getCurrentSystemTime();

    private:
        static uint64_t _clk;
        static bool clkAdd;  // Signify whether clock update in this cycle
        static deque<bool> clkStall;
        static void resetClkStall();
    };


}