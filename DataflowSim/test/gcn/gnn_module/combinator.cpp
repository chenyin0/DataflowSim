#include "combinator.h"
#include "../../../src/module/ClkSys.h"

using namespace DFSim;

CombPE::CombPE()
{
}

bool CombPE::PushData(Data &data)
{
    bool pushSuccess = false;
    if (buffer_.size() < buffer_size_)
    {
        data.cycle = ClkDomain::getInstance()->getClk() + cycle_;
        buffer_.push_back(data);
        pushSuccess = true;
    }

    return pushSuccess;
}

Data CombPE::PopData()
{
    Data data;
    data.valid = 0;

    if (!buffer_.empty())
    {
        data = buffer_.front();
        if (data.cycle > ClkDomain::getInstance()->getClk())
        {
            data.valid = 1;
        }
        buffer_.pop_front();
    }

    return data;
}

Combinator::Combinator(const uint64_t _pe_num, const uint64_t _pe_buffer_size, const uint64_t _cycle)
{
    pes_.resize(_pe_num);
    for (auto &pe : pes_)
    {
        pe.cycle_ = _cycle;
        pe.buffer_size_ = _pe_buffer_size;
        pe.buffer_.resize(_pe_buffer_size);
    }

    // inPort.first = vector<int>(_pe_num, -1);  // Init '-1' represents an invalid entry
    in_port_.resize(_pe_num);
    out_port_.resize(_pe_num);
}

void Combinator::Update()
{
    PushData(in_port_);
    PopData();
}

void Combinator::PushData(vector<Data> &data)
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

void Combinator::PopData()
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