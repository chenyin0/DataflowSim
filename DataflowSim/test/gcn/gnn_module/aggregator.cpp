#include "aggregator.h"
#include "../../../src/module/ClkSys.h"

using namespace DFSim;

AggrPE::AggrPE()
{
}

bool AggrPE::PushData(Data &data)
{
    bool pushSuccess = false;
    if (data.tag == v_tag_)
    {
        reg_.cycle += cycle_;
    }
    else
    {
        finish = true;
    }

    if (data.valid && hasPop)
    {
        reg_ = data;
        v_tag_ = data.tag;
        reg_.cycle = ClkDomain::getInstance()->getClk() + cycle_;
        hasPop = false;
        pushSuccess = true;
    }

    return pushSuccess;
}

Data AggrPE::PopData()
{
    Data data = reg_;
    if (finish && reg_.cycle >= ClkDomain::getInstance()->getClk())
    {
        data.valid = 1;
        hasPop = true;
    }
    else
    {
        data.valid = 0;
    }

    return data;
}

Aggregator::Aggregator(const uint64_t _pe_num, const uint64_t _cycle)
{
    pes_.resize(_pe_num);
    for (auto &pe : pes_)
    {
        pe.cycle_ = _cycle;
    }

    // inPort.first = vector<int>(_pe_num, -1);  // Init '-1' represents an invalid entry
    in_port_.resize(_pe_num);
    out_port_.resize(_pe_num);
}

void Aggregator::Update()
{
    PushData(in_port_);
    PopData();
}

void Aggregator::PushData(vector<Data> &data)
{
    for (auto i = 0; i < data.size(); ++i)
    {
        auto &pe = pes_[i];
        if (pe.PushData(data[i]))
        {
            data[i].valid = 0;
            ++push_data_cnt_;
        }
    }
}

void Aggregator::PopData()
{
    // Issue ready data to outPort
    for (auto i = 0; i < out_port_.size(); ++i)
    {
        if (out_port_[i].valid == 0)
        {
            auto data = pes_[i].PopData();
            if (data.valid)
            {
                out_port_[i] = data;
                ++pop_data_cnt_;
            }
        }
    }
}