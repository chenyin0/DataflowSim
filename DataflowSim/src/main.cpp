#include "../src/define/Define.hpp"
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"
#include "../tb/SimpleFlowTest.hpp"

int main()
{
	DFSim::ClkDomain clk();
	std::cout << "Begin test" << std::endl;

	DFSim::Debug* debug = new DFSim::Debug(string(".\\resource\\output\\SimpleFlowTest\\log_") + string(xstr(ARCH)) + string(".txt"));

	debug->getFile() << std::endl;
	debug->getFile() << "******  System Config  *******" << std::endl;
	debug->getFile() << "Arch: " << xstr(ARCH) << std::endl;
	debug->getFile() << std::endl;
	debug->getFile() << "******************************" << std::endl;


	//std::ofstream log;
	//log.open(".\\resource\\output\\SimpleFlowTest\\log.txt");
	//if (log.is_open())
	//	std::cout << "file open" << std::endl;
	//log << DFSim::ClkDomain::getInstance()->getCurrentSystemTime() << std::endl;

	DFSimTest::simpleFlowTest(debug);

	return 0;
}