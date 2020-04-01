/*
Develop log
	1. sendEndUpdate() destroy encapsulation
*/

#include "../src/define/Define.hpp"
#include "../src/module/execution/Channel.h"
#include "../src/module/ClkSys.h"
#include "../src/module/execution/Lc.h"
#include "../src/sim/Debug.h"
#include "../src/define/Para.h"


namespace DFSimTest
{
	using namespace DFSim;
	/*
	test case: Matrix multiply
	Matrix C = Matrix A * Matrix B (size = 3*3)
	*/
	void simpleFlow_SGMF(Debug* debug)
	{
		// Create input data
		vector<int> a = { 8, 9, 5, 7, 12, 59, 86, 75, 106 };
		vector<int> b = { 26, 8, 3, 47, 86, 159, 72, 48, 87 };

		// Declare lc
		ChanSGMF* lc0_loopVar = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // BundleSize = 1
		ChanBase* lc0_getEnd = new ChanBase(2, 0);
		ChanBase* lc0_sendEnd = new ChanBase(2, 0);

		ChanSGMF* lc0_mux_trueChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);
		ChanSGMF* lc0_mux_falseChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);
		ChanSGMF* lc0_mux_outChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);

		MuxSGMF* lc0_mux = new MuxSGMF(lc0_mux_trueChan, lc0_mux_falseChan, lc0_mux_outChan);
		lc0_mux->addPort({ lc0_loopVar }, { }, { lc0_loopVar });
		//MuxSGMF* lc0_mux = new MuxSGMF({ lc0_loopVar }, { }, { lc0_loopVar });
		LcSGMF* lc0 = new LcSGMF(lc0_loopVar, lc0_getEnd, lc0_sendEnd, lc0_mux);

		ChanSGMF* lc1_loopVar = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // BundleSize = 1
		ChanBase* lc1_getEnd = new ChanBase(2, 0);
		ChanBase* lc1_sendEnd = new ChanBase(2, 0);

		ChanSGMF* lc1_mux_trueChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);
		ChanSGMF* lc1_mux_falseChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);
		ChanSGMF* lc1_mux_outChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);

		MuxSGMF* lc1_mux = new MuxSGMF(lc1_mux_trueChan, lc1_mux_falseChan, lc1_mux_outChan);
		lc1_mux->addPort({ lc1_loopVar }, { }, { lc1_loopVar });
		//MuxSGMF* lc1_mux = new MuxSGMF({ lc1_loopVar }, { }, { lc1_loopVar });
		LcSGMF* lc1 = new LcSGMF(lc1_loopVar, lc1_getEnd, lc1_sendEnd, lc1_mux);

		// Create channels
		ChanSGMF* i_lc1 = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // a[i * size + j] * b[j * size + i]
		i_lc1->keepMode = 1;

		ChanSGMF* i_data = new ChanSGMF(INPUT_BUFF_SIZE, 4, 1);  // channel size = cycle + exepected size, in order to avoid stall
		ChanSGMF* j_data = new ChanSGMF(INPUT_BUFF_SIZE, 4, 1);

		ChanSGMF* chan_addr_a = new ChanSGMF(INPUT_BUFF_SIZE, 0);  // Memory latency
		ChanSGMF* chan_addr_b = new ChanSGMF(INPUT_BUFF_SIZE, 0);  // Memory latency

		ChanSGMF* chan_array_a = new ChanSGMF(INPUT_BUFF_SIZE, 3, 1);
		ChanSGMF* chan_array_b = new ChanSGMF(INPUT_BUFF_SIZE, 3, 1);

		ChanSGMF* c_lc1 = new ChanSGMF(INPUT_BUFF_SIZE, 0);

		ChanSGMF* chan_result = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);
		chan_result->noUpstream = 1;
		chan_result->noDownstream = 1;

		ChanBase* begin = new ChanBase(1, 0);
		begin->noUpstream = 1;
		//begin->enable = 1;
		ChanBase* end = new ChanBase(1, 0);
		end->noDownstream = 1;

		// Define channel interconnect
		begin->addDownstream({ /*lc0->loopVar*/  lc0_mux_falseChan });
		end->addUpstream({ lc0->sendEnd });

		i_lc1->addUpstream({ lc0->loopVar });
		i_lc1->addDownstream({ i_data, lc1->loopVar });

		i_data->addUpstream( Util::cast(vector<ChanSGMF*>{ lc1->loopVar, i_lc1 }, (vector<Channel*>*)nullptr));
		i_data->addDownstream({ /*c_lc1*/ chan_addr_a, chan_addr_b });

		j_data->addUpstream({ lc1->loopVar });
		j_data->addDownstream({ /*c_lc1*/ chan_addr_a, chan_addr_b });

		chan_addr_a->addUpstream({ Util::cast(vector<ChanSGMF*>{i_data}, (vector<Channel*>*)nullptr), 
							  Util::cast(vector<ChanSGMF*>{j_data}, (vector<Channel*>*)nullptr) });
		chan_addr_a->addDownstream({ chan_array_a });

		chan_addr_b->addUpstream({ Util::cast(vector<ChanSGMF*>{j_data}, (vector<Channel*>*)nullptr),
							  Util::cast(vector<ChanSGMF*>{i_data}, (vector<Channel*>*)nullptr) });
		chan_addr_b->addDownstream({ chan_array_b });

		chan_array_a->addUpstream({ chan_addr_a });
		chan_array_a->addDownstream({ c_lc1 });

		chan_array_b->addUpstream({ chan_addr_b });
		chan_array_b->addDownstream({ c_lc1 });

		c_lc1->addUpstream({ Util::cast(vector<ChanSGMF*>{chan_array_a}, (vector<Channel*>*)nullptr),
							 Util::cast(vector<ChanSGMF*>{chan_array_b}, (vector<Channel*>*)nullptr) });
		c_lc1->addDownstream({ lc1->getEnd/*, chan_result*/});

		//chan_result->addUpstream({ c_lc1 });

		// LC addPort : getAct, sendAct, getEnd, sendEnd
		lc0->addPort({ /*begin*/ }, {/*i_lc0*/i_lc1 }, { lc1->sendEnd }, { end });
		lc0->addDependence({begin}, {});  // No loop dependence

		lc1->addPort({ i_lc1 /*i_data*/ }, {/*j_lc1*/ j_data, i_data }, { c_lc1 }, { lc0->getEnd });
		lc1->addDependence({}, {});  // No loop dependence

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
		while (iter < 500)
		{
			DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
			int clk = DFSim::ClkDomain::getInstance()->getClk();
			debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
			debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

			// Outer LC0
			i = lc0->mux->mux(i, 0, lc0->sel);
			lc0->mux->muxUpdate(lc0->sel);
			lc0->loopVar->get(i);
			i = lc0->loopVar->assign(0) + 1;
			lc0->lcUpdate(i < size);

			//begin->get(1);  // update begin
			begin->valid = 0;

			// loop interface: var i
			int i_0 = lc0->loopVar->assign(0);
			i_lc1->get(i_0);

			// Inner LC1
			j = lc1->mux->mux(j, 0, lc1->sel);
			lc1->mux->muxUpdate(lc1->sel);
			lc1->loopVar->get(j);
			j = lc1->loopVar->assign(0) + 1;
			lc1->lcUpdate(j < size);

			int i_1 = i_lc1->assign(0);
			i_data->get(i_1);
			int i_d = i_data->assign(0);

			int j_1 = lc1->loopVar->assign(0);
			j_data->get(j_1);
			int j_d = j_data->assign(0);

			//int addr_a = i_d * size + j_d;
			chan_addr_a->get({ i_d, j_d });
			int addr_a = chan_addr_a->assign(0) * size + chan_addr_a->assign(1);

			chan_addr_b->get({ j_d, i_d });
			int addr_b = chan_addr_b->assign(0) * size + chan_addr_b->assign(1);

			chan_array_a->get(addr_a);
			int array_a = a[chan_array_a->assign(0)];

			chan_array_b->get(addr_b);
			int array_b = b[chan_array_b->assign(0)];

			c_lc1->get({ array_a, array_b });
			int c_1 = c_lc1->assign(0) * c_lc1->assign(1);

			//int c_1 = a[i_d * size + j_d] * b[j_d * size + i_d];
			//vector<int> tmp = c_lc1->get(c_1);
			//vector<int> tmp = chan_result->get(c_1);
	
			end->get(1);

			/*if (tmp[2])
				result.push_back(tmp[3]);*/
			result.push_back(c_1);


			// Print log
			debug->getFile() << std::endl;
			debug->getFile() << "Outer Loop i: " << i_1 << std::endl;
			debug->getFile() << "Inner Loop j: " << j_1 << std::endl;

			debug->vecPrint("Result", result);

			debug->chanPrint("begin", begin);
			//debug->chanPrint("i_lc0", i_lc0);
			debug->chanPrint("lc0->loopVar", lc0->loopVar);
			debug->chanPrint("i_lc1", i_lc1);
			debug->chanPrint("lc1_mux_trueChan", lc1_mux_trueChan);
			debug->chanPrint("lc1_mux_falseChan", lc1_mux_falseChan);
			debug->chanPrint("lc1_mux_outChan", lc1_mux_outChan);
			debug->chanPrint("lc1->loopVar", lc1->loopVar);

			debug->chanPrint("i_data", i_data);
			//debug->chanPrint("j_lc1", j_lc1);
			debug->chanPrint("j_data", j_data);

			debug->chanPrint("chan_addr_a", chan_addr_a);
			debug->chanPrint("chan_addr_b", chan_addr_b);

			debug->chanPrint("chan_array_a", chan_array_a);
			debug->chanPrint("chan_array_b", chan_array_b);

			debug->chanPrint("c_lc1", c_lc1);
			debug->chanPrint("chan_result", chan_result);

			debug->chanPrint("lc1->getEnd", lc1->getEnd);
			debug->chanPrint("lc1->sendEnd", lc1->sendEnd);
			debug->chanPrint("lc0->getEnd", lc0->getEnd);
			debug->chanPrint("lc0->sendEnd", lc0->sendEnd);

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
