#include "Channel.h"
#include "ClkSys.h"

using namespace DFSim;

Channel::Channel(uint _size, uint _speedup, uint _cycle) 
	: size(_size), speedup(_speedup), cycle(_cycle)
{
	initial();
}

void Channel::initial()
{
	#ifdef DGSF
		enable = 0;  // enable default value is 0 in DGSF, need active in manual
	#else
		enable = 1;
	#endif

	currId = 1; // Id begins at 1
	bp = 0;

	// default branch setting
	branchMode = 0;
	isCond = 0;
	channelCond = 1;

	sendActiveMode = 0;

	noUpstream = 0;
	noDownstream = 0;

	//// each upstream has an individual req queue, and each queue's size equal to the channel size
	//req.resize(upstream.size());
	//for (auto& q : req)
	//{
	//	q.resize(size);
	//}
}

void Channel::addUpstream(const vector<Channel*>& _upStream)
{
	//upstream = _upStream;
	upstream.assign(_upStream.begin(), _upStream.end());
}

void Channel::addDownstream(const vector<Channel*>& _downStream)
{
	//downstream = _downStream;
	downstream.assign(_downStream.begin(), _downStream.end());
}

void Channel::checkConnect()
{
	if ((!noUpstream && upstream.empty()) || (!noDownstream && downstream.empty()))
	{
		// upstream/downstream is empty
		DEBUG_ASSERT(false);
	}
}

bool Channel::checkUpstream()
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
		if (channel.size() == size)
			ready = 0;
	}

	return ready;
}

void Channel::pushChannel(int _data, uint clk)
{
	if (!noUpstream)
	{
		Data data = upstream.front()->channel.front();
		data.value = _data;
		uint cycleTemp = data.cycle + cycle;
		data.cycle = cycleTemp > clk ? cycleTemp : clk;

		// update data last/graphSwitch flag; If only one input data's last = 1, set current data's last flag; 
		for (auto channel : upstream)
		{
			data.last |= channel->channel.front().last;
			data.lastOuter |= channel->channel.front().lastOuter;
			data.graphSwitch |= channel->channel.front().graphSwitch;
		}

		// replace the value of the last data, to avoid overflow
		if (data.last && !isCond)  // if current channel is a cond channel, do not replace!
			data.value = lastVal;
		else
			lastVal = _data;

		// push getLast
		if (data.last)
			getLast.push_back(1);

		if (branchMode)
		{
			if (static_cast<bool>(data.value) == channelCond)
			{
				if (!upstream.front()->isCond)
				{
					// the cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
					DEBUG_ASSERT(false);
				}
				else
				{
					channel.push_back(data);
					//hasReceived.push_back(1);
				}
			}
		}
		else
		{
			channel.push_back(data);
			//hasReceived.push_back(1);
		}
	}
	else
	{
		Data data = Data();
		data.value = _data;
		data.cycle = clk;
		channel.push_back(data);
	}
}

vector<int> Channel::push(int data)
{
	uint clk = ClkDomain::getInstance()->getClk();

	// push data in channel
	if (checkUpstream())
	{
		pushChannel(data, clk);
		return {1, data};
	}
	else
		return {0, data};
}

vector<int> Channel::pop()
{
	int popSuccess = 0;
	int popData = 0;

	bool popReady = valid && enable;

	// popLastReady only used in keepMode
	bool popLastReady = 1;
	if (!noDownstream && keepMode)
	{
		for (auto channel : downstream)
		{
			// if only one of the downstream channel set last, the data can not be poped
			// lc->cond produces last tag rather than get last tag, so ignore lc->cond!
			if (!channel->isCond && channel->getLast.empty()) 
			{
				popLastReady = 0;
				break;
			}
		}
	}

	if (popReady && (!keepMode || popLastReady)) // if keepMode = 0, popLastReady is irrelevant
	{
		Data data = channel.front();
		channel.pop_front();
		popSuccess = 1;
		popData = data.value;

		// clear the last flags of downstreams
		if (keepMode)
		{
			for (auto& channel : downstream)
			{
				// lc->cond produces last tag rather than get last tag, so ignore lc->cond!
				if (!channel->isCond) 
				{
					channel->getLast.pop_front();
				}
			}
		}

#ifdef DGSF
		// reset enable
		if (data.graphSwitch == 1)
		{
			sendActive();
			enable = 0;  // disable current channel for graph switch
		}
#endif
	}

	if (popReady && keepMode && !popLastReady)  // update data cycle in keepMode
	{
		for (auto& data : channel)
		{
			++data.cycle;
		}
	}

	return { popSuccess, popData };  // for debug
}

void Channel::statusUpdate()
{
	// set valid
	valid = 1;

	if (channel.empty() || !enable)
	{
		valid = 0;
	}
	else
	{
		uint clk = ClkDomain::getInstance()->getClk();
		Data data = channel.front();

		if (data.cycle > clk)
		{
			valid = 0;
		}
	}

	if (!noDownstream)
	{
		for (auto channel : downstream)
		{
			if (channel->bp/* || !channel->enable*/)  // _modify 2.27
			{
				valid = 0;
				break;
			}
		}
	}

	// push clkStall in parallel execution mode
	if (currId != speedup && !channel.empty() && channel.front().graphSwitch == 0)  // if the parallel execution dosen't finish, stall the system clock;
	{
		ClkDomain::getInstance()->addClkStall();
	}
	currId = currId % speedup + 1;

}

#ifdef DGSF
void Channel::sendActive()
{
	if (sendActiveMode)
	{
		if (activeStream.empty())
		{
			// activeStream is empty
			DEBUG_ASSERT(false);
		}
		else
		{
			for (auto& channel : activeStream)
			{
				enable = 1;
			}
		}
		//else if(channel.size() == size || channel.back().graphSwitch == 1) // when current sub-graph is over, active next sub-graph
		//{
		//	enable = 1;  // _modify 2.27
		//	/*for (auto& channel : activeStream)
		//	{
		//		channel->enable = 1;
		//	}*/
		//}
	}
}
#endif

void Channel::bpUpdate()
{
	if (channel.size() < size)
		bp = 0;
	else
		bp = 1;
}

// channel get data from the program variable 
vector<int> Channel::get(int data)
{	
	vector<int> pushState(2);
	vector<int> popState(2);

	checkConnect();
	popState = pop(); // data lifetime in nested loop
	pushState = push(data);
	statusUpdate(); // set valid according to the downstream channels' status
//#ifdef DGSF
//	sendActive(); // in DGSF architecture
//#endif
	bpUpdate();

	return { pushState[0], pushState[1], popState[0], popState[1] };
}

// assign channel value to program varieties
int Channel::assign()
{
	if (!this->channel.empty())
	{
		Data data = channel.front();
		return data.value;
	}
	else
		return 0;
}


