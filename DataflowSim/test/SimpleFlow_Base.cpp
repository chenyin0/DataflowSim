///*
//Develop log
//	1. sendEndUpdate() destroy encapsulation
//*/
//
//#include "../src/define/Define.hpp"
//#include "../src/module/execution/Channel.h"
//#include "../src/module/ClkSys.h"
//#include "../src/module/execution/Lc.h"
//#include "../src/sim/Debug.h"
//
//
//namespace DFSimTest
//{
//	using namespace DFSim;
//	/*
//	test case: Matrix multiply
//	Matrix C = Matrix A * Matrix B (size = 3*3)
//	*/
//
//	void simpleFlow_Base(Debug* debug)
//	{
//		// Create input data
//		vector<int> a = { 8, 9, 5, 7, 12, 59, 86, 75, 106 };
//		vector<int> b = { 26, 8, 3, 47, 86, 159, 72, 48, 87 };
//
//		// Declare lc
//		ChanBase* lc0_loopVar = new ChanBase(2, 0);
//		ChanBase* lc0_getEnd = new ChanBase(2, 0);
//		ChanBase* lc0_sendEnd = new ChanBase(2, 0);
//
//		ChanBase* lc0_mux_trueChan = new ChanBase(2, 0);
//		ChanBase* lc0_mux_falseChan = new ChanBase(2, 0);
//		ChanBase* lc0_mux_outChan = new ChanBase(2, 0);
//
//		Mux* lc0_mux = new Mux(lc0_mux_trueChan, lc0_mux_falseChan, lc0_mux_outChan);
//		lc0_mux->addPort({ lc0_loopVar }, { }, { lc0_loopVar });
//		//Mux* lc0_mux = new Mux({ lc0_loopVar }, { }, { lc0_loopVar });
//		Lc* lc0 = new Lc(lc0_loopVar, lc0_getEnd, lc0_sendEnd, lc0_mux);
//
//		ChanBase* lc1_loopVar = new ChanBase(2, 0);
//		ChanBase* lc1_getEnd = new ChanBase(2, 0);
//		ChanBase* lc1_sendEnd = new ChanBase(2, 0);
//
//		ChanBase* lc1_mux_trueChan = new ChanBase(2, 0);
//		ChanBase* lc1_mux_falseChan = new ChanBase(2, 0);
//		ChanBase* lc1_mux_outChan = new ChanBase(2, 0);
//
//		Mux* lc1_mux = new Mux(lc1_mux_trueChan, lc1_mux_falseChan, lc1_mux_outChan);
//		lc1_mux->addPort({ lc1_loopVar }, { }, { lc1_loopVar });
//		//Mux* lc1_mux = new Mux({ lc1_loopVar }, { }, { lc1_loopVar });
//		Lc* lc1 = new Lc(lc1_loopVar, lc1_getEnd, lc1_sendEnd, lc1_mux);
//
//		// Create channels
//		ChanBase* i_lc1 = new ChanBase(2, 0);  // a[i * size + j] * b[j * size + i]
//		i_lc1->keepMode = 1;
//
//		ChanBase* i_data = new ChanBase(12, 7);   // channel size = cycle + exepected size, in order to avoid stall
//		ChanBase* j_data = new ChanBase(12, 7);
//		ChanBase* c_lc1 = new ChanBase(20, 0);
//
//		ChanBase* begin = new ChanBase(1, 0);
//		begin->noUpstream = 1;
//		//begin->enable = 1;
//		ChanBase* end = new ChanBase(1, 0);
//		end->noDownstream = 1;
//
//		// define channel interconnect
//		begin->addDownstream({/*lc0->loopVar*/ lc0_mux_falseChan /*, i_lc0*/});
//		end->addUpstream({lc0->sendEnd});
//		
//		i_lc1->addUpstream({/*i_lc0/*, lc1->cond*/ lc0->loopVar});
//		i_lc1->addDownstream({i_data, lc1->loopVar/*, j_lc1*//*, c_lc1*/});
//
//		i_data->addUpstream({/*lc1->cond*/ lc1->loopVar, i_lc1});
//		i_data->addDownstream({ /*lc1->cond, */c_lc1 });
//
//		j_data->addUpstream({/*j_lc1*/ lc1->loopVar});
//		j_data->addDownstream({ c_lc1 });
//		
//		c_lc1->addUpstream({i_data, j_data/*, i_lc1*/});
//		c_lc1->addDownstream({lc1->getEnd});
//		
//		// LC addPort : getAct, sendAct, getEnd, sendEnd
//		lc0->addPort({/*begin*/}, {/*i_lc0*/i_lc1}, {lc1->sendEnd}, {end});
//		lc0->addDependence({ begin }, {});  // No loop dependence
//
//		lc1->addPort({i_lc1 /*i_data*/}, {/*j_lc1*/ j_data, i_data}, {c_lc1}, {lc0->getEnd});
//		lc1->addDependence({}, {});  // No loop dependence
//
//		// Create output data
//		vector<int> c;
//		int size = 3;
//		for (int i = 0; i < size; ++i)
//		{
//			for (int j = 0; j < size; ++j)
//			{
//				c.push_back(a[i * size + j] * b[j * size + i]);
//			}
//		}
//
//		debug->vecPrint("Output reference", c);
//
//		// Initiation
//		begin->get(1); // trigger
//		uint iter = 0;
//
//		int i = 0;
//		int j = 0;
//
//		vector<int> result;
//
//		// Execute
//		while (iter < 50)
//		{
//			DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
//			int clk = DFSim::ClkDomain::getInstance()->getClk();
//			debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
//			debug->getFile() << " Clk:" << clk << " ********************" << std::endl;
//
//			// Outer LC0
//			i = lc0->mux->mux(i, 0, lc0->sel);
//			lc0->mux->muxUpdate(lc0->sel);
//			lc0->loopVar->get(i);
//			i = lc0->loopVar->assign() + 1;
//			lc0->lcUpdate(i < size);
//
//			//begin->get(1);  // update begin
//			begin->valid = 0;
//
//			// loop interface: var i
//			int i_0 = lc0->loopVar->assign();
//			i_lc1->get(i_0);
//
//			// Inner LC1
//			j = lc1->mux->mux(j, 0, lc1->sel);
//			lc1->mux->muxUpdate(lc1->sel);
//			lc1->loopVar->get(j);
//			j = lc1->loopVar->assign() + 1;
//			lc1->lcUpdate(j < size);
//
//			int i_1 = i_lc1->assign();
//			i_data->get(i_1);
//			int i_d = i_data->assign();
//
//			int j_1 = lc1->loopVar->assign();
//			j_data->get(j_1);
//			int j_d = j_data->assign();
//
//			int c_1 = a[i_d * size + j_d] * b[j_d * size + i_d];
//			vector<int> tmp = c_lc1->get(c_1);
//
//			end->get(1);
//
//			if (tmp[2])
//				result.push_back(tmp[3]);
//
//			
//			// Print log
//			debug->getFile() << std::endl;
//			debug->getFile() << "Outer Loop i: " << i_1 << std::endl;
//			debug->getFile() << "Inner Loop j: " << j_1 << std::endl;
//
//			debug->vecPrint("Result", result);
//
//			debug->chanPrint("begin", begin);
//			//debug->chanPrint("i_lc0", i_lc0);
//			debug->chanPrint("lc0->loopVar", lc0->loopVar);
//			debug->chanPrint("i_lc1", i_lc1);
//			debug->chanPrint("lc1->loopVar", lc1->loopVar);
//			debug->chanPrint("i_data", i_data);
//			//debug->chanPrint("j_lc1", j_lc1);
//			debug->chanPrint("j_data", j_data);
//			debug->chanPrint("c_lc1", c_lc1);
//
//			debug->chanPrint("lc1->getEnd", lc1->getEnd);
//			/*debug->getFile() << "lc1 loopNum: " << lc1->loopNum << std::endl;
//			debug->getFile() << "lc1 loopEnd: " << lc1->loopEnd << std::endl;
//			if (!lc1->loopNumQ.empty()) {
//				debug->getFile() << "lc1 loopNumQ: " << lc1->loopNumQ.front() << std::endl;
//			}*/
//			debug->chanPrint("lc1->sendEnd", lc1->sendEnd);
//
//			debug->chanPrint("lc0->getEnd", lc0->getEnd);
//			/*debug->getFile() << "lc0 loopNum: " << lc0->loopNum << std::endl;
//			debug->getFile() << "lc0 loopEnd: " << lc0->loopEnd << std::endl;
//			if (!lc0->loopNumQ.empty()) {
//				debug->getFile() << "lc0 loopNumQ: " << lc0->loopNumQ.front() << std::endl;
//			}*/
//			debug->chanPrint("lc0->sendEnd", lc0->sendEnd);
//
//			debug->chanPrint("end", end);
//
//			if (!end->channel.empty())
//			{
//				std::cout << std::endl;
//				std::cout << "Arch: " << xstr(ARCH) << std::endl;
//				std::cout << "*******************************" << std::endl;
//				std::cout << "Execution finished succussfully" << std::endl;
//				std::cout << "*******************************" << std::endl;
//				std::cout << "Total Cycle: " << clk << std::endl;
//
//				debug->getFile() << std::endl;
//				debug->getFile() << "*******************************" << std::endl;
//				debug->getFile() << "Execution finished succussfully" << std::endl;
//				debug->getFile() << "*******************************" << std::endl;
//				debug->getFile() << "Total Cycle: " << clk << std::endl;
//
//				break;
//			}
//
//			++iter;
//		}
//
//	}
//
//}
