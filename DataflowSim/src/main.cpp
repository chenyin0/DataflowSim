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

#include "./module/execution/Channel.h"

using namespace DFSim;

int main()
{
    DFSim::ClkDomain clk();
    std::cout << "Begin test" << std::endl;

    /* TestBench name list
    SimpleFlowTest
    MemoryTest
    Gemm
    Bfs
    */
    TestBench_name tb_name = TestBench_name::Aes;
    ArchType arch = ArchType::Base;

    DFSim::Debug* debug = new DFSim::Debug(string("./resource/") + tb_name_convert::toString(tb_name) + string("/") + tb_name_convert::toString(tb_name) + string("_log_") + string(xstr(ARCH)) + string(".txt"));

    //DFSim::Debug* debug = new DFSim::Debug(string("./resource/output/MemoryTest/memory_test.txt"));

#ifdef ARCH
    debug->getFile() << std::endl;
    debug->getFile() << "******  System Config  *******" << std::endl;
    debug->getFile() << "Arch: " << xstr(ARCH) << std::endl;
    debug->getFile() << std::endl;
    debug->getFile() << "******************************" << std::endl;
#endif

    switch (tb_name)
    {
    case TestBench_name::SimpleFlowTest:
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
    case TestBench_name::MemoryTest:
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
    case TestBench_name::Gemm:
    {
        switch (arch)
        {
        case ArchType::Base:
            DFSimTest::GemmTest::gemm_Base(debug);
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
    case TestBench_name::Bfs:
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
    case TestBench_name::Aes:
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
            //DFSimTest::BfsTest::bfs_SGMF(debug);
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