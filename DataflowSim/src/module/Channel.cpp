#include "Channel.h"
#include "ClkSys.h"

using namespace DFSim;

Channel::Channel(vector<Channel*> _upstream, vector<Channel*> _downstream, uint _size, int _speedup, uint _cycle) 
	: upstream(_upstream), downstream(_downstream), size(_size), speedup(_speedup), cycle(_cycle)
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

	// each upstream has an individual req queue, and each queue's size equal to the channel size
	req.resize(upstream.size());
	for (auto& q : req)
	{
		q.resize(size);
	}
}

bool Channel::checkUpstream()
{
	bool ready = 1;
	for (auto channel : upstream)
	{
		if (!channel->enable || channel->channel.empty() || !valid)
		{
			ready = 0;
			break;
		}
	}
	return ready;
}

bool Channel::checkClk(uint clk)
{
	bool clkSatisfy = 1;
	for (auto channel : upstream)
	{
		if (channel->channel.front().cycle > clk) // data is in advance of the clock
		{
			clkSatisfy = 0;
			break;
		}
	}
	return clkSatisfy;
}

//void Channel::popUpstream()
//{
//	for (auto channel : upstream)
//	{
//		channel->channel.pop_front();
//	}
//}

void Channel::pushChannel(int _data, uint clk)
{
	Data data = upstream.front()->channel.front();

	// update data last flag; If only one input data's last flag equall to 1, set current data's last flag; 
	for (auto channel : upstream)
	{
		data.last |= channel->channel.front().last;
	}

	if (branchMode && static_cast<bool>(data.value) == channelCond)
	{
		if (!upstream.front()->isCond)
		{
			// the cond channel must be in the first element of the upstream vector (e.g. upstream vector = {cond_channel, channelA, ...} )
			DEBUG_ASSERT(false);
		}
		else
		{
			// push data in channel
			if (channel.size() < size)
			{
				data.value = _data;
				data.cycle = clk + cycle; // system clock add channel execute cycle
				channel.push_back(data);
			}
			else
			{
				// try to push data into a full channel
				DEBUG_ASSERT(false);
			}
		}
	}
}

void Channel::push(int data)
{
	uint clk = ClkDomain::getClk();

	// push data in channel
	if (checkUpstream() && checkClk(clk))
	{
		pushChannel(data, clk);
	}
}

void Channel::pop()
{
	bool popReady = 1;
	for (auto channel : downstream)
	{
		if (channel->bp) // if only one of the downstream channel set bp, the data can not be poped
		{
			popReady = 0;
			break;
		}
	}

	if (popReady)
	{
		channel.pop_front();
	}
}

void Channel::statusUpdate()
{
	// set valid
	valid = 1;
	for (auto channel : downstream)
	{
		if (channel->channel.size() >= size)
		{
			valid = 0;
			break;
		}
	}

	// push clkStall in parallel execution mode
	currId = (++currId) % speedup;
	if (currId != speedup && channel.front().last == 0)  // if the parallel execution dosen't finish, stall the system clock;
	{
		ClkDomain::addClkStall();
	}

}

void Channel::sendActive()
{
	if (sendActiveMode)
	{
		if (activeStream.empty())
		{
			// activeStream is empty
			DEBUG_ASSERT(false);
		}
		else if(channel.size() == size || channel.back().last == 1) // when current sub-graph is over, active next sub-graph
		{
			for (auto channel : activeStream)
			{
				channel->enable = 1;
			}
		}
	}
}

void Channel::bpUpdate()
{
	if (channel.size() < size)
		bp = 0;
	else
		bp = 1;
}

// channel get data from the program variable 
void Channel::get(int data)
{
	pop(); // data lifetime in nested loop
	push(data);
	statusUpdate(); // set valid according to the downstream channels' status
	sendActive(); // in DGSF architecture
	bpUpdate();
}

// assign channel value to program varieties
int Channel::assign(Channel* c)
{
	if (!c->channel.empty())
	{
		return c->channel.front().value;
	}
}
