#include "../src/define/Define.hpp"
#include "../src/define/Para.h"
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"
#include "./sim/global.h"

#include "../test/simpleflow/SimpleFlow_Base.cpp"
#include "../test/simpleflow/SimpleFlow_DGSF.cpp"
#include "../test/simpleflow/SimpleFlow_SGMF.cpp"

#include "../test/memory_test/MemoryTest.h"
#include "../test/gemm/Gemm.h"
#include "../test/bfs/Bfs.h"
#include "../test/aes/Aes.h"
#include "../test/lud/Lud.h"
#include "../test/fft/fft.h"
#include "../test/viterbi/viterbi.h"
#include "../test/cfd/cfd.h"
#include "../test/gesummv/gesummv.h"
#include "../test/sort/sort.h"
#include "../test/ge/ge.h"
#include "../test/cholesky/cholesky.h"
#include "../test/hotspot/hotspot.h"
#include "../test/gcn/gcn.h"

// #include "./module/execution/Channel.h"
#include "string.h"

using namespace DFSim;

int parseOptions(int argc, char **argv, string &dataset, string &arch_name, string &deg_th)
{
    int i = 0;
    if (argc == 1)
    {
        return -1;
    }
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--dataset") == 0)
        {
            dataset = string(argv[++i]);
        }
        else if (strcmp(argv[i], "--arch") == 0)
        {
            arch_name = string(argv[++i]);
        }
        else if (strcmp(argv[i], "--deg_th") == 0)
        {
            deg_th = string(argv[++i]);
        }
        else
            return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    DFSim::ClkDomain clk();
    std::cout << "Begin test" << std::endl;

    string config_file, dataset, arch_name, deg_th;
    /*if (-1 == parseOptions(argc, argv, dataset, arch_name, deg_th))
    {
        printf("Get parameter failed!\n");
        exit(-1);
    }*/
#ifdef WINDOWS
    config_file = "./config/config.json";
#endif
#ifdef LINUX
    config_file = "./DataflowSim/config/config.json";
#endif
    dataset = "citeseer";
    arch_name = "hygcn";

    string input_file_path;
    string log_file_path;

    if (arch_name != "delta-gnn" && arch_name != "delta-gnn-opt")
    {
        input_file_path = Global::file_path + "mem_trace/" + dataset + "_" + arch_name + ".txt";
        log_file_path = Global::file_path + "results/" + dataset + "_" + arch_name + ".txt";
    }
    else
    {
        input_file_path = Global::file_path + "mem_trace/" + dataset + "_" + arch_name + "_" + deg_th + ".txt";
        log_file_path = Global::file_path + "results/" + dataset + "_" + arch_name + "_" + deg_th + ".txt";
    }

    // DFSim::Debug* debug = new DFSim::Debug(Global::file_path + App_name_convert::toString(Global::app_name) + string("_log_") + string(xstr(ARCH)) + string(".txt"));
    DFSim::Debug *debug = new DFSim::Debug(log_file_path);

    Global::load_config(config_file);

#ifdef ARCH
    debug->getFile() << std::endl;
    debug->getFile() << "******  System Config  *******" << std::endl;
    debug->getFile() << "Arch: " << xstr(ARCH) << std::endl;
    debug->getFile() << "Benchmark: " << App_name_convert::toString(Global::app_name) << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "******************************" << std::endl;
#endif

    switch (Global::app_name)
    {
    case App_name::SimpleFlowTest:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // DFSimTest::simpleFlow_Base(debug);
            break;
        case ArchType::DGSF:
            // DFSimTest::simpleFlow_DGSF(debug);
            break;
        case ArchType::SGMF:
            // DFSimTest::simpleFlow_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::MemoryTest:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::DGSF:
            // DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::SGMF:
            // DFSimTest::MemoryTest::memory_test(debug);
            break;
        }

        break;
    }
    case App_name::Gemm:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // Debug_yin_05.05
            DFSimTest::GemmTest::gemm_Base_auto_sim(debug);
            // DFSimTest::GemmTest::gemm_Base(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::GemmTest::gemm_DGSF(debug);
            break;
        case ArchType::SGMF:
            // DFSimTest::MemoryTest::memory_test(debug);
            break;
        }

        break;
    }
    case App_name::Bfs:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // DFSimTest::GemmTest::gemm_base(debug);
            break;
        case ArchType::DGSF:
            // DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::SGMF:
            DFSimTest::BfsTest::bfs_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::Aes:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::AesTest::aes_Base(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::AesTest::aes_DGSF(debug);
            break;
        case ArchType::SGMF:
            // DFSimTest::AesTest::bfs_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::Lud:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // DFSimTest::LudTest::lud_Base(debug);
            DFSimTest::LudTest::lud_Base_auto(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::LudTest::lud_DGSF(debug);
            break;
        case ArchType::SGMF:
            // DFSimTest::LudTest::lud_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::FFT:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::FFT_Test::fft_Base(debug);
            break;
            // case ArchType::DGSF:
            //     DFSimTest::AesTest::aes_DGSF(debug);
            //     break;
            // case ArchType::SGMF:
            //     //DFSimTest::AesTest::bfs_SGMF(debug);
            //     break;
        }

        break;
    }
    case App_name::Viterbi:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Viterbi_Test::viterbi_Base(debug);
            break;
            // case ArchType::DGSF:
            //     DFSimTest::AesTest::aes_DGSF(debug);
            //     break;
            // case ArchType::SGMF:
            //     //DFSimTest::AesTest::bfs_SGMF(debug);
            //     break;
        }

        break;
    }
    case App_name::Cfd:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Cfd_Test::cfd_Base(debug);
            break;
        }

        break;
    }
    case App_name::Gesummv:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Gesummv_Test::gesummv_Base(debug);
            break;
        }

        break;
    }
    case App_name::Sort:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Sort_Test::sort_Base(debug);
            break;
        }

        break;
    }
    case App_name::Ge:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Ge_Test::ge_Base(debug);
            break;
        }

        break;
    }
    case App_name::Cholesky:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::Cholesky_Test::cholesky_Base(debug);
            break;
        }

        break;
    }
    case App_name::HotSpot:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            DFSimTest::HotSpot_Test::hotSpot_Base(debug);
            break;
        }

        break;
    }
    case App_name::GCN:
    {
        switch (Global::arch)
        {
        case ArchType::Base:
            // DFSimTest::GCN_Test::gcn_Base(debug);
            // DFSimTest::GCN_Test::gcn_Base_trace(debug);
            DFSimTest::GCN_Test::gcn_Base_trace_systolic(debug, input_file_path, dataset, arch_name, deg_th);
            break;
        }

        break;
    }
    default:
        std::cout << "Not selected a simulation function" << std::endl;
    }

    // #ifdef DGSF
    //     DFSimTest::simpleFlow_DGSF(debug);
    // #endif
    // #ifdef Base
    //     DFSimTest::simpleFlow_Base(debug);
    // #endif
    // #ifdef SGMF
    //     DFSimTest::simpleFlow_SGMF(debug);
    // #endif

    // DFSimTest::MemoryTest::memory_test(debug);

    delete debug;
    return 0;
}