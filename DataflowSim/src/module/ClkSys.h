#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"

namespace DFSim
{
    /**
     * ！- 修改为正确的单例类，clk用全局静态变量来实现
     * ！- 使用clk用类名调用
     *
     * 使用前先初始化 ClkDomain::initClk()
     * 每个周期调用自加 ClkDomain::selfAdd()
     * 若需要人工设置初始值，用ClkDomain::setClk(xxx)
     * 采样时钟用 ClkDomain::getClk()
     *
     * 行 2019.10.6
     */
    class ClkDomain : public Singleton<ClkDomain>
    {
    private:
        friend Singleton<ClkDomain>;

        ClkDomain() = default;

    public:
        static uint getClk();

        static void selfAdd();

        static void setClk(uint clk_);

        static void initClk();

        static void clkUpdate();

        static void addClkStall();

        static bool checkClkAdd();

        static const string getCurrentSystemTime();

    private:
        static uint _clk;
        static bool clkAdd;  // Signify whether clock update in this cycle
        static deque<bool> clkStall;
        static void resetClkStall();
    };


}