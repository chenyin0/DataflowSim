#include "./Gemm.h"

using namespace DFSimTest;

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
					i_row = i * row_size;  // Chan_i_row
					k_row = (k  + kk) * row_size;  // Chan_k_row
					temp_x = m1[i_row + k + kk];  // Chan_m1_addr, Lse_ld_m1, Chan_m1_getData
					loopj:for (j = 0; j < block_size; ++j){
						mul = temp_x * m2[k_row + j + jj];  // Chan_m2_addr, Lse_ld_m2
						prod[i_row + j + jj] += mul;  // Chan_partialSum_addr, Lse_ld_partialSum, Chan_partialSum, Lse_st_partialSum
					}
				}
			}
		}
	}
}

*/

void GemmTest::generateData()
{
	uint size = matrix_width * matrix_height;
	m1.resize(matrix_height);
	m2.resize(matrix_height);
	result.resize(matrix_height);

	for (size_t i = 0; i < matrix_height; ++i)
	{
		m1[i].resize(matrix_width);
		m2[i].resize(matrix_width);
		result[i].resize(matrix_width);

		for (size_t j = 0; j < matrix_width; ++j)
		{
			m1[i][j] = i * matrix_width + j;
			m2[i][j] = i * matrix_width + j;
		}
	}
}

const uint GemmTest::matrix_width = 500;
const uint GemmTest::matrix_height = matrix_width;
const uint GemmTest::block_size = 20;

vector<vector<int>> GemmTest::m1;
vector<vector<int>> GemmTest::m2;
vector<vector<int>> GemmTest::result;

void GemmTest::gemm_base(Debug* debug)
{
	// Generate benchmark data
	generateData();


	//******  Define module  ******//
	//*** Declare memory
	MemSystem* memSys = new MemSystem();


	//*** Declare Lse
	Lse* lse_ld_m1 = new Lse(16, 0, memSys);  // Load M1
	Lse* lse_ld_m2 = new Lse(16, 0, memSys);  // Load M2
	Lse* lse_ld_partialSum = new Lse(16, 0, memSys);  // load partial sum
	Lse* lse_st_partialSum = new Lse(16, 0, memSys);  // Store back partial sum


	//*** Declare Lc
	// Loop jj
	ChanBase* lc_jj_loopVar = new ChanBase(2, 0);
	ChanBase* lc_jj_getEnd = new ChanBase(2, 0);
	ChanBase* lc_jj_sendEnd = new ChanBase(2, 0);

	ChanBase* lc_jj_mux_trueChan = new ChanBase(2, 0);
	ChanBase* lc_jj_mux_falseChan = new ChanBase(2, 0);
	ChanBase* lc_jj_mux_outChan = new ChanBase(2, 0);

	ChanBase* chan_jj_lc = new ChanBase(2, 0);
	chan_jj_lc->keepMode = 1;

	Mux* lc_jj_mux = new Mux(lc_jj_mux_trueChan, lc_jj_mux_falseChan, lc_jj_mux_outChan);
	lc_jj_mux->addPort({ lc_jj_loopVar }, { }, { lc_jj_loopVar });
	//Mux* lc_jj_mux = new Mux({ lc_jj_loopVar }, { }, { lc_jj_loopVar });
	Lc* lc_jj = new Lc(lc_jj_loopVar, lc_jj_getEnd, lc_jj_sendEnd, lc_jj_mux);

	// Loop kk
	ChanBase* lc_kk_loopVar = new ChanBase(2, 0);
	ChanBase* lc_kk_getEnd = new ChanBase(2, 0);
	ChanBase* lc_kk_sendEnd = new ChanBase(2, 0);

	ChanBase* lc_kk_mux_trueChan = new ChanBase(2, 0);
	ChanBase* lc_kk_mux_falseChan = new ChanBase(2, 0);
	ChanBase* lc_kk_mux_outChan = new ChanBase(2, 0);

	ChanBase* chan_kk_lc = new ChanBase(2, 0);
	chan_kk_lc->keepMode = 1;

	Mux* lc_kk_mux = new Mux(lc_kk_mux_trueChan, lc_kk_mux_falseChan, lc_kk_mux_outChan);
	lc_kk_mux->addPort({ lc_kk_loopVar }, { }, { lc_kk_loopVar });
	//Mux* lc_kk_mux = new Mux({ lc_kk_loopVar }, { }, { lc_kk_loopVar });
	Lc* lc_kk = new Lc(lc_kk_loopVar, lc_kk_getEnd, lc_kk_sendEnd, lc_kk_mux);

	// Loop i
	ChanBase* lc_i_loopVar = new ChanBase(2, 0);
	ChanBase* lc_i_getEnd = new ChanBase(2, 0);
	ChanBase* lc_i_sendEnd = new ChanBase(2, 0);

	ChanBase* lc_i_mux_trueChan = new ChanBase(2, 0);
	ChanBase* lc_i_mux_falseChan = new ChanBase(2, 0);
	ChanBase* lc_i_mux_outChan = new ChanBase(2, 0);

	ChanBase* chan_i_lc = new ChanBase(2, 0);
	chan_i_lc->keepMode = 1;

	Mux* lc_i_mux = new Mux(lc_i_mux_trueChan, lc_i_mux_falseChan, lc_i_mux_outChan);
	lc_i_mux->addPort({ lc_i_loopVar }, { }, { lc_i_loopVar });
	//Mux* lc_i_mux = new Mux({ lc_i_loopVar }, { }, { lc_i_loopVar });
	Lc* lc_i = new Lc(lc_i_loopVar, lc_i_getEnd, lc_i_sendEnd, lc_i_mux);

	// Loop k
	ChanBase* lc_k_loopVar = new ChanBase(2, 0);
	ChanBase* lc_k_getEnd = new ChanBase(2, 0);
	ChanBase* lc_k_sendEnd = new ChanBase(2, 0);

	ChanBase* lc_k_mux_trueChan = new ChanBase(2, 0);
	ChanBase* lc_k_mux_falseChan = new ChanBase(2, 0);
	ChanBase* lc_k_mux_outChan = new ChanBase(2, 0);

	ChanBase* chan_k_lc = new ChanBase(2, 0);
	chan_k_lc->keepMode = 1;

	Mux* lc_k_mux = new Mux(lc_k_mux_trueChan, lc_k_mux_falseChan, lc_k_mux_outChan);
	lc_k_mux->addPort({ lc_k_loopVar }, { }, { lc_k_loopVar });
	//Mux* lc_k_mux = new Mux({ lc_k_loopVar }, { }, { lc_k_loopVar });
	Lc* lc_k = new Lc(lc_k_loopVar, lc_k_getEnd, lc_k_sendEnd, lc_k_mux);

	// Loop j
	ChanBase* lc_j_loopVar = new ChanBase(2, 0);
	ChanBase* lc_j_getEnd = new ChanBase(2, 0);
	ChanBase* lc_j_sendEnd = new ChanBase(2, 0);

	ChanBase* lc_j_mux_trueChan = new ChanBase(2, 0);
	ChanBase* lc_j_mux_falseChan = new ChanBase(2, 0);
	ChanBase* lc_j_mux_outChan = new ChanBase(2, 0);

	//// Inner most loop not need to declare the loop index channel below
	//ChanBase* chan_j_lc = new ChanBase(2, 0);
	//chan_j_lc->keepMode = 1;  // Inner-most loop, not in keepMode

	Mux* lc_j_mux = new Mux(lc_j_mux_trueChan, lc_j_mux_falseChan, lc_j_mux_outChan);
	lc_j_mux->addPort({ lc_j_loopVar }, { }, { lc_j_loopVar });
	//Mux* lc_j_mux = new Mux({ lc_j_loopVar }, { }, { lc_j_loopVar });
	Lc* lc_j = new Lc(lc_j_loopVar, lc_j_getEnd, lc_j_sendEnd, lc_j_mux);


	//*** Declare channel
	ChanBase* begin = new ChanBase(1, 0);
	begin->noUpstream = 1;
	ChanBase* end = new ChanBase(1, 0);
	end->noDownstream = 1;

	ChanBase* chan_m1_addr = new ChanBase(16, 4);

	ChanBase* chan_k_row = new ChanBase(16, 4);
	chan_k_row->keepMode = 1;

	ChanBase* chan_i_row = new ChanBase(16, 4);
	chan_i_row->keepMode = 1;

	ChanBase* chan_m1_getData = new ChanBase(16, 4);
	chan_m1_getData->keepMode = 1;

	ChanBase* chan_m2_addr = new ChanBase(16, 4);

	ChanBase* chan_mul = new ChanBase(16, 4);

	ChanBase* chan_partialSum_addr = new ChanBase(16, 4);
	/*ChanBase* chan_partialSum_getData = new ChanBase(16, 4);*/

	ChanBase* chan_partialSum = new ChanBase(16, 4);


	//*** Define interconnect
	// Begin & end connect to the outer most loop
	begin->addDownstream({ lc_jj_mux_falseChan });
	end->addUpstream({ lc_jj->sendEnd });

	// Loop index channel
	chan_jj_lc->addUpstream({ lc_jj->loopVar });
	chan_jj_lc->addDownstream({ lc_kk->loopVar, chan_m2_addr, chan_partialSum_addr });

	chan_kk_lc->addUpstream({ lc_kk->loopVar });
	chan_kk_lc->addDownstream({ lc_i->loopVar, chan_m1_addr, chan_k_row });

	chan_i_lc->addUpstream({ lc_i->loopVar });
	chan_i_lc->addDownstream({ lc_k->loopVar, chan_m1_addr });

	chan_k_lc->addUpstream({ lc_k->loopVar });
	chan_k_lc->addDownstream({ lc_j->loopVar/*, chan_m1_addr, chan_m2_addr */});  // Only add inner loop channels as downstream; As for same loop channel, added as loopVar's downstream

	//chan_j_lc->addUpstream({  });
	//chan_j_lc->addDownstream({  });

	chan_m1_addr->addUpstream({ chan_i_row, lc_k->loopVar, chan_kk_lc });
	chan_m1_addr->addDownstream({ lse_ld_m1 });

	lse_ld_m1->addUpstream({ chan_m1_addr });
	lse_ld_m1->addDownstream({ chan_m1_getData });

	chan_m1_getData->addUpstream({ lse_ld_m1 });
	chan_m1_getData->addDownstream({ chan_partialSum });

	chan_i_row->addUpstream({ chan_i_lc });
	chan_i_row->addDownstream({ chan_m1_addr, chan_partialSum_addr });

	chan_k_row->addUpstream({ lc_k->loopVar, chan_kk_lc });
	chan_k_row->addDownstream({ chan_m2_addr });

	chan_m2_addr->addUpstream({ chan_k_row, lc_j->loopVar, chan_jj_lc });
	chan_m2_addr->addDownstream({ lse_ld_m2 });

	lse_ld_m2->addUpstream({ chan_m2_addr });
	lse_ld_m2->addDownstream({ chan_partialSum });

	/*chan_m2_getData->addUpstream({  });
	chan_m2_getData->addDownstream({  });*/

	chan_mul->addUpstream({ lse_ld_m1, lse_ld_m2 });
	chan_mul->addDownstream({ chan_partialSum });

	chan_partialSum_addr->addUpstream({ chan_i_row, lc_j->loopVar, chan_jj_lc });
	chan_partialSum_addr->addDownstream({ lse_ld_partialSum });

	lse_ld_partialSum->addUpstream({ chan_partialSum_addr });
	lse_ld_partialSum->addDownstream({ chan_partialSum });

	chan_partialSum->addUpstream({ lse_ld_partialSum, chan_mul });
	chan_partialSum->addDownstream({ lse_st_partialSum });

	lse_st_partialSum->addUpstream({ chan_partialSum });
	lse_st_partialSum->noDownstream = 1;


	//*** LC addPort : getAct, sendAct, getEnd, sendEnd
	lc_jj->addPort({ }, { chan_jj_lc }, { lc_kk->sendEnd }, { end });
	lc_jj->addDependence({ begin }, {});  // No loop dependence

	lc_kk->addPort({ chan_jj_lc }, { chan_kk_lc }, { lc_i->sendEnd }, { lc_jj->getEnd });
	lc_kk->addDependence({}, {});  // No loop dependence

	lc_i->addPort({ chan_kk_lc }, { chan_i_lc }, { lc_k->sendEnd }, { lc_kk->getEnd });
	lc_i->addDependence({}, {});  // No loop dependence

	lc_k->addPort({ chan_i_lc }, { chan_k_lc, chan_k_row, chan_m1_addr }, { lc_j->sendEnd }, { lc_i->getEnd });
	lc_k->addDependence({}, {});  // No loop dependence

	lc_j->addPort({ chan_k_lc }, { /*chan_j_lc*/ chan_m2_addr, chan_partialSum_addr}, { chan_partialSum }, { lc_k->getEnd });
	lc_j->addDependence({}, {});  // No loop dependence


	////*** Generate gold results
	//for (size_t i = 0; i < matrix_height; ++i)
	//{
	//	for (size_t j = 0; j < matrix_width; ++j)
	//	{
	//		for (size_t k = 0; k < matrix_height; ++k)
	//		{
	//			result[i][j] += m1[i][k] * m2[k][j];
	//		}
	//	}
	//}


	//*** Simulate
	// Initiation
	begin->get(1);
	uint iter = 0;

	int jj = 0;
	int kk = 0;
	int i = 0;
	int k = 0;
	int j = 0;

	vector<int> res;  // Result
	vector<int> temp; // temp_result

	// Execute
	while (iter < 10000)
	{
		DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
		int clk = DFSim::ClkDomain::getInstance()->getClk();
		debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
		debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

		//** Loop Lc_jj
		jj = lc_jj->mux->mux(jj, 0, lc_jj->sel);
		lc_jj->mux->muxUpdate(lc_jj->sel);
		lc_jj->loopVar->get(jj);
		jj = lc_jj->loopVar->assign() + block_size;
		lc_jj->lcUpdate(jj < matrix_height);

		//begin->get(1);  // update begin
		begin->valid = 0;

		// loop interface: var jj
		int jj_ = lc_jj->loopVar->assign();
		chan_jj_lc->get(jj_);

		//** Loop Lc_kk
		kk = lc_kk->mux->mux(kk, 0, lc_kk->sel);
		lc_kk->mux->muxUpdate(lc_kk->sel);
		lc_kk->loopVar->get(kk);
		kk = lc_kk->loopVar->assign() + block_size;
		lc_kk->lcUpdate(kk < matrix_height);

		// loop interface: var kk
		int kk_ = lc_kk->loopVar->assign();
		chan_kk_lc->get(kk_);

		//** Loop Lc_i
		i = lc_i->mux->mux(i, 0, lc_i->sel);
		lc_i->mux->muxUpdate(lc_i->sel);
		lc_i->loopVar->get(i);
		i = lc_i->loopVar->assign() + 1;
		lc_i->lcUpdate(i < matrix_height);

		// loop interface: var i
		int i_ = lc_i->loopVar->assign();
		chan_i_lc->get(i_);

		//** Loop Lc_k
		k = lc_k->mux->mux(k, 0, lc_k->sel);
		lc_k->mux->muxUpdate(lc_k->sel);
		lc_k->loopVar->get(k);
		k = lc_k->loopVar->assign() + 1;
		lc_k->lcUpdate(k < block_size);

		// loop interface: var k
		int k_ = lc_k->loopVar->assign();
		chan_k_lc->get(k_);

		int i_chan = chan_i_lc->assign();
		int i_row = i_chan * matrix_height;
		chan_i_row->get(i_row);

		int k_var = lc_k->loopVar->assign();
		int kk_chan = chan_kk_lc->assign();
		int k_row = (k_var + kk_chan) * matrix_height;
		chan_k_row->get(k_row);

		int m1_addr_chan = i_row + k_var + kk_chan;
		chan_m1_addr->get(m1_addr_chan);

		uint m1_addr_lse = chan_m1_addr->assign();
		lse_ld_m1->get(false, m1_addr_lse);

		uint m1_addr_ack = lse_ld_m1->assign();
		uint m1_rowId = m1_addr_ack / matrix_width;
		uint m1_colId = m1_addr_ack % matrix_width;
		int m1_data = m1[m1_rowId][m1_colId];
		chan_m1_getData->get(m1_data);

		//** Loop Lc_j
		j = lc_j->mux->mux(j, 0, lc_j->sel);
		lc_j->mux->muxUpdate(lc_j->sel);
		lc_j->loopVar->get(j);
		j = lc_j->loopVar->assign() + 1;
		lc_j->lcUpdate(j < block_size);

		//// loop interface: var j
		//int j_ = lc_j->loopVar->assign();
		//chan_j_lc->get(j_);

		int k_row_lc_j = chan_k_row->assign();
		int j_ = lc_j->loopVar->assign();
		int jj_lc_j = chan_jj_lc->assign();

		uint m2_addr_chan = k_row_lc_j + j_ + jj_lc_j;
		chan_m2_addr->get(m2_addr_chan);

		uint m2_addr_lse = chan_m2_addr->assign();
		lse_ld_m2->get(false, m2_addr_lse);

		uint m2_addr_ack = lse_ld_m2->assign();
		uint m2_rowId = m2_addr_ack / matrix_width;
		uint m2_colId = m2_addr_ack % matrix_width;
		int m2_data = m2[m2_rowId][m2_colId];
		
		int m1_data_lc_j = chan_m1_getData->assign();
		int mul_data = m1_data_lc_j * m2_data;
		chan_mul->get(mul_data);

		int i_row_lc_j = chan_i_row->assign();
		uint partialSum_addr = i_row_lc_j + j_ + jj_lc_j;
		lse_ld_partialSum->get(false, partialSum_addr);

		uint partialSum_addr_ack = lse_ld_partialSum->assign();
		uint partialSum_rowId = partialSum_addr_ack / matrix_width;
		uint partialSum_colId = partialSum_addr_ack % matrix_width;
		int partialSum_data = m2[partialSum_rowId][partialSum_colId];

		int mul_data_chan = chan_mul->assign();
		int partialSum_data_update = partialSum_data + mul_data_chan;
		temp = chan_partialSum->get(partialSum_data_update);

		lse_st_partialSum->get(false, partialSum_addr);

		if (temp[2])
		{
			res.push_back(temp[3]);
		}

		//** MemorySystem update
		memSys->MemSystemUpdate();

		//** Print log
		debug->getFile() << std::endl;
		debug->getFile() << "Loop index jj: " << jj_ << std::endl;
		debug->getFile() << "Loop index kk: " << kk_ << std::endl;
		debug->getFile() << "Loop index i: " << i_ << std::endl;
		debug->getFile() << "Loop index k: " << k_ << std::endl;
		debug->getFile() << "Loop index j: " << j_ << std::endl;

		debug->getFile() << "Loop index jj: " << jj_lc_j << std::endl;
		debug->getFile() << "Loop index kk: " << kk_chan << std::endl;
		debug->getFile() << "Loop index i: " << i_chan << std::endl;
		debug->getFile() << "Loop index k: " << chan_k_lc->assign() << std::endl;
		debug->getFile() << "Loop index j: " << j_ << std::endl;

		debug->vecPrint("Result", res, 15);

		//debug->chanPrint("begin", begin);
		//debug->chanPrint("lc_jj->loopVar", lc_jj->loopVar);
		//debug->chanPrint("chan_jj_lc", chan_jj_lc);
		//debug->chanPrint("lc_kk->loopVar", lc_kk->loopVar);
		//debug->chanPrint("chan_kk_lc", chan_kk_lc);
		//debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
		//debug->chanPrint("chan_i_lc", chan_i_lc);
		//debug->chanPrint("lc_k->loopVar", lc_k->loopVar);
		//debug->chanPrint("chan_k_lc", chan_k_lc);
		//debug->chanPrint("lc_j->loopVar", lc_j->loopVar);
		///*debug->chanPrint("chan_j_lc", chan_j_lc);*/

		//debug->chanPrint("chan_m1_addr", chan_m1_addr);
		//debug->chanPrint("chan_k_row", chan_k_row);
		//debug->chanPrint("chan_i_row", chan_i_row);
		//debug->chanPrint("lse_ld_m1", lse_ld_m1);
		//debug->chanPrint("chan_m1_getData", chan_m1_getData);
		//debug->chanPrint("chan_m2_addr", chan_m2_addr);
		//debug->chanPrint("lse_ld_m2", lse_ld_m2);
		//debug->chanPrint("chan_mul", chan_mul);
		//debug->chanPrint("chan_partialSum_addr", chan_partialSum_addr);
		//debug->chanPrint("lse_ld_partialSum", lse_ld_partialSum);
		//debug->chanPrint("chan_partialSum", chan_partialSum);

		//debug->chanPrint("lc_j->getEnd", lc_j->getEnd);
		//debug->chanPrint("lc_j->sendEnd", lc_j->sendEnd);
		//debug->chanPrint("lc_k->getEnd", lc_k->getEnd);
		//debug->chanPrint("lc_k->sendEnd", lc_k->sendEnd);
		//debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
		//debug->chanPrint("lc_i->sendEnd", lc_i->sendEnd);
		//debug->chanPrint("lc_kk->getEnd", lc_kk->getEnd);
		//debug->chanPrint("lc_kk->sendEnd", lc_kk->sendEnd);
		//debug->chanPrint("lc_jj->getEnd", lc_jj->getEnd);
		//debug->chanPrint("lc_jj->sendEnd", lc_jj->sendEnd);

		//debug->chanPrint("end", end);

		//debug->memSysPrint(memSys);

		++iter;
	}

}