#pragma once
#include "./Define.hpp"

namespace DFSim {

    class Hardware_Para
    {
    public:
        // PE
        static float getAluDynamicEnergy()
        {
            // TODO: return according to the PE's op
            static float mul_32b = 4.0125;  // pJ (Mul)
            static float add_32b = 0.0501;  // pJ
            return mul_32b * 0.5 + add_32b * 0.5;
        }

        static float getAluLeakagePower()
        {
            static float alu_leakage_power = 0.237;  // mW (or 0.151)
            return alu_leakage_power;
        }

        static float getPeCtrlEnergyDynamic()
        {
            static float peCtrlEnergy = 0.158;  // pJ
            return peCtrlEnergy;
        }

        static float getPeCtrlLeakagePower()
        {
            static float pe_ctrl_leakage_power = 0.000692;  // mW
            return pe_ctrl_leakage_power;
        }

        static float getTiaPeCtrlEnergyDynamic()
        {
            static float tia_pe_ctrl_energy = 1.719;  // pJ
            return tia_pe_ctrl_energy;
        }

        static float getTiaPeCtrlLeakagePower()
        {
            static float tia_pe_ctrl_leakage_power = 0.0443;  // mW
            return tia_pe_ctrl_leakage_power;
        }

        static float getRegAccessEnergy()
        {
            static float regAccessEnergy = 0.317;  // pJ
            return regAccessEnergy;
        }

        static float getRegLeakagePower()
        {
            static float reg_leakage_power = 0.000634;  // mW
            return reg_leakage_power;
        }

        static float getContextBufferAccessEnergy()  // 48bit * 8 entry
        {
            static float contextBufferAccessEnergy = 3.867;  // pJ
            return contextBufferAccessEnergy;
        }

        static float getContextBufferLeakagePower()  // 48bit * 8 entry
        {
            static float contextBuffer_leakage_power = 0.0376;  // mW
            return contextBuffer_leakage_power;
        }

        static float getTiaInstructionBufferAccessEnergy()  // 128bit * 16 entry
        {
            static float tia_instr_buffer_access_energy = 4.359;  // pJ
            return tia_instr_buffer_access_energy;
        }

        static float getTiaInstructionBufferLeakagePower()  // 128bit * 16 entry
        {
            static float tia_instr_buffer_leakage_power = 0.0808;  // mW
            return tia_instr_buffer_leakage_power;
        }

        // On-chip buffer
        static float getDataBufferAccessEnergy()
        {
            static float dataBufferAccessEnergy = 15;  // pJ  (128KB, 16 Bank, 64B)
            return dataBufferAccessEnergy;
        }

        static float getDataBufferLeakagePower()
        {
            static float dataBuffer_leakage_power = 14.998;  // mW (64KB, 16 Bank, 64B)
            return dataBuffer_leakage_power;
        }

        static float getDataBufferCtrlEnergy()
        {
            static float dataBufferCtrlEnergy = 4.489;  // pJ
            return dataBufferCtrlEnergy;
        }

        static float getDataBufferCtrlLeakagePower()
        {
            static float dataBuffer_ctrl_leakage_power = 0.0166;  // mW
            return dataBuffer_ctrl_leakage_power;
        }

        // Graph scheduler
        static float getGraphSchedulerEnergyDynamic()
        {
            static float graphSchedulerEnergy = 1.719;  // 0.322 // pJ
            return graphSchedulerEnergy;
        }

        static float getGraphSchedulerLeakagePower()
        {
            static float graphScheduler_leakage_power = 0.0083;  // mW
            return graphScheduler_leakage_power;
        }
    };
}