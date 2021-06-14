#include "./Profiler.h"
#include "../define/hw_para.hpp"

using namespace DFSim;

Profiler::Profiler(Registry* _registry, MemSystem* _memSys, Debug* _debugger)
{
    registry = _registry;
    memSys = _memSys;
    debug = _debugger;
}

Profiler::~Profiler()
{
    //if (registry != nullptr)
    //{
    //    delete registry;
    //}

    //if (debug != nullptr)
    //{
    //    delete debug;
    //}

    //if (memSys != nullptr)
    //{
    //    delete memSys;
    //}
}

void Profiler::init()
{
    profilingTable.resize(registry->registryTable.size());  // Indexed by moduleId
    maxDataNum.resize(registry->registryTable.size());
}

void Profiler::recordComputingCycle(uint _moduleId, ChanType _chanType, uint _clk)
{

}

void Profiler::printLseProfiling(string lseName, Lse* _lsePtr)
{
    double reqBlockRate = 100.0 * _lsePtr->memReqBlockCnt / _lsePtr->memReqCnt;
    //std::cout << _lsePtr->memReqBlockCnt << "\t" << _lsePtr->memReqCnt << std::endl;

    debug->getFile() << std::setw(20) << lseName 
        << "\taccessCnt: " << std::setw(5) << _lsePtr->memAccessCnt
        << "\taccessCntCoalesce: " << std::setw(5) << _lsePtr->memAccessCnt / std::min(_lsePtr->speedup, uint(BANK_BLOCK_SIZE/DATA_PRECISION))
        << "\tAvgLat: " << std::setw(5) << _lsePtr->avgMemAccessLat
        << "\treqBlockRate: " << std::fixed << setprecision(1) << std::setw(5) << reqBlockRate << "%"
        << "\treqBlockCnt: " << std::setw(5) << _lsePtr->memReqBlockCnt
        << "\treqCnt: " << std::setw(5) << _lsePtr->memReqCnt
        << std::endl;
}

void Profiler::printCacheMissRate()
{
    uint cacheLevel = CACHE_MAXLEVEL;
    for (size_t level = 0; level < cacheLevel; ++level)
    {
        double missRate = memSys->cache->get_miss_rate(level);
        debug->getFile() << "L" << level + 1 << "_miss_rate: " << std::fixed << setprecision(1) << missRate << "%" << std::endl;
    }
}

void Profiler::updateBufferMaxDataNum()
{
    uint cnt = 0;
    for (auto& entry : registry->registryTable)
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            Channel* chan = entry.chanPtr;
            for (auto& buffer : chan->chanBuffer)
            {
                cnt = std::max(cnt, buffer.size());
            }

            maxDataNum[chan->moduleId] = std::max(cnt, maxDataNum[chan->moduleId]);
        }
    }
}

void Profiler::printBufferMaxDataNum(string chanName, Channel* chanPtr)
{
    debug->getFile() << chanName << ":\t" << maxDataNum[chanPtr->moduleId] << std::endl;
}

void Profiler::updateChanUtilization()
{
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            if (entry.chanPtr->valid)
            {
                if (entry.chanPtr->branchMode)
                {
                    if (entry.chanPtr->channel.front().cond == entry.chanPtr->channelCond)
                    {
                        entry.chanPtr->activeCnt++;
                    }
                }
                else
                {
                    entry.chanPtr->activeCnt++;
                }
            }
        }
    }
}

//void Profiler::updateChanUtilization(uint _currSubgraphId)
//{
//    for (auto& entry : registry->getRegistryTable())
//    {
//        if (entry.moduleType == ModuleType::Channel)
//        {
//            if (entry.chanPtr->valid)
//            {
//                if (entry.chanPtr->branchMode)
//                {
//                    if (entry.chanPtr->channel.front().cond == entry.chanPtr->channelCond)
//                    {
//                        entry.chanPtr->activeCnt++;
//                    }
//                }
//                else
//                {
//                    entry.chanPtr->activeCnt++;
//                }
//
//                if (ClkDomain::checkClkAdd())
//                {
//                    ++entry.chanPtr->activeClkCnt;
//                }
//            }
//            else
//            {
//                if (entry.chanPtr->subgraphId == _currSubgraphId)
//                {
//                    if (ClkDomain::checkClkAdd())
//                    {
//                        ++entry.chanPtr->activeClkCnt;
//                    }
//                }
//            }
//
//        }
//    }
//}

void Profiler::updateChanUtilization(uint _currSubgraphId)
{
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            if (entry.chanPtr->pushChannelSuccess)
            {
                if (entry.chanPtr->branchMode)
                {
                    if (entry.chanPtr->channel.front().cond == entry.chanPtr->channelCond)
                    {
                        entry.chanPtr->activeCnt++;
                    }
                }
                else
                {
                    entry.chanPtr->activeCnt++;
                }
            }

            if (entry.chanPtr->subgraphId == _currSubgraphId ||
                entry.chanPtr->pushChannelSuccess ||
                entry.chanPtr->valid)
            {
                if (ClkDomain::checkClkAdd())
                {
                    ++entry.chanPtr->activeClkCnt;
                }
            }
        }
    }

    // For graphScheduler profiling
    if (_currSubgraphId != lastSubgraphId)
    {
        ++graphSwitchTimes;
        lastSubgraphId = _currSubgraphId;
    }
}

void Profiler::printChanProfiling()
{
    debug->getFile() << "******* Channel Utilization *********" << std::endl;

    uint chanNum = 0;
    uint avgWeight = 0;
    float chanUtilAvg = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            uint activeNum = entry.chanPtr->activeCnt;
            //float utilization = static_cast<float>(activeNum) / static_cast<float>((entry.chanPtr->speedup * entry.chanPtr->activeClkCnt/*ClkDomain::getClk()*/)) * 100;
            float utilization = std::min(static_cast<float>(activeNum) / static_cast<float>((entry.chanPtr->speedup * entry.chanPtr->activeClkCnt/*ClkDomain::getClk()*/)) * 100, float(100));
            //debug->getFile() << "ChanName: " << entry.chanPtr->moduleName << "\t" << std::fixed << utilization << setprecision(2) << "%" << std::endl;
            
            // TODO: Exclude channel in "Nop"
            if (entry.chanPtr->moduleName != "Chan_begin" &&
                entry.chanPtr->moduleName != "Chan_end" &&
                entry.chanPtr->isPhysicalChan &&
                (entry.chanPtr->masterName == "None" || entry.chanPtr->isLoopVar)/* &&
                (entry.chanPtr->keepMode != 1 && entry.chanPtr->drainMode != 1)*/)
            {
                debug->getFile() << "ChanName: " << entry.chanPtr->moduleName << "\t" << std::fixed << utilization << setprecision(2) << "%" << std::endl;
                ++chanNum;
                avgWeight += entry.chanPtr->activeClkCnt;
                chanUtilAvg += utilization * entry.chanPtr->activeClkCnt;

                chanActiveNumTotal += activeNum;
            }
        }
    }

    debug->getFile() << std::endl;
    debug->getFile() << "Avg channel utilization: " << std::fixed << float(chanActiveNumTotal * 100) / float(ARRAY_SIZE * ClkDomain::getClk())/*chanUtilAvg / avgWeight*/ << setprecision(2) << "%" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << "******* ALU/Reg Access Times *********" << std::endl;
    debug->getFile() << "Total ALU Active Times: " << chanActiveNumTotal << std::endl;
    debug->getFile() << "Total Reg Access Times: " << chanActiveNumTotal * 3 << std::endl;


    // Print the max data number in channel
    debug->getFile() << std::endl;
    debug->getFile() << "******* Max Data Num in Buffer *********" << std::endl;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            if (entry.chanPtr->moduleName != "Chan_begin"
                && entry.chanPtr->moduleName != "Chan_end"
                && entry.chanPtr->masterName == "None")
            {
                printBufferMaxDataNum(entry.chanPtr->moduleName, entry.chanPtr);
            }
        }
    }
}

void Profiler::printLseProfiling()
{
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Lse)
        {
            printLseProfiling(entry.chanPtr->moduleName, dynamic_cast<Lse*>(entry.chanPtr));
        }
    }
}

void Profiler::printPowerProfiling()
{
    // PE array
    //uint aluActiveTimes = chanActiveNumTotal;
    uint reg_active_times = chanActiveNumTotal * 3;
    uint pe_ctrl_active_times = chanActiveNumTotal;
    uint contextBuffer_active_times = graphSwitchTimes * ARRAY_SIZE;

    float alu_dynamic_energy = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Base)
        {
            alu_dynamic_energy += static_cast<float>(entry.chanPtr->activeCnt) * Hardware_Para::getAluDynamicEnergy();
        }
    }
    float alu_dynamic_power = transEnergy2Power(alu_dynamic_energy);
    float alu_leakage_power = Hardware_Para::getAluLeakagePower();
    float alu_power = alu_dynamic_power + alu_dynamic_power;

    float reg_dynamic_energy = static_cast<float>(reg_active_times) * Hardware_Para::getRegAccessEnergy();
    float reg_dynamic_power = transEnergy2Power(reg_dynamic_energy);
    float reg_leakage_power = Hardware_Para::getRegLeakagePower();
    float reg_power = reg_dynamic_power + reg_leakage_power;

    float pe_ctrl_dynamic_energy = static_cast<float>(pe_ctrl_active_times) * Hardware_Para::getPeCtrlEnergyDynamic();
    float pe_ctrl_dynamic_power = transEnergy2Power(pe_ctrl_dynamic_energy);
    float pe_ctrl_leakage_power = Hardware_Para::getPeCtrlLeakagePower();
    float pe_ctrl_power = pe_ctrl_dynamic_power + pe_ctrl_leakage_power;

    float contextBuffer_dynamic_energy = static_cast<float>(contextBuffer_active_times) * Hardware_Para::getContextBufferAccessEnergy();
    float contextBuffer_dynamic_power = transEnergy2Power(contextBuffer_dynamic_energy);
    float contextBuffer_leakage_power = Hardware_Para::getContextBufferLeakagePower();
    float contextBuffer_power = contextBuffer_dynamic_power + contextBuffer_leakage_power;

    // On-chip buffer
    uint dataBuffer_access_times = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel 
            && (entry.chanPtr->chanType == ChanType::Chan_Lse || entry.chanPtr->chanType == ChanType::Chan_DGSF))
        {
            uint coalesceRate = std::min(entry.chanPtr->speedup, uint(BANK_BLOCK_SIZE / DATA_PRECISION));
            dataBuffer_access_times += entry.chanPtr->activeCnt / coalesceRate;
        }
    }
    float dataBuffer_dynamic_energy = static_cast<float>(dataBuffer_access_times) * (Hardware_Para::getDataBufferAccessEnergy() + Hardware_Para::getDataBufferCtrlEnergy());
    float dataBuffer_dynamic_power = transEnergy2Power(dataBuffer_dynamic_energy);
    float dataBuffer_leakage_power = Hardware_Para::getDataBufferLeakagePower() + Hardware_Para::getDataBufferCtrlLeakagePower();
    float dataBuffer_power = dataBuffer_dynamic_power + dataBuffer_leakage_power;

    // GraphScheduler
    uint graphScheduler_active_times = graphSwitchTimes;
    float graphScheduler_dynamic_energy = static_cast<float>(graphScheduler_active_times) * Hardware_Para::getGraphSchedulerEnergyDynamic();
    float graphScheduler_dynamic_power = transEnergy2Power(graphScheduler_dynamic_energy);
    float graphScheduler_leakage_power = Hardware_Para::getGraphSchedulerLeakagePower();
    float graphScheduler_power = graphScheduler_dynamic_power + graphScheduler_leakage_power;

    float total_power = alu_power + 
                        reg_power + 
                        pe_ctrl_power + 
                        contextBuffer_power + 
                        dataBuffer_power + 
                        graphScheduler_power;

    debug->getFile() << std::endl;
    debug->getFile() << "******* Power profiling *********" << std::endl;
    debug->getFile() << ">>> PE Array: " << std::endl;
    debug->getFile() << "PE active total times: " << chanActiveNumTotal << std::endl;

    debug->getFile() << "ALU power: " << alu_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << alu_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << alu_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "Reg power: " << reg_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << reg_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << reg_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "Ctrl logic power: " << pe_ctrl_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << pe_ctrl_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << pe_ctrl_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "Context Buffer power: " << contextBuffer_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << contextBuffer_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << contextBuffer_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> On-chip Buffer: " << std::endl;
    debug->getFile() << "Access times: " << dataBuffer_access_times << std::endl;
    debug->getFile() << "On-chip buffer power: " << dataBuffer_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> Graph Scheduler: " << std::endl;
    debug->getFile() << "Graph switch times: " << graphScheduler_active_times << std::endl;
    debug->getFile() << "Graph switch power: " << graphScheduler_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << graphScheduler_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << graphScheduler_leakage_power << setprecision(4) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> Total power: "  << total_power << setprecision(2) << " mW" << std::endl;
}

float Profiler::transEnergy2Power(float _energy)
{
    float energy = _energy / static_cast<float>(ClkDomain::getClk());  // pJ
    float power = energy * static_cast<float>(FREQ);  // uW
    return power / 1000;  // Transfer to mW
}