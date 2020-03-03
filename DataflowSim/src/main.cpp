#include "../src/define/Define.hpp"
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"

#include "../tb/SimpleFlow_Base.hpp"
#include "../tb/SimpleFlow_DGSF.hpp"


//class Channel
//{
//public:
//	Channel() { std::cout << "Channel" << std::endl; }
//	void print() { std::cout << "This is Channel" << std::endl; }
//};
//
//class ChanDGSF : public Channel
//{
//public:
//	ChanDGSF() { std::cout << "ChanDGSF" << std::endl; }
//	void print() { std::cout << "This is ChannDGSF" << std::endl; }
//};
//
//class LC
//{
//public:
//	LC() {}
//	void func();
//	Channel* chan = new Channel();
//};
//
//void LC::func()
//{
//	chan->print();
//}
//
//class LcDGSF : public LC
//{
//public:
//	LcDGSF() {}
//	void func();
//	ChanDGSF* chan = new ChanDGSF();
//};
//
//void LcDGSF::func()
//{
//	chan->print();
//}

int main()
{
	//LcDGSF* lc = new LcDGSF();
	//lc->LC::func();

	DFSim::ClkDomain clk();
	std::cout << "Begin test" << std::endl;

	DFSim::Debug* debug = new DFSim::Debug(string(".\\resource\\output\\SimpleFlowTest\\log_") + string(xstr(ARCH)) + string(".txt"));

	debug->getFile() << std::endl;
	debug->getFile() << "******  System Config  *******" << std::endl;
	debug->getFile() << "Arch: " << xstr(ARCH) << std::endl;
	debug->getFile() << std::endl;
	debug->getFile() << "******************************" << std::endl;


#ifdef DGSF
	DFSimTest::simpleFlow_DGSF(debug);
#endif
#ifdef Base
	DFSimTest::simpleFlow_Base(debug);
#endif

	return 0;
}