#pragma once
#include "./Define.hpp"

namespace DFSim {

    class Hardware_Para
    {
    public:
        // PE
        static float getAluEnergy()
        {
            static float aluEnergy = 4.0125;  // pJ
            return aluEnergy;
        }

        static float getPeCtrlEnergy()
        {
            static float peCtrlEnergy = 0.158;  // pJ
            return peCtrlEnergy;
        }

        static float getRegAccessEnergy()
        {
            static float regAccessEnergy = 0.317;  // pJ
            return regAccessEnergy;
        }

        static float getContextBufferAccessEnergy()
        {
            static float contextBufferAccessEnergy = 3.867;  // pJ
            return contextBufferAccessEnergy;
        }

        // On-chip buffer
        static float getDataBufferAccessEnergy()
        {
            static float dataBufferAccessEnergy = 1020;  // pJ
            return dataBufferAccessEnergy;
        }

        static float getDataBufferCtrlEnergy()
        {
            static float dataBufferCtrlEnergy = 4.489;  // pJ
            return dataBufferCtrlEnergy;
        }

        // Graph scheduler
        static float getGraphSchedulerEnergy()
        {
            static float graphSchedulerEnergy = 0;
            return graphSchedulerEnergy;
        }
    };
}