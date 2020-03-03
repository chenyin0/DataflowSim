/*
Develop log
	1. sendEndUpdateÆÆ»µÁË·â×°ÐÔ
*/

#include "../src/define/Define.hpp"
#include "../src/module/Channel.h"
#include "../src/module/ClkSys.h"
#include "../src/module/LC.h"
#include "../src/sim/Debug.h"


namespace DFSimTest
{
	using namespace DFSim;
	/*
	test case: Matrix multiply
	Matrix C = Matrix A * Matrix B (size = 3*3)
	*/

	void simpleFlow_Base(Debug* debug)
	{
		// create input data
		vector<int> a = { 8, 9, 5, 7, 12, 59, 86, 75, 106 };
		vector<int> b = { 26, 8, 3, 47, 86, 159, 72, 48, 87 };

		// declare lc
		Channel* lc0_cond = new Channel(2, 0);
		Channel* lc0_getEnd = new Channel(2, 0);
		Channel* lc0_sendEnd = new Channel(2, 0);
		LC* lc0 = new LC(lc0_cond, lc0_getEnd, lc0_sendEnd);

		Channel* lc1_cond = new Channel(2, 0);
		Channel* lc1_getEnd = new Channel(2, 0);
		Channel* lc1_sendEnd = new Channel(2, 0);
		LC* lc1 = new LC(lc1_cond, lc1_getEnd, lc1_sendEnd);

		// create channels
		Channel* i_lc0 = new Channel(3, 0);
		Channel* i_lc1 = new Channel(2, 0);  // a[i * size + j] * b[j * size + i]
		i_lc1->keepMode = 1;

		Channel* i_data = new Channel(12, 7);   // channel size = cycle + exepected size, in order to avoid stall
		//i_data->keepMode = 1;  // inner/outer loop interface channel set in keepMode
		Channel* j_lc1 = new Channel(2, 0);
		Channel* j_data = new Channel(12, 7);
		Channel* c_lc1 = new Channel(20, 0);

		Channel* begin = new Channel(1, 0);
		begin->noUpstream = 1;
		//begin->enable = 1;
		Channel* end = new Channel(1, 0);
		end->noDownstream = 1;

		// define channel interconnect
		begin->addDownstream({lc0->cond, i_lc0});
		end->addUpstream({lc0->sendEnd});

		i_lc0->addUpstream({ lc0->cond, begin});
		i_lc0->addDownstream({ i_lc1 });
		
		i_lc1->addUpstream({i_lc0/*, lc1->cond*/});
		i_lc1->addDownstream({i_data, lc1->cond, j_lc1/*, c_lc1*/});

		i_data->addUpstream({lc1->cond, i_lc1});
		i_data->addDownstream({ /*lc1->cond, */c_lc1 });
		
		j_lc1->addUpstream({lc1->cond, i_lc1});
		j_lc1->addDownstream({j_data});

		j_data->addUpstream({j_lc1});
		j_data->addDownstream({ c_lc1 });
		
		c_lc1->addUpstream({i_data, j_data/*, i_lc1*/});
		c_lc1->addDownstream({lc1->getEnd});
		
		// LC addPort : getAct, sendAct, getEnd, sendEnd
		lc0->addPort({begin}, {i_lc0}, {lc1->sendEnd}, {end});
		lc1->addPort({i_lc1 /*i_data*/}, {j_lc1, i_data}, {c_lc1}, {lc0->getEnd});

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

		debug->vecPrint("Output reference", c);

		// Initiation
		begin->get(1); // trigger
		uint iter = 0;

		int i = 0;
		int j = 0;

		vector<int> result;

		// Execute
		while (iter < 50)
		{
			DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
			int clk = DFSim::ClkDomain::getInstance()->getClk();
			debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
			debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

			// Outer LC0
			i = lc0->mux(0, i);
			bool flag0 = i < size;
			lc0->cond->get(flag0);
			lc0->lcUpdate();
			i_lc0->get(i);  // push i into channel after LC cond has updated
			i = i_lc0->assign() + 1;

			begin->get(1);  // update begin

			// loop interface: var i
			int i_0 = i_lc0->assign();
			i_lc1->get(i_0);
			//int i_1 = i_lc1->assign();
			//i_data->get(i_1);
			//int i_d = i_data->assign();

			// Inner LC1
			j = lc1->mux(0, j);
			bool flag1 = j < size;
			lc1->cond->get(flag1);
			lc1->lcUpdate();
			j_lc1->get(j);
			j = j_lc1->assign() + 1;

			int i_1 = i_lc1->assign();
			i_data->get(i_1);
			int i_d = i_data->assign();

			int j_1 = j_lc1->assign();
			j_data->get(j_1);
			int j_d = j_data->assign();

			int c_1 = a[i_d * size + j_d] * b[j_d * size + i_d];
			vector<int> tmp = c_lc1->get(c_1);

			end->get(1);

			if (tmp[2])
				result.push_back(tmp[3]);

			
			// print log
			debug->getFile() << std::endl;
			debug->getFile() << "Outer Loop i: " << i_1 << std::endl;
			debug->getFile() << "Inner Loop j: " << j_1 << std::endl;

			debug->vecPrint("Result", result);

			debug->chanPrint("begin", begin);
			debug->chanPrint("i_lc0", i_lc0);
			debug->chanPrint("lc0->cond", lc0->cond);
			debug->chanPrint("i_lc1", i_lc1);
			debug->chanPrint("lc1->cond", lc1->cond);
			debug->chanPrint("i_data", i_data);
			debug->chanPrint("j_lc1", j_lc1);
			debug->chanPrint("j_data", j_data);
			debug->chanPrint("c_lc1", c_lc1);

			debug->chanPrint("lc1->getEnd", lc1->getEnd);
			debug->chanPrint("lc1->sendEnd", lc1->sendEnd);
			debug->chanPrint("lc0->getEnd", lc0->getEnd);
			debug->chanPrint("lc0->sendEnd", lc0->sendEnd);
			debug->chanPrint("end", end);

			if (!end->channel.empty())
			{
				debug->getFile() << std::endl;
				debug->getFile() << "*******************************" << std::endl;
				debug->getFile() << "Execution finished succussfully" << std::endl;
				debug->getFile() << "*******************************" << std::endl;

				debug->getFile() << "Total Cycle: " << clk << std::endl;

				break;
			}

			++iter;
		}

	}

}
