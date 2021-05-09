#include "../src/define/Define.hpp"
#include "../src/define/Para.h"
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"

#include "../test/simpleflow/SimpleFlow_Base.cpp"
#include "../test/simpleflow/SimpleFlow_DGSF.cpp"
#include "../test/simpleflow/SimpleFlow_SGMF.cpp"

#include "../test/memory_test/MemoryTest.h"
#include "../test/gemm/Gemm.h"
#include "../test/bfs/Bfs.h"
#include "../test/aes/Aes.h"
#include "../test/lud/Lud.h"

#include "./module/execution/Channel.h"

using namespace DFSim;

int main()
{
    DFSim::ClkDomain clk();
    std::cout << "Begin test" << std::endl;

    /* App name list
    SimpleFlowTest
    MemoryTest
    Gemm
    Bfs
    */
    static App_name g_tb_name = App_name::Gemm;
    static string g_filePath = "./resource/" + tb_name_convert::toString(g_tb_name) + "/";

    //** Define Arch
#ifdef Base
    ArchType arch = ArchType::Base;
#endif
#ifdef DGSF
    ArchType arch = ArchType::DGSF;
#endif
#ifdef SGMF
    ArchType arch = ArchType::SGMF;
#endif
    //**

    DFSim::Debug* debug = new DFSim::Debug(string("./resource/") + tb_name_convert::toString(g_tb_name) + string("/") + tb_name_convert::toString(g_tb_name) + string("_log_") + string(xstr(ARCH)) + string(".txt"));

    //DFSim::Debug* debug = new DFSim::Debug(string("./resource/output/MemoryTest/memory_test.txt"));

#ifdef ARCH
    debug->getFile() << std::endl;
    debug->getFile() << "******  System Config  *******" << std::endl;
    debug->getFile() << "Arch: " << xstr(ARCH) << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "******************************" << std::endl;
#endif

    switch (g_tb_name)
    {
    case App_name::SimpleFlowTest:
    {
        switch (arch)
        {
        case ArchType::Base:
            //DFSimTest::simpleFlow_Base(debug);
            break;
        case ArchType::DGSF:
            //DFSimTest::simpleFlow_DGSF(debug);
            break;
        case ArchType::SGMF:
            //DFSimTest::simpleFlow_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::MemoryTest:
    {
        switch (arch)
        {
        case ArchType::Base:
            //DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::DGSF:
            //DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::SGMF:
            //DFSimTest::MemoryTest::memory_test(debug);
            break;
        }

        break;
    }
    case App_name::Gemm:
    {
        switch (arch)
        {
        case ArchType::Base:
            // Debug_yin_05.05
            DFSimTest::GemmTest::gemm_Base_auto_sim(debug);
            //DFSimTest::GemmTest::gemm_Base(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::GemmTest::gemm_DGSF(debug);
            break;
        case ArchType::SGMF:
            //DFSimTest::MemoryTest::memory_test(debug);
            break;
        }

        break;
    }
    case App_name::Bfs:
    {
        switch (arch)
        {
        case ArchType::Base:
            //DFSimTest::GemmTest::gemm_base(debug);
            break;
        case ArchType::DGSF:
            //DFSimTest::MemoryTest::memory_test(debug);
            break;
        case ArchType::SGMF:
            DFSimTest::BfsTest::bfs_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::Aes:
    {
        switch (arch)
        {
        case ArchType::Base:
            DFSimTest::AesTest::aes_Base(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::AesTest::aes_DGSF(debug);
            break;
        case ArchType::SGMF:
            //DFSimTest::AesTest::bfs_SGMF(debug);
            break;
        }

        break;
    }
    case App_name::Lud:
    {
        switch (arch)
        {
        case ArchType::Base:
            DFSimTest::LudTest::lud_Base(debug);
            break;
        case ArchType::DGSF:
            DFSimTest::LudTest::lud_DGSF(debug);
            break;
        case ArchType::SGMF:
            //DFSimTest::LudTest::lud_SGMF(debug);
            break;
        }

        break;
    }
    default:
        std::cout << "Not selected a simulation function" << std::endl;
    }

//#ifdef DGSF
//    DFSimTest::simpleFlow_DGSF(debug);
//#endif
//#ifdef Base
//    DFSimTest::simpleFlow_Base(debug);
//#endif
//#ifdef SGMF
//    DFSimTest::simpleFlow_SGMF(debug);
//#endif

    //DFSimTest::MemoryTest::memory_test(debug);

    return 0;
}