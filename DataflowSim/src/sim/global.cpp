#include "./global.h"
#include "../define/Para.h"
#include "./Debug.h"
#include "../../lib/jsoncpp/include/json/json.h"

using namespace DFSim;

/* App name list
SimpleFlowTest
MemoryTest
Gemm
Bfs
*/
App_name Global::app_name = App_name::GCN;
#ifdef WINDOWS
string Global::file_path = "./resource/" + App_name_convert::toString(app_name) + "/";
#endif
#ifdef LINUX
string Global::file_path = "./DataflowSim/resource/" + App_name_convert::toString(app_name) + "/";
#endif

//** Define Arch
#ifdef Base
ArchType Global::arch = ArchType::Base;
#endif
#ifdef DGSF
ArchType Global::arch = ArchType::DGSF;
#endif
#ifdef SGMF
ArchType Global::arch = ArchType::SGMF;
#endif
//**

uint64_t Global::freq = 0;
uint64_t Global::array_size;
uint64_t Global::tia_array_size;

uint64_t Global::base_input_buffer_size;

uint64_t Global::bram_bank_depth;
uint64_t Global::DGSF_input_buffer_size;
uint64_t Global::bram_access_delay;

uint64_t Global::SGMF_input_buffer_size;
uint64_t Global::SGMF_tag_size;

uint64_t Global::lse_queue_size;
bool Global::lse_O3;

uint64_t Global::ADD;
uint64_t Global::SUB;
uint64_t Global::MUL;
uint64_t Global::MAC;
uint64_t Global::DIV;
uint64_t Global::RELU;

bool Global::no_memory;
uint64_t Global::data_precision;
uint64_t Global::bus_delay;
uint64_t Global::bank_block_size;

uint64_t Global::memSystem_queue_bank_num;
uint64_t Global::memSystem_req_queue_size_per_bank;
uint64_t Global::memSystem_ack_queue_size_per_bank;

bool Global::memSys_coalescing_enable;
uint64_t Global::Global::memSys_coalescer_entry_num;
uint64_t Global::memSys_coalescer_size_per_entry;

bool Global::spm_enable;
uint64_t Global::spm_access_latency;
uint64_t Global::spm_bank_num;
uint64_t Global::spm_bank_depth;
uint64_t Global::spm_req_queue_size;

bool Global::cache_enable;
bool Global::cache_all_hit;
uint64_t Global::req_queue_to_mem_size;

uint64_t Global::cache_max_level;
uint64_t Global::cache_size_L1;
uint64_t Global::cache_size_L2;

uint64_t Global::cache_line_size_L1;
uint64_t Global::cache_line_size_L2;

uint64_t Global::cache_mapping_way_L1;
uint64_t Global::cache_mapping_way_L2;
uint64_t Global::cache_access_latency_L1;
uint64_t Global::cache_access_latency_L2;
uint64_t Global::cache_bank_num_L1;
uint64_t Global::cache_bank_num_L2;
uint64_t Global::cache_req_queue_size_per_bank_L1;
uint64_t Global::cache_req_queue_size_per_bank_L2;
uint64_t Global::cache_ack_queue_size_per_bank_L1;
uint64_t Global::cache_ack_queue_size_per_bank_L2;
bool Global::cache_mshr_enable_L1;
bool Global::cache_mshr_enable_L2;
uint64_t Global::cache_mshr_entry_num_L1;
uint64_t Global::cache_mshr_entry_num_L2;
uint64_t Global::cache_mshr_size_per_entry_L1;
uint64_t Global::cache_mshr_size_per_entry_L2;

void Global::load_config(const string &config_path)
{
    std::cout << ">> Load Config ..." << std::endl;
    std::ifstream ifs(config_path);
    if (ifs.is_open())
    {
        Json::Reader jsonReader;
        Json::Value root;
        if (jsonReader.parse(ifs, root))
        {
            freq = root["freq"].asUInt64();
            array_size = root["array_size"].asUInt64();
            tia_array_size = root["tia_array_size"].asUInt64();

            base_input_buffer_size = root["base_input_buffer_size"].asUInt64();

            bram_bank_depth = root["bram_bank_depth"].asUInt64();
            DGSF_input_buffer_size = root["DGSF_input_buffer_size"].asUInt64();
            bram_access_delay = root["bram_access_delay"].asUInt64();

            SGMF_input_buffer_size = root["SGMF_input_buffer_size"].asUInt64();
            SGMF_tag_size = SGMF_input_buffer_size;

            lse_queue_size = root["lse_queue_size"].asUInt64();
            lse_O3 = root["lse_O3"].asBool();

            ADD = root["ADD"].asUInt64();
            SUB = root["SUB"].asUInt64();
            MUL = root["MUL"].asUInt64();
            MAC = root["MAC"].asUInt64();
            DIV = root["DIV"].asUInt64();
            RELU = root["RELU"].asUInt64();

            no_memory = root["no_memory"].asBool();
            data_precision = root["data_precision"].asUInt64();
            bus_delay = root["bus_delay"].asUInt64();
            bank_block_size = root["bank_block_size"].asUInt64();

            memSystem_queue_bank_num = root["memSystem_queue_bank_num"].asUInt64();
            memSystem_req_queue_size_per_bank = root["memSystem_req_queue_size_per_bank"].asUInt64();
            memSystem_ack_queue_size_per_bank = root["memSystem_ack_queue_size_per_bank"].asUInt64();

            memSys_coalescing_enable = root["memSys_coalescing_enable"].asBool();
            Global::memSys_coalescer_entry_num = root["Global::memSys_coalescer_entry_num"].asUInt64();
            memSys_coalescer_size_per_entry = root["memSys_coalescer_size_per_entry"].asUInt64();

            spm_enable = root["spm_enable"].asBool();
            spm_access_latency = root["spm_access_latency"].asUInt64();
            spm_bank_num = root["spm_bank_num"].asUInt64();
            spm_bank_depth = root["spm_bank_depth"].asUInt64();
            spm_req_queue_size = root["spm_req_queue_size"].asUInt64();

            cache_enable = root["cache_enable"].asBool();
            cache_all_hit = root["cache_all_hit"].asBool();
            req_queue_to_mem_size = root["req_queue_to_mem_size"].asUInt64();

            cache_max_level = root["cache_max_level"].asUInt64();
            if (root.isMember("cache_size_L1_KB"))
                cache_size_L1 = root["cache_size_L1_KB"].asUInt64() * 1024;
            else if (root.isMember("cache_size_L1_MB"))
                cache_size_L1 = root["cache_size_L1_MB"].asUInt64() * 1024 * 1024;

            if (root.isMember("cache_size_L2_KB"))
                cache_size_L2 = root["cache_size_L2_KB"].asUInt64() * 1024;
            else if (root.isMember("cache_size_L2_MB"))
                cache_size_L2 = root["cache_size_L2_MB"].asUInt64() * 1024 * 1024;

            cache_line_size_L1 = root["cache_line_size_L1"].asUInt64();
            cache_line_size_L2 = root["cache_line_size_L2"].asUInt64();

            cache_mapping_way_L1 = root["cache_mapping_way_L1"].asUInt64();
            cache_mapping_way_L2 = root["cache_mapping_way_L2"].asUInt64();
            cache_access_latency_L1 = root["cache_access_latency_L1"].asUInt64();
            cache_access_latency_L2 = root["cache_access_latency_L2"].asUInt64();
            cache_bank_num_L1 = root["cache_bank_num_L1"].asUInt64();
            cache_bank_num_L2 = root["cache_bank_num_L2"].asUInt64();
            cache_req_queue_size_per_bank_L1 = root["cache_req_queue_size_per_bank_L1"].asUInt64();
            cache_req_queue_size_per_bank_L2 = root["cache_req_queue_size_per_bank_L2"].asUInt64();
            cache_ack_queue_size_per_bank_L1 = root["cache_ack_queue_size_per_bank_L1"].asUInt64();
            cache_ack_queue_size_per_bank_L2 = root["cache_ack_queue_size_per_bank_L2"].asUInt64();
            cache_mshr_enable_L1 = root["cache_mshr_enable_L1"].asBool();
            cache_mshr_enable_L2 = root["cache_mshr_enable_L2"].asBool();
            cache_mshr_entry_num_L1 = root["cache_mshr_entry_num_L1"].asUInt64();
            cache_mshr_entry_num_L2 = root["cache_mshr_entry_num_L2"].asUInt64();
            cache_mshr_size_per_entry_L1 = root["cache_mshr_size_per_entry_L1"].asUInt64();
            cache_mshr_size_per_entry_L2 = root["cache_mshr_size_per_entry_L2"].asUInt64();
        }
    }
    else
    {
        Debug::throwError("Load config failed!", __FILE__, __LINE__);
    }
}