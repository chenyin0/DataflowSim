#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"

namespace DFSim
{
    /**
     * ��- �޸�Ϊ��ȷ�ĵ����࣬clk��ȫ�־�̬������ʵ��
     * ��- ʹ��clk����������
     *
     * ʹ��ǰ�ȳ�ʼ�� ClkDomain::initClk()
     * ÿ�����ڵ����Լ� ClkDomain::selfAdd()
     * ����Ҫ�˹����ó�ʼֵ����ClkDomain::setClk(xxx)
     * ����ʱ���� ClkDomain::getClk()
     *
     * �� 2019.10.6
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