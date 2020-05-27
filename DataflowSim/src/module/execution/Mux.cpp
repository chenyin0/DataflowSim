#include "./Mux.h"
#include "../../util/util.hpp"
#include "../Registry.h"

using namespace DFSim;

Mux::Mux(ChanBase* _trueChan, ChanBase* _falseChan, ChanBase* _outChan) :
	trueChan(_trueChan), falseChan(_falseChan), outChan(_outChan)
{
	moduleId = Registry::registerMux(this);

	trueChan->noDownstream = 1;
	falseChan->noDownstream = 1;
	outChan->noUpstream = 1;

	//trueChan->addUpstream(_trueChan);
	//falseChan->addUpstream(_falseChan);
	//outChan->addDownstream(_downstream);

	//// Add trueChan as downstream for each upstream automatically
	//for (auto& chan : _trueChan)
	//{
	//	chan->addDownstream({ trueChan });
	//}

	//// Add falseChan as downstream for each upstream automatically
	//for (auto& chan : _falseChan)
	//{
	//	chan->addDownstream({ falseChan });
	//}

	//// Add outChan as upstream for each downstream automatically
	//for (auto& chan : _downstream)
	//{
	//	chan->addUpstream({ outChan });
	//}
}

Mux::~Mux()
{
	//delete trueChan;
	//delete falseChan;
	//delete outChan;
}

void Mux::addPort(vector<Channel*> _trueChanUpstream, vector<Channel*> _falseChanUpstream, vector<Channel*> _outChanDownstream)
{
	trueChan->addUpstream(_trueChanUpstream);
	falseChan->addUpstream(_falseChanUpstream);
	outChan->addDownstream(_outChanDownstream);

	// Add trueChan as downstream for each upstream automatically
	for (auto& chan : _trueChanUpstream)
	{
		chan->addDownstream({ trueChan });
	}

	// Add falseChan as downstream for each upstream automatically
	for (auto& chan : _falseChanUpstream)
	{
		chan->addDownstream({ falseChan });
	}

	// Add outChan as upstream for each downstream automatically
	for (auto& chan : _outChanDownstream)
	{
		chan->addUpstream({ outChan });
	}
}

bool Mux::checkUpstream(vector<Channel*>& upstream)
{
	bool ready = 1;

	if (!upstream.empty())
	{
		for (auto& channel : upstream)
		{
			if (sel == false)
			{
				if (!channel->isFeedback && !channel->valid)  // If select the initial value, ignore the feedback channel(e.g. loopVar)
				{
					ready = 0;
					break;
				}
			}
			else
			{
				if (!channel->valid)
				{
					ready = 0;
					break;
				}
			}
		}

		if (ready)
		{
			recordDataStatus(upstream);
		}
	}

	return ready;
}

void Mux::recordDataStatus(vector<Channel*>& upstream)
{
	upstreamDataStatus.last = 0;
	upstreamDataStatus.lastOuter = 0;
	upstreamDataStatus.graphSwitch = 0;

	// Record upstream's data status
	for (auto& chan : upstream)
	{
		if (!chan->channel.empty())
		{
			upstreamDataStatus.last |= chan->channel.front().last;
			upstreamDataStatus.lastOuter |= chan->channel.front().lastOuter;
			upstreamDataStatus.graphSwitch |= chan->channel.front().graphSwitch;
		}
	}
}

void Mux::bpUpdate()
{
	bool bp = 0;
	for (auto& channel : outChan->downstream)  // Check outChan's downstream rather than outChan, due to the outChan is like a pass wire.
	{
		if (!channel->isFeedback && channel->bp)
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

void Mux::pushOutChan()
{
	outChan->get(1);

	// Update data status
	outChan->channel.back().last = upstreamDataStatus.last;
	outChan->channel.back().lastOuter = upstreamDataStatus.lastOuter;
	outChan->channel.back().graphSwitch = upstreamDataStatus.graphSwitch;
}

void Mux::muxUpdate(bool sel)
{
	this->sel = sel;

	outChan->pop();
	outChan->statusUpdate();

	muxSuccess = 0;

	if (sel)
	{
		if (checkUpstream(trueChan->upstream))
		{
			//outChan->get(1);

			//// Update data status
			//outChan->channel.back().last = upstreamDataStatus.last;
			//outChan->channel.back().lastOuter = upstreamDataStatus.lastOuter;
			//outChan->channel.back().graphSwitch = upstreamDataStatus.graphSwitch;

			pushOutChan();
			muxSuccess = 1;
		}
	}
	else
	{
		if (checkUpstream(falseChan->upstream))
		{
			//outChan->get(1);

			//// Update data status
			//outChan->channel.back().last = upstreamDataStatus.last;
			//outChan->channel.back().lastOuter = upstreamDataStatus.lastOuter;
			//outChan->channel.back().graphSwitch = upstreamDataStatus.graphSwitch;
			
			pushOutChan();
			muxSuccess = 1;
		}
	}

	bpUpdate();
}

int Mux::mux(int trueData, int falseData, bool sel)
{
	return sel ? trueData : falseData;
}


//// class MuxLc
//MuxLc::MuxLc(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream) :
//	Mux(_trueChan, _falseChan, _downstream)
//{
//
//}
//
//bool MuxLc::checkUpstream(vector<Channel*>& upstream)
//{
//	bool ready = 1;
//	for (auto& channel : upstream)
//	{
//		if (/*!channel->isCond*/ !channel->isLoopVar && !channel->valid)
//		{
//			ready = 0;
//			break;
//		}
//	}
//	return ready;
//}

//void MuxLc::bpUpdate()
//{
//	bool bp = 0;
//	for (auto& channel : outChan->downstream)
//	{
//		if (/*!channel->isCond*/ !channel->isLoopVar && channel->bp)  // Avoid LC->loopVar deadlock due to bp
//		{
//			bp = 1;
//			break;
//		}
//	}
//
//	if (bp)
//	{
//		trueChan->bp = 1;
//		falseChan->bp = 1;
//	}
//	else
//	{
//		trueChan->bp = 0;
//		falseChan->bp = 0;
//	}
//}


// class MuxSGMF
MuxSGMF::MuxSGMF(ChanSGMF* _trueChan, ChanSGMF* _falseChan, ChanSGMF* _outChan) :
	Mux(_trueChan, _falseChan, _outChan), trueChan(_trueChan), falseChan(_falseChan), outChan(_outChan)
{
	trueChan->noDownstream = 1;
	falseChan->noDownstream = 1;
	outChan->noUpstream = 1;

	//vector<vector<Channel*>> trueChanTemp;
	//vector<vector<Channel*>> falseChanTemp;
	//for (size_t i = 0; i < trueChan->chanBundle.size(); ++i)
	//{
	//	trueChanTemp.push_back(Util::cast(_trueChan, (vector<Channel*>*)nullptr));
	//}

	//for (size_t i = 0; i < falseChan->chanBundle.size(); ++i)
	//{
	//	falseChanTemp.push_back(Util::cast(_falseChan, (vector<Channel*>*)nullptr));
	//}

	//trueChan->addUpstream(trueChanTemp);  // Send same data into all the channel bundles
	//falseChan->addUpstream(falseChanTemp);  // Send same data into all the channel bundles

	//trueChan->addUpstream(Util::cast(_trueChan, (vector<Channel*>*)nullptr));  // addUpstream for single channel
	//falseChan->addUpstream(Util::cast(_falseChan, (vector<Channel*>*)nullptr));  // addUpstream for single channel
	//outChan->addDownstream(Util::cast(_downstream, (vector<Channel*>*)nullptr));

	//// Add trueChan as downstream for each upstream automatically
	//for (auto& chan : _trueChan)
	//{
	//	chan->addDownstream({ trueChan });
	//}

	//// Add falseChan as downstream for each upstream automatically
	//for (auto& chan : _falseChan)
	//{
	//	chan->addDownstream({ falseChan });
	//}

	//// Add outChan as upstream only for the single channel downstream automatically
	//for (auto& chan : _downstream)
	//{
	//	if (chan->chanBundle.size() == 1)
	//	{
	//		chan->addUpstream({ outChan });
	//	}

	//	//chan->addUpstream({ { outChan }, {outChan} });
	//}
}

MuxSGMF::~MuxSGMF()
{
	//delete trueChan;
	//delete falseChan;
	//delete outChan;
}

void MuxSGMF::addPort(vector<ChanSGMF*> _trueChanUpstream, vector<ChanSGMF*> _falseChanUpstream, vector<ChanSGMF*> _outChanDownstream)
{
	trueChan->addUpstream(Util::cast(_trueChanUpstream, (vector<Channel*>*)nullptr));  // addUpstream for single channel
	falseChan->addUpstream(Util::cast(_falseChanUpstream, (vector<Channel*>*)nullptr));  // addUpstream for single channel
	outChan->addDownstream(Util::cast(_outChanDownstream, (vector<Channel*>*)nullptr));

	// Add trueChan as downstream for each upstream automatically
	for (auto& chan : _trueChanUpstream)
	{
		chan->addDownstream({ trueChan });
	}

	// Add falseChan as downstream for each upstream automatically
	for (auto& chan : _falseChanUpstream)
	{
		chan->addDownstream({ falseChan });
	}

	// Add outChan as upstream only for the single channel downstream automatically
	for (auto& chan : _outChanDownstream)
	{
		if (chan->chanBundle.size() == 1)
		{
			chan->addUpstream({ outChan });
		}

		//chan->addUpstream({ { outChan }, {outChan} });
	}
}

void MuxSGMF::bpUpdate()
{
	/*	In SGMF mode, backpressure mechanism is realized by same tag check rather than BP.
		So in the MuxSGMF, each cycle trueChan & falseChan update their channel status by outChan's downstream.
		In other words, when trueChan/falseChan's upstreams execute checkSend(), they actually are checking the outChan's downstream.
	*/
	Data data;
	for (int i = 0; i < trueChan->chanBundle[0].size(); ++i)  // Traverse each tag 
	{
		data.tag = i;
		bool setValid = 0;

		for (auto& chan : outChan->downstream)
		{
			if (!chan->isFeedback)  // Ignore feedback channel to aviod deadlock (e.g. loopVar or inter-loop dependency)
			{
				if (!chan->checkSend(data, outChan))
				{
					for (auto& chan : trueChan->chanBundle)
					{
						chan[i].valid = 1;  // Copy downstream's status
					}

					for (auto& chan : falseChan->chanBundle)
					{
						chan[i].valid = 1;  // Copy downstream's status
					}

					setValid = 1;
					break;
				}
			}
		}

		if (!setValid)
		{
			for (auto& chan : trueChan->chanBundle)
			{
				chan[i].valid = 0;  // Copy downstream's status
			}

			for (auto& chan : falseChan->chanBundle)
			{
				chan[i].valid = 0;  // Copy downstream's status
			}
		}
	}
}

void MuxSGMF::recordDataStatus(vector<Channel*>& upstream)
{
	upstreamDataStatus.last = 0;
	upstreamDataStatus.lastOuter = 0;
	upstreamDataStatus.graphSwitch = 0;

	// Record upstream's data status
	for (auto& chan : upstream)
	{
		if (!chan->channel.empty())
		{
			upstreamDataStatus.last |= chan->channel.front().last;
			upstreamDataStatus.lastOuter |= chan->channel.front().lastOuter;
			upstreamDataStatus.graphSwitch |= chan->channel.front().graphSwitch;

			// If a upstream channel in keepMode, its tag is not consistent with others
			if (!chan->keepMode)
			{
				upstreamDataStatus.tag = chan->channel.front().tag;  // For SGMF, inherit tag from upstream
			}
		}
	}

	// Reset the tag of loopVar when a loop is over
	if (isLcMux && sel == false)
	{
		upstreamDataStatus.tag = 0;
	}
}

void MuxSGMF::pushOutChan()
{
	//// Send data to each channel bundle of outChan
	//vector<int> tmp;
	//for (int i = 0; i < outChan->chanBundle.size(); ++i)
	//{
	//	tmp.push_back(1);
	//}
	//outChan->get(tmp);

	uint tag = upstreamDataStatus.tag;
	outChan->get(1, tag);  // outChan is a single channel

	// Update data status
	// Note: due to pass checkUpstream, the data received by outChan must been pushed into the outChan's channel in the same cycle, 
	//		 so update the channel.back() with upstreamDataStatus is OK.
	outChan->channel.back().last = upstreamDataStatus.last;
	outChan->channel.back().lastOuter = upstreamDataStatus.lastOuter;
	outChan->channel.back().graphSwitch = upstreamDataStatus.graphSwitch;
	//outChan->channel.back().tag = upstreamDataStatus.tag;  // Inherit tag from upstream
}


//// class for MuxLcSGMF
//MuxSGMFLc::MuxSGMFLc(vector<ChanSGMF*> _trueChan, vector<ChanSGMF*> _falseChan, vector<ChanSGMF*> _downstream) :
//	MuxSGMF(_trueChan, _falseChan, _downstream)
//{
//	// In Lc of SGMF, add outChan as upstream for the downstream(loopVar) automatically
//	for (auto& chan : _downstream)
//	{
//		chan->addUpstream({ outChan }); 
//	}
//}

//bool MuxLcSGMF::checkUpstream(vector<Channel*>& upstream)
//{
//	bool ready = 1;
//
//	if (!upstream.empty())
//	{
//		for (auto& channel : upstream)
//		{
//			if (/*!channel->isCond*/ !channel->isLoopVar && !channel->valid)
//			{
//				ready = 0;
//				break;
//			}
//		}
//
//		if (ready)
//		{
//			recordDataStatus(upstream);
//		}
//	}
//
//	return ready;
//}
