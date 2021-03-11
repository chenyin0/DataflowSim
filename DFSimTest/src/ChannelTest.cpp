#include "CppUnitTest.h"

#include "../../DataflowSim/src/module/DataType.h"
#include "../../DataflowSim/src/module/ClkSys.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DFSimTest
{
	using namespace DFSim;

	TEST_CLASS(ChannelTest)
	{
	public:
		/*
		test case: Matrix multiply
		Matrix C = Matrix A * Matrix B (size = 3*3)
		*/
		TEST_METHOD(SimpleFlow)
		{
			//ClkDomain clk();

			// create input data
			vector<int> a = { 8, 9, 6, 7, 12, 59, 86, 75, 106 };
			vector<int> b = { 26, 8, 3, 47, 86, 159, 72, 48, 87 };

			//create output data
			vector<int> c;
			int size = 3;
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < size; ++j)
				{
					c.push_back(a[i * size + j] * b[j * size + i]);
				}
			}
			
			// print output
			for (auto i : c)
			{
				//std::cout << i << std::endl;
				Logger::WriteMessage("hello world");
			}

			uint loop = 1;
			while (loop)
			{
				--loop;



				//DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
				ClkDomain::getInstance()->selfAdd();
			}
		}
	};
}