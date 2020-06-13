#include "../src/define/Define.hpp"
#include "../src/define/Para.h"
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"

#include "../test/SimpleFlow_Base.cpp"
#include "../test/SimpleFlow_DGSF.cpp"
#include "../test/SimpleFlow_SGMF.cpp"

#include "../test/MemoryTest.h"
#include "../test/gemm/Gemm.h"
#include "../test/bfs/Bfs.h"

#include "./module/execution/Channel.h"

using namespace DFSim;
//class Channel
//{
//public:
//    Channel() { std::cout << "Channel" << std::endl; }
//    void print() { std::cout << "This is Channel" << std::endl; }
//};
//
//class ChanDGSF : public Channel
//{
//public:
//    ChanDGSF() { std::cout << "ChanDGSF" << std::endl; }
//    void print() { std::cout << "This is ChannDGSF" << std::endl; }
//};
//
//class LC
//{
//public:
//    LC() {}
//    void func();
//    Channel* chan = new Channel();
//};
//
//void LC::func()
//{
//    chan->print();
//}
//
//class LcDGSF : public LC
//{
//public:
//    LcDGSF() {}
//    void func();
//    ChanDGSF* chan = new ChanDGSF();
//};
//
//void LcDGSF::func()
//{
//    chan->print();
//}

//class A
//{
//public:
//    A()
//        :n2(0),
//        n1(n2 + 2)
//    {
//        //std::cout << "AAA" << std::endl;
//        //std::cout << std::endl;
//        //std::cout << std::endl;
//    }
//
//    //~A();
//
//    void print()
//    {
//        //std::cout << "n1:" << n1 << " ,  n2:" << n2 << std::endl;
//        //std::cout << "n3:" << n3 << std::endl;
//        std::cout << "this is class A" << std::endl;
//    }
//
//private:
//    int n2;
//    int n1;
//    int n3 = n1;
//};
//
//class B : public A
//{
//public:
//    B() {}
//
//    int val = 9;
//    bool te = 0;
//
//    void print()
//    {
//        std::cout << "this is class B" << std::endl;
//    }
//};


int main()
{
    /*A a;
    a.print();
    B b;*/
    /*A* a = new A;
    B* b = new B;
    A* tmp = b;
    std::cout << "a= " << a << std::endl;
    std::cout << "b= " << b << std::endl;
    std::cout << "tmp= " << tmp << std::endl;
    system("pause");*/

    //B* chan1 = new B();
    //B* chan2 = new B();
    //vector<B*> chanBundle = { chan1, chan2 };

    //for (auto& i : chanBundle)
    //{
    //    i->print();
    //}

    ////vector<A*> Abundle = chanBundle;
    //vector<A*> Abundle = Util::cast(chanBundle, (vector<A*>*)nullptr);
    //
    //for (auto& i : Abundle)
    //{
    //    i->print();
    //}

    //delete chan1, chan2;

    //system("pause");

    //cout << "hello" << endl;

    //uint clkk = 0;
    //uint buffer_size = 210;
    //uint dram_lat = 200;
    //uint tile = 16;
    //uint buffer_cnt = 210;
    //deque<uint> req;

    //while (clkk < 100000)
    //{
    //    if (clkk % 16 == 0)
    //    {
    //        req.push_back(dram_lat);
    //    }

    //    --buffer_cnt;

    //    for (auto& i : req)
    //    {
    //        --i;
    //        if (i == 0)
    //        {
    //            buffer_cnt += 16;
    //            req.pop_front();
    //        }
    //    }

    //    ++clkk;

    //    cout << buffer_cnt << endl;
    //    if (buffer_cnt == 0)
    //    {
    //        cout << "!!!!!!!!!" << endl;
    //        system("pause");
    //    }
    //}
    //system("pause");

    DFSim::ClkDomain clk();
    std::cout << "Begin test" << std::endl;

    /* TestBench name list
    SimpleFlowTest
    MemoryTest
    Gemm
    */
    TestBench_name tb_name = TestBench_name::Gemm;
    ArchType arch = ArchType::DGSF;

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