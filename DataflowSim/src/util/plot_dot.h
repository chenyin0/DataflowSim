#pragma once
#include "../define/Define.hpp"
#include "../sim/graph.h"

namespace DFSim
{
    class PlotDot
    {
    public:
        static void plotDot(const Dfg& _dfg);
        static void plotDot(const ChanGraph& _chanGraph);

    private:
        static std::fstream dfg_out;
        static std::fstream chan_graph_out;
    };
}