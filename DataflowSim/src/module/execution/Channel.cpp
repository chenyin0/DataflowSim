#include "Channel.h"
#include "../ClkSys.h"
#include "../../sim/Debug.h"

using namespace DFSim;

Channel::Channel(uint _size, uint _cycle) : size(_size), cycle(_cycle)
{
	speedup = 1;  // Default speedup = 1, signify no speedup
	//currId = 1;  // Begin at 1
}

Channel::Channel(uint _size, uint _cycle, uint _speedup) :
	size(_size), cycle(_cycle), speedup(_speedup)
{
}

Channel::~Channel()
{
	for (auto& chan : upstream)
	{
		delete chan;
	}

	for (auto& chan : downstream)
	{
		delete chan;
	}
}

void Channel::addUpstream(const vector<Channel*>& _upStream)
{
	for (auto& chan : _upStream)
	{
		upstream.push_back(chan);
	}
}

void Channel::addDownstream(const vector<Channel*>& _downStream)
{
	for (auto& chan : _downStream)
	{
		downstream.push_back(chan);
	}
}

void Channel::checkConnect()
{
	if ((!noUpstream && upstream.empty()) || (!noDownstream && downstream.empty()))
	{
		Debug::throwError("Upstream/Downstream is empty!", __FILE__, __LINE__);
	}
}

void Channel::parallelize()
{
	// Push clkStall in parallel execution mode
	if (currId != speedup && !channel.empty())  // If the parallel execution dosen't finish, stall the system clock;
	{
		ClkDomain::getInstance()->addClkStall();
	}
	currId = currId % speedup + 1;
}

//bool Channel::checkUpstream()
//{
//	bool ready = 1;
//	if (!noUpstream)
//	{
//		for (auto channel : upstream)
//		{
//			if (!channel->valid)
//			{
//				ready = 0;
//				break;
//			}
//		}
//	}
//	else
//	{
//		if (channel.size() == size)  // When channel is full
//			ready = 0;
//	}
//
//	return ready;
//}

//bool Channel::checkSend(Data _data, Channel* upstream)
//{
//	bool sendable = 1;
//	if (bp)
//	{
//		sendable = 0;
//	}
//	return sendable;
//}

//void Channel::bpUpdate()
//{
//	if (channel.size() < size)
//		bp = 0;
//	else
//		bp = 1;
//}


// class ChanBase
ChanBase::ChanBase(uint _size, uint _cycle) : 
	Channel(_size, _cycle)
{
	initial();
}

ChanBase::ChanBase(uint _size, uint _cycle, uint _speedup) : 
	Channel(_size, _cycle, _speedup)
{
	initial();
}

void ChanBase::initial()
{
	enable = 1;
	bp = 0;

	// Default branch setting
	branchMode = 0;
	isCond = 0;
	channelCond = 1;

	noUpstream = 0;
	noDownstream = 0;
}

bool ChanBase::checkUpstream()
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
		if (channel.size() == size)  // When channel is full
			ready = 0;
	}

	return ready;
}

void ChanBase::pushChannel(int _data)
{
	if (!noUpstream)
	{
		Data data = upstream.front()->channel.front();
		data.last = 0;  // Reset last flag
		data.value = _data;
		data.cycle = data.cycle + cycle;
		//uint cycleTemp = data.cycle + cycle;
		//data.cycle = cycleTemp > clk ? cycleTemp : clk;

		// Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
		for (auto channel : upstream)
		{
			// loopVar not receive last, only receive lastOuter
			// Due to a channel in keepMode may repeatly send a data with a last for many times
			if (/*!isCond*/ !isLoopVar && channel->keepMode == 0)
			{
				data.last |= channel->channel.front().last;
			}
			data.lastOuter |= channel->channel.front().lastOuter;
			data.graphSwitch |= channel->channel.front().graphSwitch;
		}

		// Push getLast
		if (data.last)
			getLast.push_back(1);

		if (branchMode)
		{
			if (!upstream.front()->isCond)
			{
				// The cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
				Debug::throwError("The cond channel must be in the first element of the upstream vector!", __FILE__, __LINE__);
			}
			else if (data.cond == channelCond)
			{
				channel.push_back(data);
			}

			if (data.graphSwitch)
			{
				channel.back().graphSwitch = 1;  // In branch mode, if upstream send a graphSwitch flag, all the downstreams(branch paths) must receive it.
			}
		}
		else
		{
			channel.push_back(data);
		}
	}
	else
	{
		Data data = Data();
		data.valid = 1;
		data.value = _data;
		data.cycle = ClkDomain::getInstance()->getClk();
		channel.push_back(data);
	}
}

bool ChanBase::popLastCheck()
{
	bool popLastReady = 1;

	if (!noDownstream && keepMode)
	{
		//// If the only downstream is inner loop's loopVar, pop data when the loopVar produce last tag
		//if(downstream.size() == 1 && downstream[0]->isLoopVar)  
		//{
		//	if (downstream[0]->produceLast.empty())
		//	{
		//		popLastReady = 0;
		//	}
		//}
		//else
		//{
		//	for (auto channel : downstream)
		//	{
		//		// If only one of the downstream channel set last, the data can not be poped
		//		// lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
		//		if (/*!channel->isCond*/ !channel->isLoopVar && channel->getLast.empty())
		//		{
		//			popLastReady = 0;
		//			break;
		//		}
		//	}
		//}

		for (auto channel : downstream)
		{
			// If only one of the downstream channel set last, the data can not be poped
			// If the channel is loopVar, check produceLast queue; else check getLast queue;
			if ((channel->isLoopVar && channel->produceLast.empty()) || (!channel->isLoopVar && channel->getLast.empty()))
			{
				popLastReady = 0;
				break;
			}
		}
	}

	return popLastReady;
}

vector<int> ChanBase::popChannel(bool popReady, bool popLastReady)
{
	int popSuccess = 0;
	int popData = 0;

	if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
	{
		Data data = channel.front();
		channel.pop_front();
		popSuccess = 1;
		popData = data.value;
		lastPopVal = data.value;  // For LC->loopVar, record last pop data when the channel pop empty

		// Clear the last flags of downstreams
		if (keepMode)
		{
			for (auto& channel : downstream)
			{
				// lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
				if (!channel->isLoopVar)
				{
					channel->getLast.pop_front();
				}
				else
				{
					// If it is a loopVar, pop produceLast queue
					if (!channel->produceLast.empty())
					{
						channel->produceLast.pop_front();
					}
				}
			}
		}
	}

	return { popSuccess , popData };
}

vector<int> ChanBase::pop()
{
	bool popReady = valid;

	// popLastReady only used in keepMode
	bool popLastReady = popLastCheck();
	vector<int> popState = popChannel(popReady, popLastReady);
	updateCycle(popReady, popLastReady);

	return { popState[0], popState[1] };  // For debug
}

// Update cycle in keepMode
void ChanBase::updateCycle(bool popReady, bool popLastReady)
{
	// Update cycle in keepMode, only when the system clk updates successfully
	if (ClkDomain::getInstance()->checkClkAdd())
	{
		if (popReady && keepMode && !popLastReady)  // Update data cycle in keepMode
		{
			for (auto& data : channel)
			{
				++data.cycle;
			}
		}
	}
}

vector<int> ChanBase::push(int data)
{
	// Push data in channel
	if (checkUpstream())
	{
		pushChannel(data);
		return { 1, data };
	}
	else
		return { 0, data };
}

void ChanBase::statusUpdate()
{
	uint clk = ClkDomain::getInstance()->getClk();
	// Set valid
	valid = 1;

	if (channel.empty() || !enable)
	{
		valid = 0;
	}
	else
	{
		//uint clk = ClkDomain::getInstance()->getClk();
		Data data = channel.front();

		if (data.cycle > clk)
		{
			valid = 0;
		}

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

	if (valid)
	{
		channel.front().cycle = clk;  // Update cycle when pop data
	}

	//if (!noDownstream)
	//{
	//	for (auto channel : downstream)
	//	{
	//		if (channel->bp/* || !channel->enable*/)  // _modify 2.27
	//		{
	//			valid = 0;
	//			break;
	//		}
	//	}
	//}

	//if (!noDownstream)
	//{
	//	for (auto& channel : downstream)
	//	{
	//		if (!channel->checkSend(data, this))
	//		{
	//			valid = 0;
	//			break;
	//		}
	//	}
	//}

	bpUpdate();

	if (speedup > 1)
	{
		parallelize();
	}
}

bool ChanBase::checkSend(Data _data, Channel* upstream)
{
	bool sendable = 1;
	if (bp)
	{
		sendable = 0;
	}
	return sendable;
}

void ChanBase::bpUpdate()
{
	if (channel.size() < size)
		bp = 0;
	else
		bp = 1;
}

// Channel get data from the program variable 
vector<int> ChanBase::get(int data)
{
	vector<int> pushState(2);
	vector<int> popState(2);

	checkConnect();
	popState = pop(); // Data lifetime in nested loop
	pushState = push(data);
	statusUpdate(); // Set valid according to the downstream channels' status
	//bpUpdate();

	return { pushState[0], pushState[1], popState[0], popState[1] };
}

// Assign channel value to program varieties
int ChanBase::assign()
{
	if (!this->channel.empty())
	{
		Data data = channel.front();
		return data.value;
	}
	else
		return lastPopVal;
}


// class ChanDGSF
ChanDGSF::ChanDGSF(uint _size, uint _cycle, uint _speedup)
	: ChanBase(_size, _cycle, _speedup)
{
	//enable = 1;
	currId = 1; // Id begins at 1
	sendActiveMode = 0;  // Default set to false
}

ChanDGSF::~ChanDGSF()
{
	for (auto& chan : activeStream)
	{
		delete chan;
	}
}

vector<int> ChanDGSF::popChannel(bool popReady, bool popLastReady)
{
	int popSuccess = 0;
	int popData = 0;

	if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
	{
		Data data = channel.front();
		channel.pop_front();
		popSuccess = 1;
		popData = data.value;
		lastPopVal = data.value;

		// Clear the last flags of downstreams
		if (keepMode)
		{
			for (auto& channel : downstream)
			{
				// lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
				if (/*!channel->isCond*/ !channel->isLoopVar)
				{
					channel->getLast.pop_front();
				}
				else
				{
					// If it is a loopVar, pop produceLast queue
					if (!channel->produceLast.empty())
					{
						channel->produceLast.pop_front();
					}
				}
			}
		}

		// Active downstream and disable itself
		if (data.graphSwitch == 1 && sendActiveMode == 1)
		{
			sendActive();
			enable = 0;  // Disable current channel for graph switch
		}
	}

	return { popSuccess , popData };
}

void ChanDGSF::sendActive()
{
	if (sendActiveMode)
	{
		if (activeStream.empty())
		{
			Debug::throwError("ActiveStream is empty!", __FILE__, __LINE__);
		}
		else
		{
			for (auto& channel : activeStream)
			{
				channel->enable = 1;
			}
		}
	}
}

void ChanDGSF::parallelize()
{
	// Push clkStall in parallel execution mode
	if (currId != speedup && !channel.empty() && channel.front().graphSwitch == 0)  // If the parallel execution dosen't finish, stall the system clock;
	{
		ClkDomain::getInstance()->addClkStall();
	}
	currId = currId % speedup + 1;
}

void ChanDGSF::statusUpdate()
{
	uint clk = ClkDomain::getInstance()->getClk();
	// set valid
	valid = 1;

	if (channel.empty() || !enable)
	{
		valid = 0;
	}
	else
	{
		//uint clk = ClkDomain::getInstance()->getClk();
		Data data = channel.front();

		if (data.cycle > clk)
		{
			valid = 0;
		}

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

	if (valid)
	{
		channel.front().cycle = clk;  // Update cycle when pop data
	}

	//if (!noDownstream)
	//{
	//	for (auto channel : downstream)
	//	{
	//		if (channel->bp)  // _modify 2.27
	//		{
	//			valid = 0;
	//			break;
	//		}
	//	}
	//}

	//if (!noDownstream)
	//{
	//	for (auto& channel : downstream)
	//	{
	//		if (!channel->checkSend(Data(), this))
	//		{
	//			valid = 0;
	//			break;
	//		}
	//	}
	//}

	bpUpdate();

	//// Push clkStall in parallel execution mode
	//if (currId != speedup && !channel.empty() && channel.front().graphSwitch == 0)  // If the parallel execution dosen't finish, stall the system clock;
	//{
	//	ClkDomain::getInstance()->addClkStall();
	//}
	//currId = currId % speedup + 1;

	if (speedup > 1)
	{
		parallelize();
	}

}


// class ChanSGMF
ChanSGMF::ChanSGMF(uint _size, uint _cycle) : 
	ChanBase(_size, _cycle), chanSize(_size)
{
	init();
}

ChanSGMF::ChanSGMF(uint _size, uint _cycle, uint _bundleSize) : 
	ChanBase(_size, _cycle), chanSize(_size), chanBundleSize(_bundleSize)
{
	init();
}

ChanSGMF::~ChanSGMF()
{
	for (auto& upstream : upstreamBundle)
	{
		for (auto& chan : upstream)
		{
			delete chan;
		}
	}
}

void ChanSGMF::init()
{
	chanBundle.resize(chanBundleSize);  // Default channel number(chanBundleSize) = 2;
	upstreamBundle.resize(chanBundleSize);
	//downstream.resize(chanBundleSize);

	for (auto& chan : chanBundle)
	{
		//chan.resize(chanSize);
		chan.resize(chanSize * std::max(cycle, static_cast<uint>(1)));  // Avoid tag conflict stall in multi-cycle channel
	}

	//matchQueue.resize(chanSize);
}

void ChanSGMF::checkConnect()
{
	bool upstreamEmpty = 0;
	for (auto& upstream : upstreamBundle)
	{
		upstreamEmpty |= upstream.empty();
	}

	if ((!noUpstream && upstreamEmpty) || (!noDownstream && downstream.empty()))
	{
		Debug::throwError("Upstream/Downstream is empty!", __FILE__, __LINE__);
	}
}

void ChanSGMF::addUpstream(const vector<vector<Channel*>>& _upstreamBundle)
{
	for (size_t i = 0; i < _upstreamBundle.size(); ++i)
	{
		for (auto& chan : _upstreamBundle[i])
		{
			upstreamBundle[i].push_back(chan);
		}
	}
}

// addUpstream for single channel chanSGMF
void ChanSGMF::addUpstream(const vector<Channel*>& _upstream)
{
	if (chanBundle.size() != 1)
	{
		Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
	}
	else
	{
		for (auto& chan : _upstream)
		{
			upstreamBundle[0].push_back(chan);
			upstream.push_back(chan);  // For single channel
		}
	}
}

//void ChanSGMF::addDownstream(const vector<vector<ChanSGMF*>>& _downstreamBundle)
//{
//	for (size_t i = 0; i < _downstreamBundle.size(); ++i)
//	{
//		for (auto& chan : _downstreamBundle[i])
//		{
//			downstreamBundle[i].push_back(chan);
//		}
//	}
//}

vector<int> ChanSGMF::get(vector<int> data)
{
	vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
	vector<int> popState(2);

	checkConnect();
	popState = pop(); // Data lifetime in nested loop
	
	for (size_t i = 0; i < chanBundle.size(); ++i)
	{
		vector<int> tmp(2);
		tmp = push(data[i], i, 0);  // If no upstream, default tag = 0
		pushState[i] = tmp[0];
		pushState[i + 1] = tmp[1];
	}
	
	statusUpdate(); // Set valid according to the downstream channels' status
	//bpUpdate();

	pushState.insert(pushState.end(), popState.begin(), popState.end());
	return pushState;
}

// For single channel
vector<int> ChanSGMF::get(int data)
{
	if (chanBundleSize != 1)
	{
		Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
	}

	vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
	vector<int> popState(2);

	checkConnect();
	popState = pop(); // Data lifetime in nested loop
	pushState = push(data, 0, 0);  // For single channel, chanId = 0 defaultly; If no upstream, tag = 0 defaultly;
	statusUpdate(); // Set valid according to the downstream channels' status
	//bpUpdate();

	pushState.insert(pushState.end(), popState.begin(), popState.end());
	return pushState;
}

// For no upstream channel
vector<int> ChanSGMF::get(int data, uint tag)
{
	if (chanBundleSize != 1)
	{
		Debug::throwError("chanBundle's size is not equal to 1!", __FILE__, __LINE__);
	}

	if (!noUpstream)
	{
		Debug::throwError("It is not a no upstream channel!", __FILE__, __LINE__);
	}

	vector<int> pushState(chanBundleSize * 2);  // Push into Din1 and Din2
	vector<int> popState(2);

	checkConnect();
	popState = pop(); // Data lifetime in nested loop
	pushState = push(data, 0, tag);  // For single channel, chanId = 0 defaultly
	statusUpdate(); // Set valid according to the downstream channels' status
	//bpUpdate();

	pushState.insert(pushState.end(), popState.begin(), popState.end());
	return pushState;
}

vector<int> ChanSGMF::popChannel(bool popReady, bool popLastReady)
{
	int popSuccess = 0;
	int popData = 0;

	if (popReady && (!keepMode || popLastReady)) // If keepMode = 0, popLastReady is irrelevant
	{
		Data data = channel.front();
		uint tag = data.tag;
		uint addr = (std::max(cycle, uint(1)) - 1) * chanSize + tag;

		channel.pop_front();  // Pop channel(sendQueue)
		for (auto& chan : chanBundle)
		{
			chan[addr].valid = 0;  // Pop corrsponding data in each channel, set valid to 0
		}

		//for (auto& data : matchQueue)
		//{
		//	if (data.tag == tag)  // Pop matchQueue
		//	{
		//		data.valid = 0;
		//	}
		//}

		for (auto ptr = matchQueue.begin(); ptr != matchQueue.end();/* ++ptr*/)
		{
			if (ptr->tag == tag)
			{
				ptr = matchQueue.erase(ptr);  // Pop matchQueue
			}
			else
			{
				++ptr;
			}
		}

		popSuccess = 1;
		popData = data.value;
		lastPopVal = data.value;  // For LC->cond, record last pop data when the channel pop empty

		// Clear the last flags of downstreams
		if (keepMode)
		{
			for (auto& channel : downstream)
			{
				// lc->loopVar produces last tag rather than get last tag, so ignore lc->loopVar!
				if (/*!channel->isCond*/ !channel->isLoopVar)
				{
					channel->getLast.pop_front();
				}
				else
				{
					// If it is a loopVar, pop produceLast queue
					if (!channel->produceLast.empty())
					{
						channel->produceLast.pop_front();
					}
				}
			}
		}
	}

	return { popSuccess , popData };
}

void ChanSGMF::updateCycle(bool popReady, bool popLastReady)
{
	if (popReady && keepMode && !popLastReady)  // Update data cycle in keepMode
	{
		for (auto& chan : chanBundle)  // Update data cycle in channel
		{
			for (auto& data : chan)
			{
				++data.cycle;
			}
		}

		for (auto& data : matchQueue)  // Update data cycle in popFifo
		{
			++data.cycle;
		}

		for (auto& data : channel)  // Update data cycle in channel(sendQueue)
		{
			++data.cycle;
		}
	}
}

vector<int> ChanSGMF::push(int data, uint chanId, uint tag)
{
	// Push data in channel
	if (checkUpstream(chanId))
	{
		pushChannel(data, chanId, tag);
		return { 1, data };
	}
	else
		return { 0, data };
}

bool ChanSGMF::checkUpstream(uint chanId)
{
	bool ready = 1;
	if (!noUpstream)
	{
		for (auto channel : upstreamBundle[chanId])
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
		if (channel.size() == size)  // if no upstream
			ready = 0;
	}

	return ready;
}

void ChanSGMF::pushChannel(int _data, uint chanId, uint _tag)
{
	if (!noUpstream)
	{
		Data data = upstreamBundle[chanId].front()->channel.front();
		data.value = _data;
		data.cycle = data.cycle + cycle;
		//uint cycleTemp = data.cycle + cycle;
		//data.cycle = cycleTemp > clk ? cycleTemp : clk;

		// Bind tags for data; (Ignore the the tag of upstream channel in keepMode)
		uint tag;
		bool tagBind = 0;
		for (auto& chan : upstreamBundle[chanId])
		{
			if (!chan->keepMode)
			{
				tag = data.tag;
				tagBind = 1;
				break;
			}
		}
		if (!tagBind)
		{
			Debug::throwError("Need more upstream channels except the one in keepMode!", __FILE__, __LINE__);
		}

		if (tagUpdateMode)
		{
			tag = (tag + 1) % chanSize;  // Update tag in tagUpdateMode
			data.tag = tag;
		}

		// loopVar not receive last, only receive lastOuter
		// Update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
		for (auto& channel : upstreamBundle[chanId])
		{
			if (/*!isCond*/ !isLoopVar && channel->keepMode == 0)
			{
				data.last |= channel->channel.front().last;
			}
			data.lastOuter |= channel->channel.front().lastOuter;
			//data.graphSwitch |= channel->channel.front().graphSwitch;
		}

		// Push getLast
		if (data.last)
			getLast.push_back(1);
		//if (!keepMode && data.last)
		//{
		//	getLast.push_back(1);
		//}

		if (branchMode)
		{
			if (!upstream.front()->isCond)
			{
				// The cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
				Debug::throwError("The cond channel must be in the first element of the upstream vector!", __FILE__, __LINE__);
			}
			else if (data.cond == channelCond)
			{
				chanBundle[chanId][tag] = data;
				//for (size_t i = tag; i < chanBundle[chanId].size(); i = i + chanSize)
				//{
				//	if (!chanBundle[chanId][i].valid)
				//	{
				//		//data.chanBundleAddr = i;
				//		chanBundle[chanId][i] = data;
				//		break;
				//	}
				//}
			}
		}
		else
		{
			chanBundle[chanId][tag] = data;
			//for (size_t i = tag; i < chanBundle[chanId].size(); i = i + chanSize)
			//{
			//	if (!chanBundle[chanId][i].valid)
			//	{
			//		//data.chanBundleAddr = i;
			//		chanBundle[chanId][i] = data;
			//		break;
			//	}
			//}
		}
	}
	else
	{
		Data data = Data();
		data.tag = _tag;
		data.valid = 1;
		data.value = _data;
		data.cycle = ClkDomain::getInstance()->getClk();
		chanBundle[chanId][data.tag] = data;  
		//for (size_t i = _tag; i < chanBundle[chanId].size(); i = i + chanSize)
		//{
		//	if (!chanBundle[chanId][i].valid)
		//	{
		//		//data.chanBundleAddr = i;
		//		chanBundle[chanId][i] = data;
		//		break;
		//	}
		//}
	}
}

void ChanSGMF::statusUpdate()
{
	// Reset valid
	valid = 0;

	// Shift data in multi-cycle chanBundle
	for (auto& chan : chanBundle)  // Traverse each channel in chanBundle
	{
		for (size_t tag = 0; tag < chanSize; ++tag)  // Traverse tag
		{
			if (cycle > 1)  // Only when cycle >= 2, it needs to shift data
			{
				for (uint i = 0; i < cycle - 1; ++i)
				{
					uint addrBase = (cycle - 2 - i) * chanSize;
					if (!chan[addrBase + chanSize + tag].valid)  // Next round addr
					{
						if (chan[addrBase + tag].valid)
						{
							chan[addrBase + chanSize + tag] = chan[addrBase + tag];  // Shfit data
							chan[addrBase + tag].valid = 0;  // Clear after shift
						}
					}
				}
			}
		}
	}

	// Check tag match among channels
	for (size_t i = 0; i < chanSize; ++i)
	{
		uint tag = i;
		Data data;
		bool match = 1;
		uint addr = (std::max(cycle, uint(1)) - 1) * chanSize + i;  // For multi-cycle channel, only check the bottom section's tag

		for (auto& chan : chanBundle)
		{
			if (!chan[addr].valid)
			{
				match = 0;
				break;
			}
			else
			{
				// loopVar not receive last, only receive lastOuter
				// Due to a channel in keepMode may repeatly send a data with a last for many times
				if (!isLoopVar && keepMode == 0)
				{
					data.last |= chan[addr].last;
				}
				data.lastOuter |= chan[addr].lastOuter;
				data.cycle = std::max(data.cycle, chan[addr].cycle);  // Update the cycle as the mux of each channel
			}
		}

		if (match)
		{
			data.valid = 1;
			data.tag = i;
			bool tagConflict = 0;

			// Avoid a same data being pushed into matchQueue twice
			for (auto data : matchQueue)
			{
				if (data.tag == tag)
				{
					tagConflict = 1;
					break;
				}
			}

			if (!tagConflict)
			{
				matchQueue.push_back(data);
			}
		}
	}

	// Check the cycle of data in the matchQueue
	uint clk = ClkDomain::getInstance()->getClk();
	for (auto& data : matchQueue)
	{
		if (data.cycle > clk)
		{
			data.valid = 0;
		}
		else
		{
			data.valid = 1;
		}
	}
	
	// Check whether downstream channel avaliable to receive data
	if (!noDownstream)
	{
		for (auto& data : matchQueue)
		{
			if (data.valid)
			{
				bool sendable = 1;
				// Regard the data in keepMode channel as a constant, always can be send out if the channel is valid;
				if (keepMode)
				{
					if (!channel.empty())  // Only when the channel is empty, a new data can be pushed in it
					{
						sendable = 0;
					}
				}
				else
				{
					for (auto& channel : downstream)
					{
						if (!channel->checkSend(data, this))  // Must guarantee "this" points to current upstream channel!
						{
							sendable = 0;
							break;
						}
					}
				}

				if (sendable)
				{
					channel.push_back(data);  // Push data sendable into channel(sendQueue)
					//valid = 1;  // Set valid = 1, signify the data in channel(sendQueue) is valid for all the downstream
					break;  // Only one data can be pushed in each cycle
				}
			}
		}
	}

	if (!channel.empty())
	{
		valid = 1;
		channel.front().cycle = clk;  // Update cycle when pop data
	}

	//bpUpdate();
}

bool ChanSGMF::checkSend(Data _data, Channel* _upstream)
{
	uint tag = _data.tag;
	bool sendable = 0;
	uint chanId = 0;
	bool upstreamMatch = 0;

	if (!bp)  // In ChanSGMF, bp is only updated by downstream if needed.(e.g. in trueChan & falseChan in Mux)
	{
		// Search current upstream is in which upstreamBundle
		for (size_t i = 0; i < upstreamBundle.size(); ++i)
		{
			for (auto& chan : upstreamBundle[i])
			{
				if (chan == _upstream)
				{
					upstreamMatch = 1;
					chanId = i;
					break;
				}
			}

			// Check same tag conflict
			if (upstreamMatch)
			{
				if (tagUpdateMode)
				{
					tag = (tag + 1) % chanSize;  // Update tag in tagUpdateMode
					if (!chanBundle[chanId][tag].valid)
					{
						sendable = 1;
					}
				}
				else
				{
					if (!chanBundle[chanId][tag].valid)
					{
						sendable = 1;
					}
				}

				break;
			}
		}
	}
	else
	{
		sendable = 0;
	}

	return sendable;
}

// In SGMF mode, interface func assign is to get value of corresponding channel by chanId (e.g. Din1, Din2)
int ChanSGMF::assign(uint chanId)
{
	uint tag = 0;
	if (!channel.empty())
	{
		tag = channel.front().tag;
		uint addr = (std::max(cycle, uint(1)) - 1) * chanSize + tag;
		return chanBundle[chanId][addr].value;
	}
	else
	{
		//return MAX;  // Indicate current value is invalid
		return lastPopVal;
	}
}