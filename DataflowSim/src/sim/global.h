#pragma once
#include "../define/Define.hpp"
#include "../module/EnumType.h"

namespace DFSim
{
    class Global 
    {
    public:
        static void load_config(const string& config_path);
        static App_name app_name;
        static string file_path;
        static ArchType arch;

        //** Execution parameters **//
        // Array para
        static uint64_t freq;
        static uint64_t array_size;
        static uint64_t tia_array_size;
        // Base
        static uint64_t base_input_buffer_size;
        // DGSF
        static uint64_t bram_bank_depth;
        static uint64_t DGSF_input_buffer_size;
        static uint64_t bram_access_delay;
        // SGMF
        static uint64_t SGMF_input_buffer_size;
        static uint64_t SGMF_tag_size;
        // Lse
        static uint64_t lse_queue_size;
        static bool lse_O3;

        //** ALU parameters **//
        // ALU delay cycle
        static uint64_t ADD;
        static uint64_t SUB;
        static uint64_t MUL;
        static uint64_t MAC;
        static uint64_t DIV;
        static uint64_t RELU;

        //** Memory parameters **//
        // MemSystem
        static bool no_memory;
        static uint64_t data_precision;
        static uint64_t bus_delay;
        static uint64_t bank_block_size;
        static uint64_t memSystem_queue_bank_num;
        static uint64_t memSystem_req_queue_size_per_bank;
        static uint64_t memSystem_ack_queue_size_per_bank;
        static bool memSys_coalescing_enable;
        static uint64_t memSys_coalescer_entry_num;
        static uint64_t memSys_coalescer_size_per_entry;

        // SPM
        static bool spm_enable;
        static uint64_t spm_access_latency;
        static uint64_t spm_bank_num;
        static uint64_t spm_bank_depth;
        static uint64_t spm_req_queue_size;

        // Cache
        static bool cache_enable;
        static bool cache_all_hit;
        static uint64_t req_queue_to_mem_size;
        static uint64_t cache_max_level;
        static uint64_t cache_size_L1;
        static uint64_t cache_size_L2;
        static uint64_t cache_line_size_L1;
        static uint64_t cache_line_size_L2;
        static uint64_t cache_mapping_way_L1;
        static uint64_t cache_mapping_way_L2;
        static uint64_t cache_access_latency_L1;
        static uint64_t cache_access_latency_L2;
        static uint64_t cache_bank_num_L1;
        static uint64_t cache_bank_num_L2;
        static uint64_t cache_req_queue_size_per_bank_L1;
        static uint64_t cache_req_queue_size_per_bank_L2;
        static uint64_t cache_ack_queue_size_per_bank_L1;
        static uint64_t cache_ack_queue_size_per_bank_L2;
        static bool cache_mshr_enable_L1;
        static bool cache_mshr_enable_L2;
        static uint64_t cache_mshr_entry_num_L1;
        static uint64_t cache_mshr_entry_num_L2;
        static uint64_t cache_mshr_size_per_entry_L1;
        static uint64_t cache_mshr_size_per_entry_L2;
    };
}