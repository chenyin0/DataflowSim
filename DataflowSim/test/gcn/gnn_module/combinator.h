#pragma once
#include "../../../src/define/Define.hpp"
#include "../../../src/module/DataType.h"

namespace DFSim
{
    class CombPE
    {
    public:
        CombPE();
        bool PushData(Data &data);
        Data PopData();

        // bool finish = false;
        // bool hasPop = false;
        deque<Data> buffer_;
        // uint64_t v_tag;
        uint64_t cycle_;
        uint64_t buffer_size_;
    };

    class Combinator
    {
    public:
        Combinator(const uint64_t pe_num, const uint64_t pe_buffer_size, const uint64_t cycle);
        // void init();
        void Update();
        void PopData();
        // uint64_t checkEmptySlot();
        void PushData(vector<Data> &data);

        vector<CombPE> pes_;
        // uint64_t cycle;
        // uint64_t pe_buffer_size;
        vector<Data> in_port_; // Pair<vecPort_ID, inData>
        vector<Data> out_port_;
        uint64_t push_data_cnt_ = 0;
        uint64_t pop_data_cnt_ = 0;
    };
}
