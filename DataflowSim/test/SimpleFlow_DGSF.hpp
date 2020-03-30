/*
Develop log
	1. sendEndUpdate() destroy encapsulation
*/

#include "../src/define/Define.hpp"
#include "../src/module/execution/Channel.h"
#include "../src/module/ClkSys.h"
#include "../src/module/execution/Lc.h"
#include "../src/sim/Debug.h"


namespace DFSimTest
{
	using namespace DFSim;
	/*
	test case: Matrix multiply
	Matrix C = Matrix A * Matrix B (size = 3*3)
	*/
	void simpleFlow_DGSF(Debug* debug)
	{
		// Create input data
		vector<int> a = { 8, 9, 5, 7, 12, 59, 86, 75, 106 };
		vector<int> b = { 26, 8, 3, 47, 86, 159, 72, 48, 87 };

		// Declare lc
		ChanDGSF* lc0_loopVar = new ChanDGSF(2, 0, 1);
		ChanDGSF* lc0_getEnd = new ChanDGSF(2, 0, 1);
		ChanDGSF* lc0_sendEnd = new ChanDGSF(2, 0, 1);

		Channel* lc0_mux_trueChan = new Channel(2, 0);
		Channel* lc0_mux_falseChan = new Channel(2, 0);
		Channel* lc0_mux_outChan = new Channel(2, 0);

		Mux* lc0_mux = new Mux(lc0_mux_trueChan, lc0_mux_falseChan, lc0_mux_outChan);
		lc0_mux->addPort({ lc0_loopVar }, { }, { lc0_loopVar });
		//Mux* lc0_mux = new Mux({ lc0_loopVar }, { }, { lc0_loopVar });
		LcDGSF* lc0 = new LcDGSF(lc0_loopVar, lc0_getEnd, lc0_sendEnd, lc0_mux, BRAM_BANK_DEPTH);

		ChanDGSF* lc1_loopVar = new ChanDGSF(2, 0, 1);
		ChanDGSF* lc1_getEnd = new ChanDGSF(2, 0, 1);
		ChanDGSF* lc1_sendEnd = new ChanDGSF(2, 0, 1);

		Channel* lc1_mux_trueChan = new Channel(2, 0);
		Channel* lc1_mux_falseChan = new Channel(2, 0);
		Channel* lc1_mux_outChan = new Channel(2, 0);

		Mux* lc1_mux = new Mux(lc1_mux_trueChan, lc1_mux_falseChan, lc1_mux_outChan);
		lc1_mux->addPort({ lc1_loopVar }, { }, { lc1_loopVar });
		//Mux* lc1_mux = new Mux({ lc1_loopVar }, { }, { lc1_loopVar });
		LcDGSF* lc1 = new LcDGSF(lc1_loopVar, lc1_getEnd, lc1_sendEnd, lc1_mux, BRAM_BANK_DEPTH);

		// Create channels
		ChanDGSF* i_tmp = new ChanDGSF(BRAM_BANK_DEPTH, 0, 1);
		ChanDGSF* i_lc1 = new ChanDGSF(BRAM_BANK_DEPTH/*2*/, 0, 1);  // a[i * size + j] * b[j * size + i]
		i_lc1->keepMode = 1;

		ChanDGSF* i_data = new ChanDGSF(12, 7, 5);   // channel size = cycle + exepected size, in order to avoid stall
		ChanDGSF* j_data = new ChanDGSF(12, 7, 5);
		ChanDGSF* c_lc1 = new ChanDGSF(20, 0, 1);

		ChanDGSF* begin = new ChanDGSF(1, 0, 1);
		begin->noUpstream = 1;
		//begin->enable = 1;
		ChanDGSF* end = new ChanDGSF(1, 0, 1);
		end->noDownstream = 1;

		// Define channel interconnect
		begin->addDownstream({ /*lc0->loopVar*/ lc0_mux_falseChan });
		end->addUpstream({ lc0->sendEnd });

		i_tmp->addUpstream({ lc0->loopVar });
		i_tmp->addDownstream({ i_lc1 });

		i_lc1->addUpstream({ i_tmp });
		i_lc1->addDownstream({ i_data, lc1->loopVar });

		i_data->addUpstream({ lc1->loopVar, i_lc1 });
		i_data->addDownstream({ c_lc1 });

		j_data->addUpstream({ lc1->loopVar });
		j_data->addDownstream({ c_lc1 });

		c_lc1->addUpstream({ i_data, j_data });
		c_lc1->addDownstream({ lc1->getEnd });

		// LC addPort : getAct, sendAct, getEnd, sendEnd
		lc0->addPort({ /*begin*/ }, { i_tmp }, { lc1->sendEnd }, { end });
		lc0->addDependence({begin}, {});  // No loop dependence
		lc1->addPort({ i_lc1 }, { j_data, i_data }, { c_lc1 }, { lc0->getEnd });
		lc1->addDependence({}, {});  // No loop dependence

		// Define activeChannel
		i_tmp->sendActiveMode = 1;
		i_tmp->activeStream = { i_lc1 };

		i_lc1->sendActiveMode = 1;
		i_lc1->activeStream = {i_tmp/*, begin*/};

		// Disable channel: set all the in activeMode channels' enable = 0, except the first channel(usual the most outer loop->cond)
		i_lc1->enable = 0;


		// Create output data
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
		while (iter < 100)
		{
			DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
			int clk = DFSim::ClkDomain::getInstance()->getClk();
			debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
			debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

			// Outer LC0
			i = lc0->mux->mux(i, 0, lc0->sel);
			lc0->mux->muxUpdate(lc0->sel);
			lc0->loopVar->get(i);
			i = lc0->loopVar->assign() + 1;
			lc0->lcUpdate(i < size);

			//begin->get(1);  // update begin
			begin->valid = 0;

			// loop interface: var i
			int i_0 = lc0->loopVar->assign();
			i_tmp->get(i_0);
			int i_temp = i_tmp->assign();
			i_lc1->get(i_temp);

			// Inner LC1
			j = lc1->mux->mux(j, 0, lc1->sel);
			lc1->mux->muxUpdate(lc1->sel);
			lc1->loopVar->get(j);
			j = lc1->loopVar->assign() + 1;
			lc1->lcUpdate(j < size);

			int i_1 = i_lc1->assign();
			i_data->get(i_1);
			int i_d = i_data->assign();

			int j_1 = lc1->loopVar->assign();
			j_data->get(j_1);
			int j_d = j_data->assign();

			int c_1 = a[i_d * size + j_d] * b[j_d * size + i_d];
			vector<int> tmp = c_lc1->get(c_1);

			end->get(1);

			if (tmp[2])
				result.push_back(tmp[3]);


			// Print log
			debug->getFile() << std::endl;
			debug->getFile() << "Outer Loop i: " << i_1 << std::endl;
			debug->getFile() << "Inner Loop j: " << j_1 << std::endl;

			debug->vecPrint("Result", result);

			debug->chanPrint("begin", begin);
			debug->chanPrint("lc0->loopVar", lc0->loopVar);
			debug->chanPrint("i_tmp", i_tmp);
			debug->chanPrint("i_lc1", i_lc1);
			debug->chanPrint("lc1->loopVar", lc1->loopVar);
			/*debug->chanPrint("lc1_mux_trueChan", lc1_mux_trueChan);
			debug->chanPrint("lc1_mux_falseChan", lc1_mux_falseChan);*/
			debug->chanPrint("lc1_mux_outChan", lc1_mux_outChan);
			debug->chanPrint("i_data", i_data);
			debug->chanPrint("j_data", j_data);
			debug->chanPrint("c_lc1", c_lc1);

			debug->chanPrint("lc1->getEnd", lc1_getEnd);
			debug->getFile() << "lc1 loopEnd: " << lc1->loopEnd << std::endl;
			debug->chanPrint("lc1->sendEnd", lc1_sendEnd);
			debug->chanPrint("lc0->getEnd", lc0_getEnd);
			debug->getFile() << "lc0 loopEnd: " << lc0->loopEnd << std::endl;
			debug->chanPrint("lc0->sendEnd", lc0_sendEnd);
			debug->chanPrint("end", end);
			
			if (!end->channel.empty())
			{
				std::cout << std::endl;
				std::cout << "Arch: " << xstr(ARCH) << std::endl;
				std::cout << "*******************************" << std::endl;
				std::cout << "Execution finished succussfully" << std::endl;
				std::cout << "*******************************" << std::endl;
				std::cout << "Total Cycle: " << clk << std::endl;

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
