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
				i_row = i * row_size;  // Chan_i_row
				loopk:for (k = 0; k < block_size; ++k){
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

/*
Config tips:

1. 如果channel的来源是外层循环变量，则upstream来自外层的keepMode的loopIndex channel；
	如果channel的来源是同层循环变量，则upstream来自同层的loopVar

2. 即使channel的数据来源不是同层的循环变量，upstream中也要添加当前循环的loopVar，用来接收当前循环产生的last_tag

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

const uint GemmTest::matrix_width = 50;
const uint GemmTest::matrix_height = matrix_width;
const uint GemmTest::block_size = 10;

vector<vector<int>> GemmTest::m1;
vector<vector<int>> GemmTest::m2;
vector<vector<int>> GemmTest::result;

void GemmTest::gemm_base(Debug* debug)
{
	// Generate benchmark data
	generateData();


	//******  Define module  ******//
	//*** Declare registry
	Registry* registry = new Registry();

	//*** Declare memory
	MemSystem* memSys = new MemSystem();


	//*** Declare Lse
	Lse* lse_ld_m1 = new Lse(16, 0, false, memSys);  // Load M1
	Lse* lse_ld_m2 = new Lse(16, 0, false, memSys);  // Load M2
	Lse* lse_ld_partialSum = new Lse(16, 0, false, memSys);  // load partial sum
	Lse* lse_st_partialSum = new Lse(16, 0, true, memSys);  // Store back partial sum
	lse_st_partialSum->noDownstream = 1;


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

	ChanBase* chan_k_lc = new ChanBase(20, 0);
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

	// loop kk
	ChanBase* chan_jj_relay_loop_kk = new ChanBase(16, 4);  // Relay channel in loop kk for chan_jj_lc
	chan_jj_relay_loop_kk->keepMode = 1;

	// loop i
	ChanBase* chan_i_row = new ChanBase(16, 4);
	chan_i_row->keepMode = 1;

	ChanBase* chan_jj_relay_loop_i = new ChanBase(16, 4);  // Relay channel in loop i for chan_jj_lc
	chan_jj_relay_loop_i->keepMode = 1;

	ChanBase* chan_kk_relay_loop_i = new ChanBase(16, 4);  // Relay channel in loop i for chan_kk_lc
	chan_kk_relay_loop_i->keepMode = 1;

	// loop k
	ChanBase* chan_m1_addr = new ChanBase(16, 4);

	ChanBase* chan_k_row = new ChanBase(16, 4);
	chan_k_row->keepMode = 1;

	ChanBase* chan_m1_getData = new ChanBase(16, 4);
	chan_m1_getData->keepMode = 1;

	ChanBase* chan_jj_relay_loop_k = new ChanBase(16, 4);  // Relay channel in loop k for chan_jj_lc
	chan_jj_relay_loop_k->keepMode = 1;

	// loop j
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
	chan_jj_lc->addDownstream({ lc_kk->loopVar, /*chan_m2_addr, chan_partialSum_addr*/chan_jj_relay_loop_kk });

	chan_kk_lc->addUpstream({ lc_kk->loopVar });
	chan_kk_lc->addDownstream({ lc_i->loopVar, /*chan_m1_addr, chan_k_row*/chan_kk_relay_loop_i });

	chan_i_lc->addUpstream({ lc_i->loopVar });
	chan_i_lc->addDownstream({ lc_k->loopVar /*chan_m1_addr*/ /*chan_i_row*/});

	chan_k_lc->addUpstream({ lc_k->loopVar });
	chan_k_lc->addDownstream({ lc_j->loopVar/*, chan_m1_addr, chan_m2_addr */});  // Only add inner loop channels as downstream; As for same loop channel, added as loopVar's downstream

	//chan_j_lc->addUpstream({  });
	//chan_j_lc->addDownstream({  });

	// loop kk
	chan_jj_relay_loop_kk->addUpstream({ chan_jj_lc, lc_kk->loopVar });
	chan_jj_relay_loop_kk->addDownstream({ chan_jj_relay_loop_i });

	// loop i
	chan_i_row->addUpstream({ lc_i->loopVar/*chan_i_lc*//*, lc_k->loopVar*/ });
	chan_i_row->addDownstream({ chan_m1_addr, chan_partialSum_addr });

	chan_kk_relay_loop_i->addUpstream({ chan_kk_lc, lc_i->loopVar });
	chan_kk_relay_loop_i->addDownstream({ chan_m1_addr, chan_k_row });

	chan_jj_relay_loop_i->addUpstream({ chan_jj_relay_loop_kk, lc_i->loopVar });
	chan_jj_relay_loop_i->addDownstream({ chan_jj_relay_loop_k });

	// loop k
	chan_m1_addr->addUpstream({ chan_i_row/*chan_i_lc*/, lc_k->loopVar, /*chan_kk_lc*/chan_kk_relay_loop_i });
	chan_m1_addr->addDownstream({ lse_ld_m1 });

	lse_ld_m1->addUpstream({ chan_m1_addr });
	lse_ld_m1->addDownstream({ chan_m1_getData });

	chan_m1_getData->addUpstream({ lse_ld_m1 });
	chan_m1_getData->addDownstream({ chan_mul });

	chan_k_row->addUpstream({ lc_k->loopVar, /*chan_kk_lc*/chan_kk_relay_loop_i });
	chan_k_row->addDownstream({ chan_m2_addr });

	chan_jj_relay_loop_k->addUpstream({ chan_jj_relay_loop_i, lc_k->loopVar });
	chan_jj_relay_loop_k->addDownstream({ chan_m2_addr, chan_partialSum_addr });

	// loop j
	chan_m2_addr->addUpstream({ chan_k_row, lc_j->loopVar, /*chan_jj_lc*/chan_jj_relay_loop_k });
	chan_m2_addr->addDownstream({ lse_ld_m2 });

	lse_ld_m2->addUpstream({ chan_m2_addr });
	lse_ld_m2->addDownstream({ chan_mul });

	/*chan_m2_getData->addUpstream({  });
	chan_m2_getData->addDownstream({  });*/

	chan_mul->addUpstream({ chan_m1_getData, lse_ld_m2/*, lc_j->loopVar*/ });
	chan_mul->addDownstream({ chan_partialSum });

	chan_partialSum_addr->addUpstream({ chan_i_row, lc_j->loopVar, /*chan_jj_lc*/chan_jj_relay_loop_k });
	chan_partialSum_addr->addDownstream({ lse_ld_partialSum });

	lse_ld_partialSum->addUpstream({ chan_partialSum_addr });
	lse_ld_partialSum->addDownstream({ chan_partialSum });

	chan_partialSum->addUpstream({ lse_ld_partialSum, chan_mul });
	chan_partialSum->addDownstream({ lse_st_partialSum, lc_j->getEnd });

	lse_st_partialSum->addUpstream({ chan_partialSum });
	lse_st_partialSum->noDownstream = 1;


	//*** LC addPort : getAct, sendAct, getEnd, sendEnd
	lc_jj->addPort({ }, { chan_jj_lc }, { lc_kk->sendEnd }, { end });
	lc_jj->addDependence({ begin }, {});  // No loop dependence

	lc_kk->addPort({ chan_jj_lc }, { chan_kk_lc, chan_jj_relay_loop_kk }, { lc_i->sendEnd }, { lc_jj->getEnd });
	lc_kk->addDependence({}, {});  // No loop dependence

	lc_i->addPort({ chan_kk_lc }, { chan_i_lc, chan_i_row, chan_kk_relay_loop_i, chan_jj_relay_loop_i }, { lc_k->sendEnd }, { lc_kk->getEnd });
	lc_i->addDependence({}, {});  // No loop dependence

	lc_k->addPort({ chan_i_lc }, { chan_k_lc, /*chan_i_row,*/ chan_k_row, chan_m1_addr, chan_jj_relay_loop_k }, { lc_j->sendEnd }, { lc_i->getEnd });
	lc_k->addDependence({}, {});  // No loop dependence

	lc_j->addPort({ chan_k_lc }, { /*chan_j_lc*/ chan_m2_addr, /*chan_mul, */chan_partialSum_addr}, { chan_partialSum }, { lc_k->getEnd });
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
	registry->tableInit();  // Update registry and initial all the module in registry

	begin->get({ 1 });
	uint iter = 0;

	int jj = 0;
	int kk = 0;
	int i = 0;
	int k = 0;
	int j = 0;

	vector<int> res;  // Result
	vector<int> temp; // temp_result

	uint max_iter = 5000000;
	uint segment = max_iter / 100;
	uint percent = 0;
	// Execute
	while (iter < max_iter)
	{
		// Print progress bar
		if (iter / segment > percent)
		{
			percent = iter / segment;
			std::cout << ">>>>>> progress: " << percent << "%" << std::endl;
		}

		DFSim::ClkDomain::getInstance()->clkUpdate(); // update clk in each loop
		int clk = DFSim::ClkDomain::getInstance()->getClk();
		debug->getFile() << "\n" << "**************** " << "Exe:" << iter << "  ";
		debug->getFile() << " Clk:" << clk << " ********************" << std::endl;

		//** Loop Lc_jj
		jj = lc_jj->mux->mux(jj, 0, lc_jj->sel);
		lc_jj->mux->muxUpdate(lc_jj->sel);
		lc_jj->mux->outChan->value = jj;
		lc_jj->loopVar->get();
		lc_jj->loopVar->value = lc_jj->loopVar->assign(lc_jj->mux->outChan);  // After get(), must update chan's value
		jj = lc_jj->loopVar->value + block_size;
		lc_jj->lcUpdate(jj < matrix_height);

		//begin->get(1);  // update begin
		begin->valid = 0;

		// loop interface: var jj
		//int jj_ = lc_jj->loopVar->assign(lc_jj->mux->outChan);
		chan_jj_lc->get();
		chan_jj_lc->value = chan_jj_lc->assign(lc_jj->loopVar);

		//** Loop Lc_kk
		kk = lc_kk->mux->mux(kk, 0, lc_kk->sel);
		lc_kk->mux->muxUpdate(lc_kk->sel);
		lc_kk->mux->outChan->value = kk;
		lc_kk->loopVar->get();
		lc_kk->loopVar->value = lc_kk->loopVar->assign(lc_kk->mux->outChan);
		kk = lc_kk->loopVar->assign(lc_kk->mux->outChan) + block_size;
		lc_kk->lcUpdate(kk < matrix_height);

		// loop interface: var kk
		//int kk_ = lc_kk->loopVar->assign(lc_kk->mux->outChan);
		chan_kk_lc->get();
		chan_kk_lc->value = chan_kk_lc->assign(lc_kk->loopVar);

		//int jj_relay_lp_kk = chan_jj_lc->assign(lc_jj->loopVar);
		chan_jj_relay_loop_kk->get();
		chan_jj_relay_loop_kk->value = chan_jj_relay_loop_kk->assign(chan_jj_lc);

		//** Loop Lc_i
		i = lc_i->mux->mux(i, 0, lc_i->sel);
		lc_i->mux->muxUpdate(lc_i->sel);
		lc_i->mux->outChan->value = i;
		lc_i->loopVar->get();
		lc_i->loopVar->value = lc_i->loopVar->assign(lc_i->mux->outChan);
		i = lc_i->loopVar->assign(lc_i->mux->outChan) + 1;
		lc_i->lcUpdate(i < matrix_height);

		// loop interface: var i
		//int i_ = lc_i->loopVar->assign(lc_i->mux->outChan);
		chan_i_lc->get();
		chan_i_lc->value = chan_i_lc->assign(lc_i->loopVar);

		//int i_chan = lc_i->loopVar->assign(lc_i->loopVar);
		//int i_row = i_chan * matrix_height;
		chan_i_row->get();
		chan_i_row->value = chan_i_row->assign(lc_i->loopVar) * matrix_height;

		//int kk_relay_lp_i = chan_kk_lc->assign(lc_kk->loopVar);
		chan_kk_relay_loop_i->get();
		chan_kk_relay_loop_i->value = chan_kk_relay_loop_i->assign(chan_kk_lc);

		//int jj_relay_lp_i = chan_jj_relay_loop_kk->assign(chan_jj_lc);
		chan_jj_relay_loop_i->get();
		chan_jj_relay_loop_i->value = chan_jj_relay_loop_i->assign(chan_jj_relay_loop_kk);

		//** Loop Lc_k
		k = lc_k->mux->mux(k, 0, lc_k->sel);
		lc_k->mux->muxUpdate(lc_k->sel);
		lc_k->mux->outChan->value = k;
		lc_k->loopVar->get();
		lc_k->loopVar->value = lc_k->loopVar->assign(lc_k->mux->outChan);
		k = lc_k->loopVar->assign(lc_k->mux->outChan) + 1;
		lc_k->lcUpdate(k < block_size);

		// loop interface: var k
		//int k_ = lc_k->loopVar->assign(lc_k->mux->outChan);
		chan_k_lc->get();
		chan_k_lc->value = chan_k_lc->assign(lc_k->loopVar);

		//int k_var = lc_k->loopVar->assign(lc_k->mux->outChan);
		//int kk_chan_relay_lp_i = chan_kk_relay_loop_i->assign(chan_kk_lc);
		//int k_row = (k_var + kk_chan_relay_lp_i) * matrix_height;
		chan_k_row->get();
		chan_k_row->value = chan_k_row->assign(lc_k->loopVar) + chan_k_row->assign(chan_kk_relay_loop_i);
		//int k_row = (chan_k_row->assign(lc_k->loopVar) + chan_k_row->assign(chan_kk_relay_loop_i)) * matrix_height;

		//int m1_addr_chan = i_row + k_var + kk_chan_relay_lp_i;
		chan_m1_addr->get();
		chan_m1_addr->value = chan_m1_addr->assign(chan_i_row) + chan_m1_addr->assign(lc_k->loopVar) + chan_m1_addr->assign(chan_kk_relay_loop_i);

		//int m1_addr_chan = chan_m1_addr->assign(chan_i_row) + chan_m1_addr->assign(lc_k->loopVar) + chan_m1_addr->assign(chan_kk_relay_loop_i);

		//uint m1_addr_lse = chan_m1_addr->assign();
		lse_ld_m1->get();
		lse_ld_m1->value = lse_ld_m1->assign();

		//uint m1_addr_ack = lse_ld_m1->assign();
		chan_m1_getData->get();
		uint m1_rowId = chan_m1_getData->assign(lse_ld_m1) / matrix_width;
		uint m1_colId = chan_m1_getData->assign(lse_ld_m1) % matrix_width;
		int m1_data = m1[m1_rowId][m1_colId];
		chan_m1_getData->value = m1_data;

		/*uint m1_rowId = chan_m1_getData->assign(lse_ld_m1) / matrix_width;
		uint m1_colId = chan_m1_getData->assign(lse_ld_m1) % matrix_width;
		int m1_data = m1[m1_rowId][m1_colId];*/
		
		//int jj_relay_loop_k = chan_jj_relay_loop_i->assign(chan_jj_relay_loop_kk);
		chan_jj_relay_loop_k->get();
		chan_jj_relay_loop_k->value = chan_jj_relay_loop_k->assign(chan_jj_relay_loop_i);

		//** Loop Lc_j
		j = lc_j->mux->mux(j, 0, lc_j->sel);
		lc_j->mux->muxUpdate(lc_j->sel);
		lc_j->mux->outChan->value = j;
		lc_j->loopVar->get();
		lc_j->loopVar->value = lc_j->loopVar->assign(lc_j->mux->outChan);
		j = lc_j->loopVar->assign(lc_j->mux->outChan) + 1;
		lc_j->lcUpdate(j < block_size);

		//// loop interface: var j
		//int j_ = lc_j->loopVar->assign();
		//chan_j_lc->get(j_);

		//int k_row_lc_j = chan_k_row->assign();
		//int j_ = lc_j->loopVar->assign();
		//int jj_chan_relay_lp_j = chan_jj_relay_loop_k->assign();

		//uint m2_addr_chan = k_row_lc_j + j_ + jj_chan_relay_lp_j;

		chan_m2_addr->get();
		chan_m2_addr->value = chan_m2_addr->assign(chan_k_row) + chan_m2_addr->assign(lc_j->loopVar) + chan_m2_addr->assign(chan_jj_relay_loop_k);

		//uint m2_addr_lse = chan_m2_addr->assign();
		lse_ld_m2->get();
		lse_ld_m2->value = lse_ld_m2->assign();

		//uint m2_addr_ack = lse_ld_m2->assign();
		//uint m2_rowId = m2_addr_ack / matrix_width;
		//uint m2_colId = m2_addr_ack % matrix_width;
		//int m2_data = m2[m2_rowId][m2_colId];
		//
		//int m1_data_lc_j = chan_m1_getData->assign();
		//int mul_data = m1_data_lc_j * m2_data;

		chan_mul->get();
		uint m2_addr_ack = chan_mul->assign(lse_ld_m2);
		uint m2_rowId = m2_addr_ack / matrix_width;
		uint m2_colId = m2_addr_ack % matrix_width;
		int m2_data = m2[m2_rowId][m2_colId];
		chan_mul->value = chan_mul->assign(chan_m1_getData) * m2_data;

		//int i_row_lc_j = chan_i_row->assign();
		//uint partialSum_addr = i_row_lc_j + j_ + jj_chan_relay_lp_j;

		chan_partialSum_addr->get();
		chan_partialSum_addr->value = chan_partialSum_addr->assign(chan_i_row) + chan_partialSum_addr->assign(lc_j->loopVar) + chan_partialSum_addr->assign(chan_jj_relay_loop_k);

		//uint partialSum_addr_chan = chan_partialSum_addr->assign();
		lse_ld_partialSum->get();
		lse_ld_partialSum->value = lse_ld_partialSum->assign();

		//uint partialSum_addr_ack = lse_ld_partialSum->assign();
		//uint partialSum_rowId = partialSum_addr_ack / matrix_width;
		//uint partialSum_colId = partialSum_addr_ack % matrix_width;
		//int partialSum_data = m2[partialSum_rowId][partialSum_colId];

		//int mul_data_chan = chan_mul->assign();
		//int partialSum_data_update = partialSum_data + mul_data_chan;

		chan_partialSum->get();
		uint partialSum_addr_ack = chan_partialSum->assign(lse_ld_partialSum);
		uint partialSum_rowId = partialSum_addr_ack / matrix_width;
		uint partialSum_colId = partialSum_addr_ack % matrix_width;
		int partialSum_data = m2[partialSum_rowId][partialSum_colId];
		chan_partialSum->value = chan_partialSum->assign(chan_mul) + partialSum_data;

		temp = lse_st_partialSum->get();

		if (temp[0])
		{
			res.push_back(chan_partialSum->value);
		}

		//** MemorySystem update
		memSys->MemSystemUpdate();

		end->get();

		//** Print log
		// Set debug mode
		//debug->debug_mode = Debug_mode::Print_detail;
		debug->debug_mode = Debug_mode::Turn_off;

		if (iter >= 0)
		{
			//debug->getFile() << std::endl;
			//debug->getFile() << "Loop index jj: " << jj_ << std::endl;
			//debug->getFile() << "Loop index kk: " << kk_ << std::endl;
			//debug->getFile() << "Loop index i: " << i_ << std::endl;
			//debug->getFile() << "Loop index k: " << k_ << std::endl;
			//debug->getFile() << "Loop index j: " << j_ << std::endl;

			debug->getFile() << std::endl;
			debug->getFile() << "Loop index jj: " << chan_jj_relay_loop_k->value << std::endl;  // Inner most relay channel
			debug->getFile() << "Loop index kk: " << chan_kk_relay_loop_i->value << std::endl;  // Inner most relay channel
			debug->getFile() << "Loop index i: " << chan_i_lc->value << std::endl;
			debug->getFile() << "Loop index k: " << chan_k_lc->value << std::endl;
			debug->getFile() << "Loop index j: " << lc_j->loopVar->value << std::endl;

			debug->vecPrint("Result", res, 15);

			debug->chanPrint("begin", begin);
		/*	debug->chanPrint("lc_jj->loopVar", lc_jj->loopVar);
			debug->chanPrint("chan_jj_lc", chan_jj_lc);
			debug->chanPrint("lc_kk->loopVar", lc_kk->loopVar);
			debug->chanPrint("chan_kk_lc", chan_kk_lc);
			debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
			debug->chanPrint("chan_i_lc", chan_i_lc);
			debug->chanPrint("lc_k->loopVar", lc_k->loopVar);
			debug->chanPrint("chan_k_lc", chan_k_lc);
			debug->chanPrint("lc_j->loopVar", lc_j->loopVar);*/

			// loop jj
			debug->getFile() << std::endl;
			debug->getFile() << "*****************  jj  *****************" << std::endl;
			debug->chanPrint("lc_jj->loopVar", lc_jj->loopVar);
			debug->chanPrint("chan_jj_lc", chan_jj_lc);
			// loop kk
			debug->getFile() << std::endl;
			debug->getFile() << "*****************  kk  *****************" << std::endl;
			debug->chanPrint("lc_kk->loopVar", lc_kk->loopVar);
			debug->chanPrint("chan_kk_lc", chan_kk_lc);
			debug->chanPrint("chan_jj_relay_loop_kk", chan_jj_relay_loop_kk);
			// loop i
			debug->getFile() << std::endl;
			debug->getFile() << "*****************  i  *****************" << std::endl;
			debug->chanPrint("lc_i->loopVar", lc_i->loopVar);
			debug->chanPrint("chan_i_lc", chan_i_lc);
			debug->chanPrint("chan_i_row", chan_i_row);
			debug->chanPrint("chan_jj_relay_loop_i", chan_jj_relay_loop_i);
			debug->chanPrint("chan_kk_relay_loop_i", chan_kk_relay_loop_i);
			// loop k
			debug->getFile() << std::endl;
			debug->getFile() << "*****************  k  *****************" << std::endl;
			debug->chanPrint("lc_k->loopVar", lc_k->loopVar);
			debug->chanPrint("chan_k_lc", chan_k_lc);
			debug->chanPrint("chan_m1_addr", chan_m1_addr);
			debug->chanPrint("chan_k_row", chan_k_row);
			debug->lsePrint("lse_ld_m1", lse_ld_m1);
			debug->chanPrint("chan_m1_getData", chan_m1_getData);
			debug->chanPrint("chan_jj_relay_loop_k", chan_jj_relay_loop_k);
			// loop j
			debug->getFile() << std::endl;
			debug->getFile() << "*****************  j  *****************" << std::endl;
			debug->chanPrint("lc_j->loopVar", lc_j->loopVar);
			debug->chanPrint("chan_m2_addr", chan_m2_addr);
			debug->lsePrint("lse_ld_m2", lse_ld_m2);
			debug->chanPrint("chan_mul", chan_mul);
			debug->chanPrint("chan_partialSum_addr", chan_partialSum_addr);
			debug->lsePrint("lse_ld_partialSum", lse_ld_partialSum);
			debug->chanPrint("chan_partialSum", chan_partialSum);
			debug->lsePrint("lse_st_partialSum", lse_st_partialSum);

			debug->getFile() << std::endl;
			debug->getFile() << "*****************  End signal  *****************" << std::endl;

			debug->chanPrint("lc_jj->getEnd", lc_jj->getEnd);
			debug->getFile() << "lc_jj loopEnd: " << lc_jj->loopEnd << std::endl;
			//debug->chanPrint("lc_jj->sendEnd", lc_jj->sendEnd);

			debug->chanPrint("lc_kk->getEnd", lc_kk->getEnd);
			debug->getFile() << "lc_kk loopEnd: " << lc_kk->loopEnd << std::endl;
			//debug->chanPrint("lc_kk->sendEnd", lc_kk->sendEnd);

			debug->chanPrint("lc_i->getEnd", lc_i->getEnd);
			debug->getFile() << "lc_i loopEnd: " << lc_i->loopEnd << std::endl;
			//debug->chanPrint("lc_i->sendEnd", lc_i->sendEnd);

			debug->chanPrint("lc_k->getEnd", lc_k->getEnd);
			debug->getFile() << "lc_k loopEnd: " << lc_k->loopEnd << std::endl;
			//debug->chanPrint("lc_k->sendEnd", lc_k->sendEnd);

			debug->chanPrint("lc_j->getEnd", lc_j->getEnd);
			debug->getFile() << "lc_j loopEnd: " << lc_j->loopEnd << std::endl;
			//debug->chanPrint("lc_j->sendEnd", lc_j->sendEnd);

			debug->chanPrint("end", end);

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
		}

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

	delete registry;  // All the Module pointers have been deleted when destruct registry
	delete memSys;

}