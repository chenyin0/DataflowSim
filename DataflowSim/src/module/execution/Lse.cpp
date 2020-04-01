#include "./Lse.h"
#include "../ClkSys.h"

using namespace DFSim;

Lse::Lse(uint _size, uint _cycle, MemSystem* _memSys) : 
	Channel(_size, _cycle), memSys(_memSys)
{
	reqQueue.resize(_size);

	lseId = _memSys->registerLse(this);  // Register Lse in MemSystem and return lseId
}

Lse::~Lse()
{
	delete memSys;
}

void Lse::get(bool _isWrite, uint _addr)
{
	checkConnect();
	pop();
	push(_isWrite, _addr);
	statusUpdate();
}

void Lse::pop()
{
	bool popReady = valid;

	if (popReady)
	{
		uint reqQueueIndex = channel.front().lseReqQueueIndex;
		lastPopVal = reqQueue[reqQueueIndex].first.addr;
		channel.pop_front();  // Pop channel
		reqQueue[reqQueueIndex].first.valid = 0;  // Pop reqQueue
		reqQueue[reqQueueIndex].second.valid = 0;
	}
}

bool Lse::checkUpstream()
{
	bool ready = 1;
	if (!noUpstream)
	{
		for (auto channel : upstream)
		{
			if (!channel->valid)
			{
				ready = 0;
				break;
			}
		}
	}
	else
	{
		if (reqQueue.size() == size)  // When reqQueue is full
			ready = 0;
	}

	return ready;
}

void Lse::pushReqQ(bool _isWrite, uint _addr)
{
	//uint clk = ClkDomain::getInstance()->getClk();
	Data data = upstream.front()->channel.front();  // Inherit cond status
	data.valid = 1;
	data.last = 0;  // Reset last flag
	data.cycle = data.cycle + cycle;
	/*uint cycleTemp = data.cycle + cycle;
	data.cycle = cycleTemp > clk ? cycleTemp : clk;*/

	// Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
	for (auto channel : upstream)
	{
		data.last |= channel->channel.front().last;
		data.lastOuter |= channel->channel.front().lastOuter;
		data.graphSwitch |= channel->channel.front().graphSwitch;
	}

	MemReq req;
	req.valid = 1;
	req.addr = _addr;
	req.isWrite = _isWrite;
	req.lseId = lseId;

	for (size_t i = 0; i < reqQueue.size(); ++i)
	{
		if (!reqQueue[i].first.valid)
		{
			req.lseReqQueueIndex = i;  // Record reqQueue Id, for callback from memory
			data.lseReqQueueIndex = i;  // Record reqQueue Id, for delete from channel
			reqQueue[i].first = req;
			reqQueue[i].second = data;
			break;
		}
	}
}

void Lse::push(bool _isWrite, uint _addr)
{
	// Push data in channel
	if (checkUpstream())
	{
		pushReqQ(_isWrite, _addr);
	}
}

bool Lse::sendReq(MemReq _req)
{
	if (memSys->addTransaction(_req))
	{
		uint index = _req.lseReqQueueIndex;
		reqQueue[index].first.inflight = 1;  // Req has been sent to memory

		return true;
	}
	else
	{
		return false;
	}
}

void Lse::statusUpdate()
{
	uint clk = ClkDomain::getInstance()->getClk();
	valid = 1;

	// Send req to memSystem
	for (size_t i = 0; i < reqQueue.size(); ++i)
	{
		MemReq& req = reqQueue[sendPtr].first;
		if (req.valid && !req.inflight && !req.ready)
		{
			if (reqQueue[sendPtr].second.cycle < clk)  // Satisfy clk restriction
			{
				if (!sendReq(req))  // Send req to MemSystem, if send failed (reqQueue is full) -> break;
				{
					sendPtr = (++sendPtr) % reqQueue.size();  // Update sendPtr, round-robin
					break;
				}
			}
		}

		sendPtr = (++sendPtr) % reqQueue.size();  // Update sendPtr, round-robin
	}

	// Send already data to channel
	for (auto& req : reqQueue)
	{
		if (req.first.valid && req.first.ready && !req.first.hasPushChan)
		{
			req.second.cycle = clk;  // Update cycle
			channel.push_back(req.second);
			req.first.hasPushChan = 1;
		}
	}

	if (!channel.empty())
	{
		Data data = channel.front();

		// Check sendable
		if (!noDownstream)
		{
			for (auto& channel : downstream)
			{
				if (!channel->checkSend(data, this))
				{
					valid = 0;
					break;
				}
			}
		}
	}
	else
	{
		valid = 0;
	}

	bpUpdate();
}

bool Lse::checkSend(Data _data, Channel* upstream)
{
	bool sendable = 1;
	if (bp)
	{
		sendable = 0;
	}
	return sendable;
}

void Lse::bpUpdate()
{
	/*if (reqQueue.size() < size)
		bp = 0;
	else
		bp = 1;*/

	bp = 1;
	for (auto& req : reqQueue)
	{
		if (!req.first.valid)
		{
			bp = 0;
			break;
		}
	}
}

uint Lse::assign()
{
	if (!this->channel.empty())
	{
		uint index = channel.front().lseReqQueueIndex;
		return reqQueue[index].first.addr;
	}
	else
		return lastPopVal;
}

void Lse::callback(MemReq _req)
{
	uint index = _req.lseReqQueueIndex;
	reqQueue[index].first.ready = 1;
	reqQueue[index].first.inflight = 0;
}