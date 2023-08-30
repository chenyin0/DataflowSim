#include "./gcn.h"
#include "../../src/sim/Watchdog.h"
#include "../../src/module/execution/GraphScheduler.h"
#include "../../src/define/hw_para.hpp"
#include "../../src/module/Profiler.h"
#include "./gnn_module/aggregator.h"
#include "./gnn_module/combinator.h"

using namespace DFSimTest;

void GCN_Test::gcn_Base_trace_systolic(Debug *debug, const string &input_file_path, const string &dataset, const string &arch_name, const string &deg_th)
{
    //******  Define module  ******//
    //*** Declare memory
    MemSystem *memSys = new MemSystem();

    //*** Declare registry
    Registry *registry = new Registry(memSys);

    //*** Declare Profiler
    Profiler *profiler = new Profiler(registry, memSys, debug);

    //*** Declare Watchdog
    Watchdog watchdog = Watchdog(pow(2, 7), 50000000);

    //*** Define subgraph scheduler
    GraphScheduler *graphScheduler = new GraphScheduler();

    // Generate DFG
    generateDfg();

    // Generate ChanGraph
    ChanGraph chanGraph(GCN_Test::dfg);
    chanGraph.addSpecialModeChan();

    uint64_t splitNum = 1;
    // chanGraph.subgraphPartitionCtrlRegion(splitNum, debug);
    GCN_Test::graphPartition(chanGraph, splitNum);

    // chanGraph.addChanDGSF();
    chanGraph.addNodeDelay();
    // chanGraph.setSpeedup(debug);  // Set speedup
    chanGraph.plotDot();

    registry->genModule(chanGraph);
    registry->genConnect(chanGraph);
    registry->setChanSize();
    registry->configGraphScheduler(graphScheduler);
    auto &regis = registry->getRegistryTable(); // For exposing registry in debugging
    // debug->printRegistry(registry);
    // debug->printSimNodes(chanGraph);
    registry->genSimConfig(chanGraph); // Only used for initializing the first time sim
    const auto &debugPrint = registry->genDebugPrint(chanGraph);
    auto simChans = std::get<0>(debugPrint);
    auto simLcs = std::get<1>(debugPrint);

    // Generate benchmark data
    GCN_Test::generateData(dataset);

    // Read mem trace
    // deque<uint64_t> nghNodeTrace;
    // deque<uint64_t> featTrace;
    // deque<uint64_t> delay_q;  // Record data delay
    // string fileName = "./resource/gcn/mem_trace/" + dataset_name + "_delta_ngh_deg_5.txt";
    // string fileName = "./resource/gcn/mem_trace/" + dataset_name + "_all_ngh.txt";
    // string fileName = "./resource/gcn/mem_trace/" + dataset_name + "_full_retrain.txt";
    // readMemTrace(nghNodeTrace, fileName);
    // readMemTrace(featTrace, fileName);

    // string fileName = "./resource/gcn/mem_trace/" + dataset_name + '_' + arch_name + ".txt";

    /* Trace format
     *  root_node_id  #ngh  ngh_node_id  #ngh-in-degree  #ngh-out-degree
     */
    vector<deque<uint64_t>> trace(5);
    // readMemTraceByCol(trace, fileName);
    uint64_t line_id = 0;
    bool file_read_complete = 0;
    uint64_t read_line_block_size = 16384;
    // uint64_t read_line_block_size = 10;
    // file_read_complete = readMemTraceByCol_blocked(trace, fileName, read_line_block_size, line_id);
    file_read_complete = readMemTraceByCol_blocked(trace, input_file_path, read_line_block_size, line_id);
    deque<uint64_t> nghNodeTrace = trace[2];
    deque<uint64_t> rootNodeTrace = trace[0];
    // deque<uint64_t> delay_q = trace[1];
    for (auto &i : trace)
    {
        i.clear();
    }

    ////*** Simulate
    // Declare
    //***********************************************************************

    uint64_t v_lse_num = 8;
    uint64_t w_lse_num = 8;
    uint64_t f_lse_num = 8;

    uint64_t v_lse_queue_size = 4;
    uint64_t f_lse_queue_size = 4;

    // Declare module
    vector<Lse *> vertex_lses;
    vertex_lses.resize(v_lse_num);
    vector<Lse *> weight_lses;
    weight_lses.resize(w_lse_num);
    vector<Lse *> feat_lses;
    feat_lses.resize(f_lse_num);

    for (auto &lse : vertex_lses)
    {
        lse = new Lse(v_lse_queue_size, 1, false, memSys, 1);
        lse->chanBuffer.resize(1);
        lse->noUpstream = 1;
        lse->noDownstream = 1;
        memSys->registerLse(lse);
    }

    // for (auto &lse : weight_lses)
    // {
    //     lse = new Lse(w_lse_queue_size, 1, false, memSys, 1);
    //     lse->noUpstream = 1;
    //     lse->noDownstream = 1;
    // }

    for (auto &lse : feat_lses)
    {
        lse = new Lse(f_lse_queue_size, 1, false, memSys, 1);
        lse->chanBuffer.resize(1);
        lse->noUpstream = 1;
        lse->noDownstream = 1;
        memSys->registerLse(lse);
    }

    vector<std::pair<uint64_t, deque<uint64_t>>> feat_trace_port; // pair<root_node, ngh_node>
    feat_trace_port.resize(f_lse_num);
    vector<Bool> feat_lse_occupied(f_lse_num, false);
    vector<int> feat_trace_port_lse_id(f_lse_num, -1); // Init '-1' represents an invalid Id

    uint64_t aggr_pe_num = 8;
    uint64_t aggr_pe_cycle = 2;
    uint64_t comb_pe_num = 8;
    uint64_t comb_pe_buffer_size = 4;
    uint64_t comb_pe_cycle = 4;

    Aggregator aggregator(aggr_pe_num, aggr_pe_cycle);
    Combinator combinator(comb_pe_num, comb_pe_buffer_size, comb_pe_cycle);

    // Declare interconnect

    //***********************************************************************

    // // User defined
    uint64_t iter = 0;
    uint64_t percent = 0;

    //*** Record run time
    clock_t startTime, endTime;
    startTime = clock();

    // // Usr define
    uint64_t line_id_prev = 0;
    bool exe_over = false;

    // Execute
    while (1)
    {
        watchdog.feedDog(iter);

        //// Print progress bar
        // if (iter / segment > percent)
        //{
        //     percent = iter / segment;
        //     std::cout << ">>>>>> Progress: " << percent << "%" << "\t Iter: " << iter << std::endl;
        // }

        //// Print progress bar
        // uint64_t workload_finish = workload_size - featTrace.size();
        // if (workload_finish / interval > percent)
        //{
        //     percent = workload_finish / interval;
        //     std::cout << ">>>>>> Progress: " << percent << "%" << "\t Iter: " << iter << std::endl;
        // }

        if (line_id_prev != line_id)
        {
            std::cout << ">>>>>> Process line id: " << line_id << std::endl;
            line_id_prev = line_id;
        }

        DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
        int clk = DFSim::ClkDomain::getInstance()->getClk();
        debug->getFile() << "\n"
                         << "**************** "
                         << "Exe:" << iter << "  ";
        debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

        // Below is user-defined sim
        // Do not forget invalid Chan_begin after the outer-most Lc
        // *************************************************************************************

        // Load trace to vecPort
        for (auto i = 0; i < feat_trace_port.size(); ++i)
        {
            auto &port = feat_trace_port[i];
            if (port.second.empty())
            {
                port.first = rootNodeTrace.front();
                rootNodeTrace.pop_front();
                port.second.emplace_back(nghNodeTrace.front());
                nghNodeTrace.pop_front();

                for (auto iter = rootNodeTrace.begin(); iter != rootNodeTrace.end();)
                {
                    if (*iter == port.first)
                    {
                        port.second.emplace_back(nghNodeTrace.front());
                        iter = rootNodeTrace.erase(iter);
                        nghNodeTrace.pop_front();
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        // std::cout << "RootSize: " << rootNodeTrace.size() << " NghSize: " << nghNodeTrace.size() << std::endl;

        // Lse <-> vecPort
        for (auto portId = 0; portId < feat_trace_port_lse_id.size(); ++portId)
        {
            if (feat_trace_port_lse_id[portId] == -1 && !feat_trace_port[portId].second.empty())
            {
                for (auto lseId = 0; lseId < feat_lse_occupied.size(); ++lseId)
                {
                    if (!feat_lse_occupied[lseId])
                    {
                        feat_lse_occupied[lseId] = true;
                        feat_trace_port_lse_id[portId] = lseId;
                        break;
                    }
                }
            }

            // When finish loading a vecPort
            if (feat_trace_port[portId].second.empty())
            {
                int lseId = feat_trace_port_lse_id[portId];
                feat_lse_occupied[lseId] = false;
                feat_trace_port_lse_id[portId] = -1;
            }

            // Send to Lse
            if (feat_trace_port_lse_id[portId] != -1)
            {
                uint64_t featId = feat_trace_port[portId].second.front();
                uint64_t addr = featId * feat_length + feat_base_addr;
                std::cout << "Addr: " << addr << std::endl;
                auto lseId = feat_trace_port_lse_id[portId];
                Data data;
                data.valid = 1;
                data.value = addr;
                data.tag = feat_trace_port[portId].first;
                if (Send2Lse(*feat_lses[lseId], data))
                {
                    feat_trace_port[portId].second.pop_front();
                }
            }
        }

        // Lse
        UpdateLse(feat_lses, aggregator.in_port_);

        aggregator.Update();
        arbiter(aggregator.out_port_, combinator.in_port_);
        combinator.Update();
        for (auto &port : combinator.out_port_)
        {
            port.valid = 0;
        }

        std::cout << "Aggr " << "Push: " << aggregator.push_data_cnt_ << " Pop: " << aggregator.pop_data_cnt_ << std::endl;
        std::cout << "Comb " << "Push: " << combinator.push_data_cnt_ << " Pop: " << combinator.pop_data_cnt_ << std::endl;

        // *************************************************************************************

        // Read file blocked
        if (rootNodeTrace.empty())
        {
            uint64_t i = 0;
        }
        if (rootNodeTrace.empty() && !file_read_complete)
        {
            file_read_complete = readMemTraceByCol_blocked(trace, input_file_path, read_line_block_size, line_id);
            nghNodeTrace.insert(nghNodeTrace.end(), trace[2].begin(), trace[2].end());
            rootNodeTrace.insert(rootNodeTrace.end(), trace[0].begin(), trace[0].end());
            // delay_q.insert(delay_q.end(), trace[1].begin(), trace[1].end());
            for (auto &i : trace)
            {
                i.clear();
            }
            // for (auto it = nghNodeTrace.begin(); it != nghNodeTrace.end(); ++it)
            // {
            //     /*for (auto i = 0; i < feat_length; ++i)
            //     {
            //         featTrace.emplace_back(*it + i);
            //     }*/
            //     featTrace.emplace_back(*it * feat_length);
            // }
            // nghNodeTrace.clear();
        }

        // std::cout << line_id << ' ' << nghNodeTrace.size() << ' ' << featTrace.size() << std::endl;

        ////** Update each chanDGSF
        // registry->updateChanDGSF();

        //** MemorySystem update
        memSys->MemSystemUpdate();

        //** Profiler update
        // profiler->updateBufferMaxDataNum();
        // profiler->updateChanUtilization(graphScheduler->currSubgraphId);

        // /*end->get();*/
        // Chan_end->get();	// Nop

        /*if (nghNodeTrace.empty() && featTrace.empty())
        {
            Chan_end->channel.emplace_back(Data());
        }*/

        // Execute finish
        if (rootNodeTrace.empty())
        {
            exe_over = true;
        }

        //** Print log
        // Set debug mode
        // debug->debug_mode = Debug_mode::Print_detail;
        debug->debug_mode = Debug_mode::Turn_off;

        if (13056 > iter && iter > 0 /*iter >= 0*/)
        {
            // // Print channel
            // debug->printSimInfo(simChans, simLcs);
            // debug->printGraphScheduler(graphScheduler);

            // Print MemorySystem
            debug->memSysPrint(memSys);
        }

        if (exe_over)
        {
            debug->debug_mode = Debug_mode::Print_detail;
            std::cout << std::endl;
            std::cout << "Arch: " << xstr(ARCH) << std::endl;
            std::cout << "*******************************" << std::endl;
            std::cout << "Execution finished succussfully" << std::endl;
            std::cout << "*******************************" << std::endl;
            std::cout << "Total Cycle: " << clk << std::endl;
            std::cout << "Execution Iter: " << iter << std::endl;

            debug->getFile() << std::endl;
            debug->getFile() << "*******************************" << std::endl;
            debug->getFile() << "Execution finished succussfully" << std::endl;
            debug->getFile() << "*******************************" << std::endl;
            debug->getFile() << "Arch: " << arch_name << std::endl;
            debug->getFile() << "Dataset: " << dataset << std::endl;
            debug->getFile() << "Deg_th: " << deg_th << std::endl;
            debug->getFile() << std::endl;
            debug->getFile() << "Total Cycle: " << clk << std::endl;
            debug->getFile() << "Execution Iter: " << iter << std::endl;

            break;
        }

        ++iter;
    }

    debug->debug_mode = Debug_mode::Print_detail;
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Profiling" << std::endl;
    debug->getFile() << "*******************************" << std::endl;

    // // Print channel utilization
    // debug->getFile() << std::endl;
    // debug->getFile() << "*******************************" << std::endl;
    // debug->getFile() << "Channel profiling: " << std::endl;
    // debug->getFile() << std::endl;
    // profiler->printChanProfiling(graphScheduler);

    //*** Print Lse access
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Lse profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printLseProfiling();

    //*** Print cache
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Cache miss rate: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printCacheProfiling();

    //*** Print DRAM
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "DRAM profiling: " << std::endl;
    debug->getFile() << std::endl;
    profiler->printDramProfiling();

    //*** Print power
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Power profiling " << std::endl;
    debug->getFile() << std::endl;
    // profiler->printPowerProfiling();

    // // Report power
    // std::cout.setf(std::ios::fixed, std::ios::floatfield);
    // // PE array
    // //uint64_t aluActiveTimes = chanActiveNumTotal;
    // uint64_t chanActiveNumTotal = Chan_systolic->activeCnt * systolic_array_length;
    // uint64_t reg_active_times = chanActiveNumTotal * 3;
    // uint64_t pe_ctrl_active_times = chanActiveNumTotal;

    // float alu_dynamic_energy = 0;
    // //for (auto& entry : registry->getRegistryTable())
    // //{
    // //    if (entry.moduleType == ModuleType::Channel && entry.chanPtr->chanType == ChanType::Chan_Base)
    // //    {
    // //        alu_dynamic_energy += static_cast<float>(entry.chanPtr->activeCnt) * Hardware_Para::getAluDynamicEnergy();
    // //    }
    // //}
    // alu_dynamic_energy = chanActiveNumTotal * Hardware_Para::getAluDynamicEnergy();
    // float alu_dynamic_power = Profiler::transEnergy2Power(alu_dynamic_energy);
    // float alu_leakage_power = Hardware_Para::getAluLeakagePower() * systolic_array_width * systolic_array_length;
    // float alu_power = alu_dynamic_power + alu_leakage_power;

    // //float reg_dynamic_energy = static_cast<float>(reg_active_times) * Hardware_Para::getRegAccessEnergy();
    // //float reg_dynamic_power = Profiler::transEnergy2Power(reg_dynamic_energy);
    // //float reg_leakage_power = Hardware_Para::getRegLeakagePower();
    // //float reg_power = reg_dynamic_power + reg_leakage_power;

    // float pe_ctrl_dynamic_energy = static_cast<float>(pe_ctrl_active_times) * Hardware_Para::getPeCtrlEnergyDynamic();
    // float pe_ctrl_dynamic_power = Profiler::transEnergy2Power(pe_ctrl_dynamic_energy);
    // float pe_ctrl_leakage_power = Hardware_Para::getPeCtrlLeakagePower();
    // float pe_ctrl_power = pe_ctrl_dynamic_power + pe_ctrl_leakage_power;

    // uint64_t reconfig_times = Chan_systolic->activeCnt / systolic_array_width;
    // float reconifg_dynamic_energy = static_cast<float>(reconfig_times) * Hardware_Para::getContextBufferAccessEnergy();
    // float reconifg_dynamic_power = Profiler::transEnergy2Power(reconifg_dynamic_energy);
    // float reconifg_leakage_power = Hardware_Para::getContextBufferLeakagePower();
    // float reconifg_power = reconifg_dynamic_power + reconifg_leakage_power;

    // // On-chip buffer (weight buffer)
    // buffer_access_cnt *= feat_length;
    // if (arch_name != "awb-gcn" && arch_name != "delta_gnn_opt")
    // {
    //     buffer_access_cnt = buffer_access_cnt / systolic_array_width;
    // }
    // uint64_t coalesceRate = uint64_t(16);
    // buffer_access_cnt = uint64_t(buffer_access_cnt / coalesceRate);
    // // On-chip buffer ctrl
    // float dataBuffer_ctrl_dynamic_energy = static_cast<float>(buffer_access_cnt) * Hardware_Para::getDataBufferCtrlEnergy();
    // float dataBuffer_ctrl_dynamic_power = Profiler::transEnergy2Power(dataBuffer_ctrl_dynamic_energy);
    // float dataBuffer_ctrl_leakage_power = Hardware_Para::getDataBufferCtrlLeakagePower();
    // float dataBuffer_ctrl_power = dataBuffer_ctrl_dynamic_power + dataBuffer_ctrl_leakage_power;
    // // On-chip buffer sram
    // float dataBuffer_sram_dynamic_energy = static_cast<float>(buffer_access_cnt) * Hardware_Para::getDataBufferAccessEnergy();
    // float dataBuffer_sram_dynamic_power = Profiler::transEnergy2Power(dataBuffer_sram_dynamic_energy);
    // float dataBuffer_sram_leakage_power = Hardware_Para::getDataBufferLeakagePower();
    // float dataBuffer_sram_power = dataBuffer_sram_dynamic_power + dataBuffer_sram_leakage_power;
    // // On-chip buffer total
    // float dataBuffer_dynamic_power = dataBuffer_ctrl_dynamic_power + dataBuffer_sram_dynamic_power;
    // float dataBuffer_leakage_power = dataBuffer_ctrl_leakage_power + dataBuffer_sram_leakage_power;
    // float dataBuffer_power = dataBuffer_dynamic_power + dataBuffer_leakage_power;

    // // Cache
    // uint64_t cache_access_times = 0;
    // if (memSys != nullptr && memSys->cache != nullptr)
    // {
    //     cache_access_times += memSys->cache->getCacheAccessCnt();
    // }
    // float cache_dynamic_energy = static_cast<float>(cache_access_times) * Hardware_Para::getCacheAccessEnergy();
    // float cache_dynamic_power = Profiler::transEnergy2Power(cache_dynamic_energy);
    // float cache_leakage_power = Hardware_Para::getCacheLeakagePower();
    // float cache_power = cache_dynamic_power + cache_leakage_power;

    // // DRAM
    // uint64_t mem_access_times = 0;
    // if (memSys != nullptr)
    // {
    //     mem_access_times += memSys->getMemAccessCnt();
    // }
    // float mem_access_energy = mem_access_times * Hardware_Para::getDramAccessEnergy();
    // float mem_access_power = Profiler::transEnergy2Power(mem_access_energy);

    // float total_power = alu_power +
    //     //reg_power +
    //     pe_ctrl_power +
    //     reconifg_power +
    //     dataBuffer_power +
    //     cache_power +
    //     mem_access_power;

    // debug->getFile() << std::endl;
    // debug->getFile() << "******* Power profiling *********" << std::endl;
    // debug->getFile() << ">>> PE Array: " << std::endl;
    // debug->getFile() << "PE active total times: " << chanActiveNumTotal << std::endl;

    // debug->getFile() << "ALU power: " << std::setprecision(2) << alu_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << alu_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << alu_leakage_power << " mW" << std::endl;

    // //debug->getFile() << "Reg power: " << std::setprecision(2) << reg_power << " mW" << std::endl;
    // //debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << reg_dynamic_power << " mW" << std::endl;
    // //debug->getFile() << "\t Leakage power: " << std::setprecision(4) << reg_leakage_power << " mW" << std::endl;

    // debug->getFile() << "Ctrl logic power: " << std::setprecision(2) << pe_ctrl_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << pe_ctrl_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << pe_ctrl_leakage_power << " mW" << std::endl;

    // debug->getFile() << "Reconfig power: " << std::setprecision(2) << reconifg_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << reconifg_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << reconifg_leakage_power << " mW" << std::endl;

    // debug->getFile() << std::endl;
    // debug->getFile() << ">>> On-chip Buffer: " << std::endl;
    // debug->getFile() << "Access times: " << buffer_access_cnt << std::endl;
    // debug->getFile() << "Buffer total power: " << std::setprecision(2) << dataBuffer_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << dataBuffer_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << dataBuffer_leakage_power << " mW" << std::endl;
    // debug->getFile() << "Buffer ctrl power: " << std::setprecision(2) << dataBuffer_ctrl_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << dataBuffer_ctrl_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << dataBuffer_ctrl_leakage_power << " mW" << std::endl;
    // debug->getFile() << "Buffer sram power: " << std::setprecision(2) << dataBuffer_sram_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << dataBuffer_sram_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << dataBuffer_sram_leakage_power << " mW" << std::endl;

    // debug->getFile() << std::endl;
    // debug->getFile() << ">>> Cache: " << std::endl;
    // debug->getFile() << "Cache access times: " << cache_access_times << std::endl;
    // debug->getFile() << "Cache power: " << std::setprecision(2) << cache_power << " mW" << std::endl;
    // debug->getFile() << "\t Dynamic power: " << std::setprecision(2) << cache_dynamic_power << " mW" << std::endl;
    // debug->getFile() << "\t Leakage power: " << std::setprecision(4) << cache_leakage_power << " mW" << std::endl;

    // debug->getFile() << std::endl;
    // debug->getFile() << ">>> DRAM: " << std::endl;
    // debug->getFile() << "Access times: " << mem_access_times << std::endl;
    // debug->getFile() << "Power: " << std::setprecision(2) << mem_access_power << " mW" << std::endl;

    // debug->getFile() << std::endl;
    // debug->getFile() << ">>> Total power: " << std::setprecision(2) << total_power << " mW" << std::endl;
    // debug->getFile() << ">>> EDP: " << std::setprecision(2) << pow(ClkDomain::getClk() / 1000, 2) * (total_power / 1000.0) << std::endl;

    // // Print
    // std::cout << std::endl;
    // std::cout << "Total power: " << std::setprecision(2) << total_power << " mW" << std::endl;
    // std::cout << std::endl;
    // std::cout << "Buffer access times: " << buffer_access_cnt << std::endl;
    // std::cout << "Cache access times: " << cache_access_times << std::endl;
    // std::cout << "DRAM access times: " << mem_access_times << std::endl;
    // std::cout << "EDP: " << std::setprecision(2) << pow(ClkDomain::getClk() / 1000, 2) * (total_power / 1000.0) << std::endl;

    // std::cout << std::endl;
    // std::cout << "ALU power: " << std::setprecision(2) << alu_power << " mW" << std::endl;
    // //std::cout << "Reg power: " << std::setprecision(2) << reg_power << " mW" << std::endl;
    // std::cout << "PE-ctrl power: " << std::setprecision(2) << pe_ctrl_power << " mW" << std::endl;
    // std::cout << "Reconfig power: " << std::setprecision(2) << reconifg_power << " mW" << std::endl;
    // std::cout << "Buffer power: " << std::setprecision(2) << dataBuffer_power << " mW" << std::endl;
    // std::cout << "Buffer dynamic power: " << std::setprecision(2) << dataBuffer_dynamic_power << " mW" << std::endl;
    // std::cout << "Cache power: " << std::setprecision(2) << cache_power << " mW" << std::endl;
    // std::cout << "Cache dynamic power: " << std::setprecision(2) << cache_dynamic_power << " mW" << std::endl;
    // std::cout << "DRAM power: " << std::setprecision(2) << mem_access_power << " mW" << std::endl;
    // std::cout << std::endl;

    //*** Record run time
    endTime = clock();
    std::cout << "Total run time: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "*******************************" << std::endl;
    debug->getFile() << "Total run time: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;

    delete registry; // All the Module pointers have been deleted when destruct registry
    delete profiler;
    delete graphScheduler;
    delete memSys;
}
