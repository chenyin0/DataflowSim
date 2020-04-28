#include "./Spm.h"
#include "../ClkSys.h"

using namespace DFSim;

Spm::Spm()
{
	reqQueue.resize(SPM_REQ_QUEUE_SIZE);
}

bool Spm::addTransaction(MemReq _req)
{
	//if (reqQueue.size() < SPM_REQ_QUEUE_SIZE)
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
		if (!reqQueue[i].first.valid)
		{
			_req.spmReqQueueIndex = i;  // Record the entry of reqQueue in SPM
			reqQueue[i].first = _req;
			reqQueue[i].second = SPM_ACCESS_LATENCY;  // Emulate SPM access latency
			addSuccess = 1;
			break;
		}
	}

	return addSuccess;
}

vector<MemReq> Spm::callBack()
{
	vector<MemReq> readyReq;

	for (auto& req : reqQueue)
	{
		if (req.first.valid && req.first.ready)
		{
			readyReq.push_back(req.first);
			req.first.valid = 0;  // Clear req after being sent back
		}
	}

	return readyReq;
}

void Spm::sendReq(DRAMSim::MultiChannelMemorySystem* mem)
{
	for (size_t i = 0; i < reqQueue.size(); ++i)
	{
		//sendPtr = (sendPtr + i) % reqQueue.size();  // Update sendPtr, round-robin

		MemReq& req = reqQueue[sendPtr].first;
		bool latency = reqQueue[sendPtr].second;  // Emulate SPM access latency
		if (req.valid && !req.inflight && !req.ready && !latency)
		{
			if (mem->addTransaction(req.isWrite, req.addr))  // Send req to DRAM, if send failed -> break;
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

void Spm::reqQueueUpdate()
{
	if (ClkDomain::getInstance()->checkClkAdd())  // Update SPM latency only when system clk update
	{
		for (auto& req : reqQueue)
		{
			// Emulate SPM access latency
			if (req.second != 0)
			{
				--req.second;
			}
		}
	}
}

void Spm::spmUpdate()
{
	// Undefined
	reqQueueUpdate();
}

//void Spm::mem_read_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
//{
//	for (auto& req : reqQueue)
//	{
//		if (req.valid && !req.ready && req.addr == _addr && req.isWrite == false)  // May exist WAR, WAW, RAW contention
//		{
//			req.ready = 1;
//			req.inflight = 0;
//			break;
//		}
//	}
//}
//
//void Spm::mem_write_complete(unsigned _id, uint64_t _addr, uint64_t _clk)
//{
//	for (auto& req : reqQueue)
//	{
//		if (req.valid && !req.ready && req.addr == _addr && req.isWrite == true)  // May exist WAR, WAW, RAW contention
//		{
//			req.ready = 1;
//			req.inflight = 0;
//			break;
//		}
//	}
//}

void Spm::mem_req_complete(MemReq _req)
{
	for (auto& req : reqQueue)
	{
		if (req.first.valid && !req.first.ready && req.first.addr == _req.addr && req.first.isWrite == _req.isWrite)  // May exist WAR, WAW, RAW contention
		{
			req.first.ready = 1;
			req.first.inflight = 0;
			break;
		}
	}
}


// For debug
#ifdef DEBUG_MODE
const vector<pair<MemReq, uint>>& Spm::getReqQueue() const
{
	return reqQueue;
}

#endif