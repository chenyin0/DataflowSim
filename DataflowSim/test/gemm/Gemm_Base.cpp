#include "./Gemm.h"

using namespace DFSim;

/*
Src Code(from MachSuite, blocked gemm):

void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
	int i, k, j, jj, kk;
	int i_row, k_row;
	TYPE temp_x, mul;

	loopjj:for (jj = 0; jj < row_size; jj += block_size){
		loopkk:for (kk = 0; kk < row_size; kk += block_size){
			loopi:for ( i = 0; i < row_size; ++i){
				loopk:for (k = 0; k < block_size; ++k){
					i_row = i * row_size;
					k_row = (k  + kk) * row_size;
					temp_x = m1[i_row + k + kk];
					loopj:for (j = 0; j < block_size; ++j){
						mul = temp_x * m2[k_row + j + jj];
						prod[i_row + j + jj] += mul;
					}
				}
			}
		}
	}
}

*/

void GemmTest::gemm_base(Debug* debug)
{
	// Generate benchmark data
	generateData();

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