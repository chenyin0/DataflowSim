#pragma once
#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "../DataType.h"
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"

namespace DFSim
{
	class Spm
	{
	public:
		Spm();

		// Inferface func for MemSystem
		bool addTransaction(MemReq _req);  // Add transaction to SPM
		vector<MemReq> callBack();  // Get memory access results from SPM
		
		// Interface func with DRAM
		void sendReq(DRAMSim::MultiChannelMemorySystem* mem);  // Send memory req to DRAM
		/*void mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk);
		void mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk);*/
		void mem_req_complete(MemReq _req);

		void spmUpdate();

#ifdef DEBUG_MODE  // Get private instance for debug
	public:
		const vector<MemReq>& getReqQueue() const;
#endif // DEBUG_MODE 

	private:
		uint sendPtr = 0;
		vector<MemReq> reqQueue;
		uint bankNum = SPM_BANK_NUM;
		uint bankDepth = SPM_BANK_DEPTH;
	};
}