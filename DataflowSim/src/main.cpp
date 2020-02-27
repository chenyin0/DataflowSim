#include <iostream>
#include "../src/module/ClkSys.h"
#include "../src/sim/Debug.h"
#include "../tb/SimpleFlowTest.hpp"

int main()
{
	DFSim::ClkDomain clk();
	std::cout << "Begin test" << std::endl;

	DFSim::Debug* debug = new DFSim::Debug(".\\resource\\output\\SimpleFlowTest\\log.txt");

	//std::ofstream log;
	//log.open(".\\resource\\output\\SimpleFlowTest\\log.txt");
	//if (log.is_open())
	//	std::cout << "file open" << std::endl;
	//log << DFSim::ClkDomain::getInstance()->getCurrentSystemTime() << std::endl;

	DFSimTest::simpleFlowTest(debug);

	return 0;
}