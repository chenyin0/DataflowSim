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

void Profiler::printCacheProfiling()
{
    if (memSys->cache != nullptr)
    {
        debug->getFile() << "Cache access times: " << memSys->cache->getCacheAccessCnt() << std::endl;
        printCacheMissRate();
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
                    //if (entry.chanPtr->channel.front().cond == entry.chanPtr->channelCond)
                    if (entry.chanPtr->channel.front().cond)
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
                    //if (entry.chanPtr->channel.front().cond == entry.chanPtr->channelCond)
                    if (entry.chanPtr->channel.front().cond)
                    {
                        entry.chanPtr->activeCnt++;
                    }
                }
                else
                {
                    entry.chanPtr->activeCnt++;
                }
            }

            if (entry.chanPtr->subgraphId == _currSubgraphId
                || entry.chanPtr->pushChannelSuccess
                /*|| entry.chanPtr->valid
                && entry.chanPtr->enable*/)
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

void Profiler::printChanProfiling(GraphScheduler* _graphScheduler)
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
            if (entry.chanPtr->moduleName != "Chan_begin"
                && entry.chanPtr->moduleName != "Chan_end"
                && entry.chanPtr->isPhysicalChan
                && (entry.chanPtr->masterName == "None" || entry.chanPtr->isLoopVar)/* &&
                (entry.chanPtr->keepMode != 1 && entry.chanPtr->drainMode != 1)*/
                /*&& entry.chanPtr->moduleName != "Chan_sum_update_k1_drain"
                && entry.chanPtr->moduleName != "Lse_a_update_j1"
                && entry.chanPtr->moduleName != "Chan_sum_update_k2_drain"
                && entry.chanPtr->moduleName != "Chan_sum_div"
                && entry.chanPtr->moduleName != "Lse_a_update_j2"*/)
            {
                debug->getFile() << "ChanName: " << entry.chanPtr->moduleName << "\t" << std::fixed << utilization << setprecision(2) << "%" << std::endl;
                ++chanNum;
                avgWeight += entry.chanPtr->activeClkCnt;
                chanUtilAvg += utilization * entry.chanPtr->activeClkCnt;

                chanActiveNumTotal += activeNum;
            }
        }
    }

    vector<uint> subgraphNodeNum(_graphScheduler->subgraphActiveCnt.size());
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel 
            && entry.chanPtr->isPhysicalChan
            && (entry.chanPtr->masterName == "None" || entry.chanPtr->isLoopVar)
            && entry.chanPtr->moduleName != "Chan_begin"
            && entry.chanPtr->moduleName != "Chan_end"
            /*&& entry.chanPtr->moduleName != "Chan_sum_update_k1_drain"
            && entry.chanPtr->moduleName != "Lse_a_update_j1"
            && entry.chanPtr->moduleName != "Chan_sum_update_k2_drain"
            && entry.chanPtr->moduleName != "Chan_sum_div"
            && entry.chanPtr->moduleName != "Lse_a_update_j2"*/)
        {
            subgraphNodeNum[entry.chanPtr->subgraphId]++;
        }
    }

    uint totalPeCycle = 0;
    if (subgraphNodeNum.size() > 1)
    {
        for (size_t subgraphId = 0; subgraphId < subgraphNodeNum.size(); ++subgraphId)
        {
            totalPeCycle += (ARRAY_SIZE / subgraphNodeNum[subgraphId]) * subgraphNodeNum[subgraphId] * _graphScheduler->subgraphActiveCnt[subgraphId];
            std::cout << "graphId: " << subgraphId 
                << "\t ActiveNum: " << _graphScheduler->subgraphActiveCnt[subgraphId] 
                << "\t NodeNum: " << (ARRAY_SIZE / subgraphNodeNum[subgraphId]) * subgraphNodeNum[subgraphId]
                << "\t Speedup: " << (ARRAY_SIZE / subgraphNodeNum[subgraphId]) << std::endl;
        }
        // Debug_yin_21.08.27
        //totalPeCycle = ARRAY_SIZE * ClkDomain::getClk();
    }
    else
    {
        totalPeCycle = (ARRAY_SIZE / subgraphNodeNum[0]) * subgraphNodeNum[0] * ClkDomain::getClk();
        //totalPeCycle = ARRAY_SIZE * ClkDomain::getClk();
    }

    //float avgChanUtilization = float(chanActiveNumTotal * 100) / float(ARRAY_SIZE * ClkDomain::getClk());
    float avgChanUtilization = float(chanActiveNumTotal * 100) / float(totalPeCycle);
    //std::cout << "chanActiveNumTotal:       " << chanActiveNumTotal << std::endl;
    //float avgChanUtilization = chanUtilAvg / avgWeight;
    debug->getFile() << std::endl;
    debug->getFile() << "Avg channel utilization: " << avgChanUtilization << setprecision(2) << "%" << std::endl;
    std::cout << "Avg channel utilization: " << avgChanUtilization << setprecision(2) << "%" << std::endl;

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

void Profiler::printDramProfiling()
{
    if (memSys != nullptr)
    {
        debug->getFile() << "DRAM access times: " << memSys->getMemAccessCnt() << std::endl;
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
    //for (auto& entry : registry->getRegistryTable())
    //{
    //    if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Base)
    //    {
    //        alu_dynamic_energy += static_cast<float>(entry.chanPtr->activeCnt) * Hardware_Para::getAluDynamicEnergy();
    //    }
    //}
    alu_dynamic_energy = chanActiveNumTotal * Hardware_Para::getAluDynamicEnergy();
    float alu_dynamic_power = transEnergy2Power(alu_dynamic_energy);
    float alu_leakage_power = Hardware_Para::getAluLeakagePower();
    float alu_power = alu_dynamic_power + alu_leakage_power;

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
    uint dataBuffer_mem_req_access_times = 0;
    uint dataBuffer_intermediate_data_access_times = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            uint coalesceRate = std::min(entry.chanPtr->speedup, uint(BANK_BLOCK_SIZE / DATA_PRECISION));
            if (entry.chanPtr->chanType == ChanType::Chan_Lse)
            {
                dataBuffer_mem_req_access_times += entry.chanPtr->activeCnt / coalesceRate;
            }
            else if (entry.chanPtr->chanType == ChanType::Chan_DGSF)
            {
                dataBuffer_intermediate_data_access_times += entry.chanPtr->activeCnt / coalesceRate;
            }
        }
    }
    dataBuffer_access_times = dataBuffer_mem_req_access_times + dataBuffer_intermediate_data_access_times;

    // On-chip buffer ctrl
    float dataBuffer_ctrl_dynamic_energy = static_cast<float>(dataBuffer_access_times) * Hardware_Para::getDataBufferCtrlEnergy();
    float dataBuffer_ctrl_dynamic_power = transEnergy2Power(dataBuffer_ctrl_dynamic_energy);
    float dataBuffer_ctrl_leakage_power = Hardware_Para::getDataBufferCtrlLeakagePower();
    float dataBuffer_ctrl_power = dataBuffer_ctrl_dynamic_power + dataBuffer_ctrl_leakage_power;
    // On-chip buffer sram
    float dataBuffer_sram_dynamic_energy = static_cast<float>(dataBuffer_access_times) * Hardware_Para::getDataBufferAccessEnergy();
    float dataBuffer_sram_dynamic_power = transEnergy2Power(dataBuffer_sram_dynamic_energy);
    float dataBuffer_sram_leakage_power = Hardware_Para::getDataBufferLeakagePower();
    float dataBuffer_sram_power = dataBuffer_sram_dynamic_power + dataBuffer_sram_leakage_power;
    // On-chip buffer total
    float dataBuffer_dynamic_power = dataBuffer_ctrl_dynamic_power + dataBuffer_sram_dynamic_power;
    float dataBuffer_leakage_power = dataBuffer_ctrl_leakage_power + dataBuffer_sram_leakage_power;
    float dataBuffer_power = dataBuffer_dynamic_power + dataBuffer_leakage_power;

    // GraphScheduler
    //uint graphScheduler_active_times = graphSwitchTimes;
    uint graphScheduler_active_times = ClkDomain::getClk();
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

    // Cache
    uint cache_access_times = 0;
    if (memSys != nullptr && memSys->cache != nullptr)
    {
        cache_access_times += memSys->cache->getCacheAccessCnt();
    }

    // DRAM
    uint mem_access_times = 0;
    if (memSys != nullptr)
    {
        mem_access_times += memSys->getMemAccessCnt();
    }

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
    debug->getFile() << "\t Mem req access times: " << dataBuffer_mem_req_access_times << std::endl;
    debug->getFile() << "\t Intermediate data access times: " << dataBuffer_intermediate_data_access_times << std::endl;
    debug->getFile() << "Buffer total power: " << dataBuffer_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_leakage_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "Buffer ctrl power: " << dataBuffer_ctrl_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_ctrl_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_ctrl_leakage_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "Buffer sram power: " << dataBuffer_sram_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_sram_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_sram_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> Graph Scheduler: " << std::endl;
    debug->getFile() << "Graph switch times: " << graphSwitchTimes << std::endl;
    debug->getFile() << "Graph scheduler power: " << graphScheduler_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << graphScheduler_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << graphScheduler_leakage_power << setprecision(4) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> Total power: "  << total_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << ">>> EDP: " << pow(ClkDomain::getClk() / 1000, 2) * (total_power / 1000.0) << setprecision(2) << std::endl;

    // Print
    std::cout << std::endl;
    std::cout << "Total power: " << total_power << setprecision(2) << " mW" << std::endl;
    std::cout << "GraphSwitch Times: " << graphSwitchTimes << std::endl;
    std::cout << std::endl;
    std::cout << "Mem req access times: " << dataBuffer_mem_req_access_times << std::endl;
    std::cout << "Intermediate access times: " << dataBuffer_intermediate_data_access_times << std::endl;
    std::cout << "Cache access times: " << cache_access_times << std::endl;
    std::cout << "DRAM access times: " << mem_access_times << std::endl;
    std::cout << "EDP: " << pow(ClkDomain::getClk() / 1000, 2) * (total_power / 1000.0) << setprecision(2) << std::endl;

    std::cout << std::endl;
    std::cout << "ALU power: " << alu_power << setprecision(2) << " mW" << std::endl;
    std::cout << "Reg power: " << reg_power << setprecision(2) << " mW" << std::endl;
    std::cout << "PE-ctrl power: " << pe_ctrl_power << setprecision(2) << " mW" << std::endl;
    std::cout << "Instr power: " << 0 << setprecision(2) << " mW" << std::endl;
    std::cout << "Context power: " << contextBuffer_power << setprecision(2) << " mW" << std::endl;
    std::cout << "DMem-sram power: " << dataBuffer_sram_power << setprecision(2) << " mW" << std::endl;
    std::cout << "DMem-ctrl power: " << dataBuffer_ctrl_power << setprecision(2) << " mW" << std::endl;
    std::cout << "Graph-sched. power: " << graphScheduler_power << setprecision(2) << " mW" << std::endl;
    std::cout << std::endl;
}

float Profiler::transEnergy2Power(float _energy)
{
    return transEnergy2Power(_energy, ClkDomain::getClk());
}

float Profiler::transEnergy2Power(float _energy, uint _cycle)
{
    float energy = _energy / static_cast<float>(_cycle);  // pJ
    float power = energy * static_cast<float>(FREQ);  // uW
    return power / 1000;  // Transfer to mW
}

void Profiler::tiaProfiling()
{
    // Performance
    uint totalOpNum = chanActiveNumTotal;
    float totalReqBlockRate = 0;
    float avgReqBlockRate = 0;
    uint reqNum = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Lse)
        {
            if (dynamic_cast<Lse*>(entry.chanPtr)->memReqCnt != 0)
            {
                totalReqBlockRate += float(dynamic_cast<Lse*>(entry.chanPtr)->memReqBlockCnt) / float(dynamic_cast<Lse*>(entry.chanPtr)->memReqCnt);
                reqNum++;
            }
        }
    }
    avgReqBlockRate = totalReqBlockRate / static_cast<float>(std::max(reqNum, uint(1)));

    float resII = 1.2;  // Update to Modulo mapper
    uint cycle = (totalOpNum / TIA_ARRAY_SIZE) / (1 - avgReqBlockRate) * resII;
    std::cout <<"feataetager" << totalReqBlockRate << "\t" << reqNum << std::endl;
    float utilization = (1 - avgReqBlockRate) / resII;

    debug->getFile() << std::endl;
    debug->getFile() << "******* TIA Performance profiling *********" << std::endl;
    debug->getFile() << "Total cycle: " << cycle << setprecision(2) << std::endl;
    debug->getFile() << "PE utilization: " << utilization * 100 << setprecision(2) << "%" << std::endl;

    // Power
    uint reg_active_times = chanActiveNumTotal * 3;
    uint pe_ctrl_active_times = chanActiveNumTotal;
    uint instr_buffer_active_times = chanActiveNumTotal;
    // ALU
    float alu_dynamic_energy = 0;

    //for (auto& entry : registry->getRegistryTable())
    //{
    //    if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Base)
    //    {
    //        alu_dynamic_energy += static_cast<float>(entry.chanPtr->activeCnt) * Hardware_Para::getAluDynamicEnergy();
    //    }
    //}

    alu_dynamic_energy = static_cast<float>(chanActiveNumTotal) * Hardware_Para::getAluDynamicEnergy();
    float alu_dynamic_power = transEnergy2Power(alu_dynamic_energy, cycle);
    float alu_leakage_power = Hardware_Para::getAluLeakagePower();
    float alu_power = alu_dynamic_power + alu_leakage_power;

    // RF
    float reg_dynamic_energy = static_cast<float>(reg_active_times) * Hardware_Para::getRegAccessEnergy();
    float reg_dynamic_power = transEnergy2Power(reg_dynamic_energy, cycle);
    float reg_leakage_power = Hardware_Para::getRegLeakagePower();
    float reg_power = reg_dynamic_power + reg_leakage_power;

    // PE ctrl
    float pe_ctrl_dynamic_energy = static_cast<float>(pe_ctrl_active_times) * Hardware_Para::getTiaPeCtrlEnergyDynamic();
    float pe_ctrl_dynamic_power = transEnergy2Power(pe_ctrl_dynamic_energy, cycle);
    float pe_ctrl_leakage_power = Hardware_Para::getTiaPeCtrlLeakagePower();
    float pe_ctrl_power = pe_ctrl_dynamic_power + pe_ctrl_leakage_power;

    // Instr. buffer
    float instr_buffer_dynamic_energy = static_cast<float>(instr_buffer_active_times) * Hardware_Para::getTiaInstructionBufferAccessEnergy();
    float instr_buffer_dynamic_power = transEnergy2Power(instr_buffer_dynamic_energy, cycle);
    float instr_buffer_leakage_power = Hardware_Para::getTiaInstructionBufferLeakagePower();
    float instr_buffer_power = instr_buffer_dynamic_power + instr_buffer_leakage_power;

    // On-chip buffer
    uint dataBuffer_access_times = 0;
    uint dataBuffer_mem_req_access_times = 0;
    uint dataBuffer_intermediate_data_access_times = 0;
    for (auto& entry : registry->getRegistryTable())
    {
        if (entry.moduleType == ModuleType::Channel)
        {
            if (entry.chanPtr->chanType == ChanType::Chan_Lse)
            {
                dataBuffer_mem_req_access_times += entry.chanPtr->activeCnt;
            }
            //else if (entry.chanPtr->chanType == ChanType::Chan_DGSF)
            //{
            //    dataBuffer_intermediate_data_access_times += entry.chanPtr->activeCnt;
            //}
        }
    }
    dataBuffer_access_times = dataBuffer_mem_req_access_times + dataBuffer_intermediate_data_access_times;

    // On-chip buffer ctrl
    float dataBuffer_ctrl_dynamic_energy = static_cast<float>(dataBuffer_access_times) * Hardware_Para::getDataBufferCtrlEnergy();
    float dataBuffer_ctrl_dynamic_power = transEnergy2Power(dataBuffer_ctrl_dynamic_energy, cycle);
    float dataBuffer_ctrl_leakage_power = Hardware_Para::getDataBufferCtrlLeakagePower();
    float dataBuffer_ctrl_power = dataBuffer_ctrl_dynamic_power + dataBuffer_ctrl_leakage_power;
    // On-chip buffer sram
    float dataBuffer_sram_dynamic_energy = static_cast<float>(dataBuffer_access_times) * Hardware_Para::getDataBufferAccessEnergy();
    float dataBuffer_sram_dynamic_power = transEnergy2Power(dataBuffer_sram_dynamic_energy, cycle);
    float dataBuffer_sram_leakage_power = Hardware_Para::getDataBufferLeakagePower();
    float dataBuffer_sram_power = dataBuffer_sram_dynamic_power + dataBuffer_sram_leakage_power;
    // On-chip buffer total
    float dataBuffer_dynamic_power = dataBuffer_ctrl_dynamic_power + dataBuffer_sram_dynamic_power;
    float dataBuffer_leakage_power = dataBuffer_ctrl_leakage_power + dataBuffer_sram_leakage_power;
    float dataBuffer_power = dataBuffer_dynamic_power + dataBuffer_leakage_power;

    float total_power = alu_power +
                        reg_power +
                        pe_ctrl_power +
                        instr_buffer_power +
                        dataBuffer_power;

    debug->getFile() << std::endl;
    debug->getFile() << "******* TIA Power profiling *********" << std::endl;
    debug->getFile() << ">>> TIA PE Array: " << std::endl;
    debug->getFile() << "PE active total times: " << chanActiveNumTotal << std::endl;

    debug->getFile() << "ALU power: " << alu_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << alu_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << alu_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "Reg power: " << reg_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << reg_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << reg_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "TIA PE Ctrl logic power: " << pe_ctrl_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << pe_ctrl_dynamic_power << setprecision(4) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << pe_ctrl_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << "Instruction Buffer power: " << instr_buffer_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << instr_buffer_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << instr_buffer_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> On-chip Buffer: " << std::endl;
    debug->getFile() << "Access times: " << dataBuffer_access_times << std::endl;
    debug->getFile() << "\t Mem req access times: " << dataBuffer_mem_req_access_times << std::endl;
    debug->getFile() << "\t Intermediate data access times: " << dataBuffer_intermediate_data_access_times << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "Buffer total power: " << dataBuffer_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_leakage_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "Buffer sram power: " << dataBuffer_sram_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_sram_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_sram_leakage_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "Buffer ctrl power: " << dataBuffer_ctrl_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Dynamic power: " << dataBuffer_ctrl_dynamic_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << "\t Leakage power: " << dataBuffer_ctrl_leakage_power << setprecision(2) << " mW" << std::endl;

    debug->getFile() << std::endl;
    debug->getFile() << ">>> Total power: " << total_power << setprecision(2) << " mW" << std::endl;
    debug->getFile() << ">>> EDP: " << pow(ClkDomain::getClk() / 1000, 2) * (total_power / 1000.0) << setprecision(2) << std::endl;
}