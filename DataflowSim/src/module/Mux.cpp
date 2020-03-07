#include "./Mux.h"

using namespace DFSim;

Mux::Mux(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream)
{
	trueChan->noDownstream = 1;
	falseChan->noDownstream = 1;
	outChan->noUpstream = 1;

	trueChan->addUpstream(_trueChan);
	falseChan->addUpstream(_falseChan);
	outChan->addDownstream(_downstream);

	// Add trueChan as downstream for each upstream automatically
	for (auto& chan : _trueChan)
	{
		chan->addDownstream({ trueChan });
	}

	// Add falseChan as downstream for each upstream automatically
	for (auto& chan : _falseChan)
	{
		chan->addDownstream({ falseChan });
	}

	// Add outChan as upstream for each downstream automatically
	for (auto& chan : _downstream)
	{
		chan->addUpstream({ outChan });
	}
}

bool Mux::checkUpstream(vector<Channel*>& upstream)
{
	bool ready = 1;
	for (auto& channel : upstream)
	{
		if (!channel->valid)
		{
			ready = 0;
			break;
		}
	}
	return ready;
}

void Mux::bpUpdate()
{
	bool bp = 0;
	for (auto& channel : outChan->downstream)
	{
		if (channel->bp)
		{
			bp = 1;
			break;
		}
	}

	if (bp)
	{
		trueChan->bp = 1;
		falseChan->bp = 1;
	}
	else
	{
		trueChan->bp = 0;
		falseChan->bp = 0;
	}
}

void Mux::muxUpdate(bool sel)
{
	outChan->pop();
	outChan->statusUpdate();

	muxSuccess = 0;

	if (sel)
	{
		if (checkUpstream(trueChan->upstream))
		{
			outChan->get(1);
			muxSuccess = 1;
		}
	}
	else
	{
		if (checkUpstream(falseChan->upstream))
		{
			outChan->get(1);
			muxSuccess = 1;
		}
	}

	bpUpdate();
}

int Mux::mux(int trueData, int falseData, bool sel)
{
	return sel ? trueData : falseData;
}


// class MuxLc
MuxLC::MuxLC(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream) :
	Mux(_trueChan, _falseChan, _downstream)
{

}

bool MuxLC::checkUpstream(vector<Channel*>& upstream)
{
	bool ready = 1;
	for (auto& channel : upstream)
	{
		if (!channel->isCond && !channel->valid)
		{
			ready = 0;
			break;
		}
	}
	return ready;
}

void MuxLC::bpUpdate()
{
	bool bp = 0;
	for (auto& channel : outChan->downstream)
	{
		if (!channel->isCond && channel->bp)  // Avoid LC->loopVar deadlock due to bp
		{
			bp = 1;
			break;
		}
	}

	if (bp)
	{
		trueChan->bp = 1;
		falseChan->bp = 1;
	}
	else
	{
		trueChan->bp = 0;
		falseChan->bp = 0;
	}
}