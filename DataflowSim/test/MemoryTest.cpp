#include "./MemoryTest.h"

using namespace DFSimTest;

void MemoryTest::memory_test(Debug* debug)
{
	// Define module
	MemSystem* memSys = new MemSystem();

	Lse* lse_ld = new Lse(16, 0, memSys);

	ChanBase* chan_sendAddr = new ChanBase(2, 0);
	ChanBase* chan_getData = new ChanBase(2, 0);

	// Define interconnect
	chan_sendAddr->noUpstream = 1;
	chan_sendAddr->addDownstream({ lse_ld });

	lse_ld->addUpstream({ chan_sendAddr });
	lse_ld->addDownstream({ chan_getData });

	chan_getData->addUpstream({ lse_ld });
	chan_getData->noDownstream = 1;


	vector<int> memVec;
	for (size_t i = 0; i < 10000; ++i)
	{
		memVec.push_back(i);
	}

	uint iter = 0;

	while (iter < 1000)
	{
		DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
		int clk = DFSim::ClkDomain::getInstance()->getClk();
		debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
		debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

		chan_sendAddr->get(iter);
		uint addr_req = chan_sendAddr->assign();

		lse_ld->get(false, addr_req);
		uint addr_ack = lse_ld->assign();

		int data = memVec[addr_ack];
		chan_getData->get(data);

		memSys->MemSystemUpdate();

		// Print log
		debug->chanPrint("chan_sendAddr", chan_sendAddr);
		debug->lsePrint("lse_ld", lse_ld);
		debug->chanPrint("chan_getData", chan_getData);

		debug->memSysPrint(memSys);

		++iter;
	}


}