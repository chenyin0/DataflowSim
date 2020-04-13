#include "./MemSystem.h"
#include "../ClkSys.h"

using namespace DFSim;

MemSystem::MemSystem()
{
	reqQueue.resize(MEMSYS_REQ_QUEUE_SIZE);

	spm = new Spm();
	memDataBus = new MemoryDataBus();

	mem = new DRAMSim::MultiChannelMemorySystem("../DRAMSim2/ini/DDR3_micron_16M_8B_x8_sg15.ini", "../DRAMSim2/ini/system.ini", ".", "example_app", 16384);
	TransactionCompleteCB* read_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_read_complete);
	TransactionCompleteCB* write_cb = new Callback<MemoryDataBus, void, unsigned, uint64_t, uint64_t>(&(*memDataBus), &MemoryDataBus::mem_write_complete);
	//TransactionCompleteCB* read_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_read_complete);
	//TransactionCompleteCB* write_cb = new Callback<Spm, void, unsigned, uint64_t, uint64_t>(&(*spm), &Spm::mem_write_complete);
	mem->RegisterCallbacks(read_cb, write_cb, power_callback);
}

MemSystem::~MemSystem()
{
	delete mem;
	delete spm;
	delete memDataBus;

	for (auto& lse : lseRegistry)
	{
		delete lse;
	}

}

uint MemSystem::registerLse(Lse* _lse)
{
	lseRegistry.push_back(_lse);
	uint lseId = lseRegistry.size() - 1;  // Index in LseRegistry

	return lseId;
}

//bool MemSystem::checkReqQueueIsFull()
//{
//	if (reqQueue.size() < MEMSYS_REQ_QUEUE_SIZE)
//	{
//		return false;
//	}
//	else
//	{
//		return true;
//	}
//}

bool MemSystem::addTransaction(MemReq _req)
{
	//if (reqQueue.size() < MEMSYS_REQ_QUEUE_SIZE)
	//{
	//	reqQueue.push_back(_req);
	//	return true;
	//}
	//else
	//{
	//	return false;
	//}

	bool addSuccess = 0;
	for (size_t i = 0; i < reqQueue.size(); ++i)
	{
		if (!reqQueue[i].valid)
		{
			_req.memSysReqQueueIndex = i;  // Record the entry of reqQueue in memSystem
			reqQueue[i] = _req;
			addSuccess = 1;
			break;
		}
	}

	return addSuccess;
}

void MemSystem::sendBack2Lse()
{
	for (auto& req : reqQueue)
	{
		if (req.valid && req.ready)
		{
			lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.ready = 1;
			lseRegistry[req.lseId]->reqQueue[req.lseReqQueueIndex].first.inflight = 0;
			req.valid = 0;  // Clear req
		}
	}
}

void MemSystem::send2Spm()
{
	for (size_t i = 0; i < reqQueue.size(); ++i)
	{
		//sendPtr = (sendPtr + i) % reqQueue.size();  // Update sendPtr, round-robin

		MemReq& req = reqQueue[sendPtr];
		if (req.valid && !req.inflight && !req.ready)
		{
			if (spm->addTransaction(req))  // Send req to DRAM, if send failed -> break;
			{
				req.inflight = 1;
			}
			else
			{
				break;
			}
		}

		sendPtr = (++sendPtr) % reqQueue.size();  // Update sendPtr, round-robin
	}
}

void MemSystem::getFromSpm()
{
	vector<MemReq> _req = spm->callBack();
	for (auto req : _req)
	{
		reqQueue[req.memSysReqQueueIndex].ready = 1;
		reqQueue[req.memSysReqQueueIndex].inflight = 0;
	}
}

void MemSystem::getReqAckFromMemoryDataBus(vector<MemReq> _reqAcks)
{
	for (auto& reqAck : _reqAcks)
	{
		reqAckStack.push_back(reqAck);
	}
}

void MemSystem::returnReqAck()
{
	// Send reqAck back to SPM
	for (auto& reqAck : reqAckStack)
	{
		spm->mem_req_complete(reqAck);
	}

	reqAckStack.clear();
}

void MemSystem::MemSystemUpdate()
{
	send2Spm();  // Send req to SPM
	spm->spmUpdate();
	spm->sendReq(mem);

	if (ClkDomain::getInstance()->checkClkAdd())  // Update DRAM only when system clk update (Synchronize clk domain, in DGSF speedup mode)
	{
		mem->update();
		getReqAckFromMemoryDataBus(memDataBus->MemoryDataBusUpdate());
	}

	returnReqAck();
	getFromSpm();  // Get callback from SPM
	sendBack2Lse();
}


#ifdef DEBUG_MODE
// For Debug
const vector<Lse*>& MemSystem::getLseRegistry() const
{
	return lseRegistry;
}

const vector<MemReq>& MemSystem::getReqQueue() const
{
	return reqQueue;
}
#endif


// MemoryDataBus
MemoryDataBus::MemoryDataBus()
{
}

void MemoryDataBus::mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
{
	MemReq req;
	req.valid = 1;
	req.isWrite = false;
	req.addr = _addr;

	busDelayFifo.push_back(make_pair(req, busDelay));
}

void MemoryDataBus::mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
{
	MemReq req;
	req.valid = 1;
	req.isWrite = true;
	req.addr = _addr;

	busDelayFifo.push_back(make_pair(req, busDelay));
}

vector<MemReq> MemoryDataBus::MemoryDataBusUpdate()
{
	// Emulate bus delay
	for (auto& req : busDelayFifo)
	{
		if (req.second > 0)
		{
			--req.second; 
		}
	}

	// Send ready req from memoryDataBus to reqStack in memSystem
	vector<MemReq> _reqAckStack;
	bool traverse = 1;
	while (traverse)
	{
		if (!busDelayFifo.empty() && busDelayFifo.front().second == 0)
		{
			_reqAckStack.push_back(busDelayFifo.front().first);
			busDelayFifo.pop_front();
		}
		else
		{
			traverse = 0;
		}
	}

	return _reqAckStack;
}

//vector<MemReq> MemoryDataBus::getFromBusDelayFifo()
//{
//	vector<MemReq> reqStack;
//	bool traverse = 1;
//	while (traverse)
//	{
//		if (!busDelayFifo.empty() && busDelayFifo.front().second == 0)
//		{
//			reqStack.push_back(busDelayFifo.front().first);
//			busDelayFifo.pop_front();
//		}
//		else
//		{
//			traverse = 0;
//		}
//	}
//
//	return reqStack;
//}


#ifdef DEBUG_MODE
const deque<pair<MemReq, uint>>& MemoryDataBus::getBusDelayFifo() const
{
	return busDelayFifo;
}
#endif // DEBUG_MODE
