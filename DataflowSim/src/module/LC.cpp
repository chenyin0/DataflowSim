#include "LC.h"

using namespace DFSim;

LC::LC(Channel* _loopVar, Channel* _getEnd, Channel* _sendEnd) : loopVar(_loopVar), getEnd(_getEnd), sendEnd(_sendEnd)
{
	init();
}

void LC::addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd)
{
	loopVar->addUpstream(_getActive);
	loopVar->addDownstream(_sendActive);
	getEnd->addUpstream(_getEnd);
	sendEnd->addDownstream(_sendEnd);

	// Add getActive to LC Mux's trueChan and falseChan
	mux->trueChan->addUpstream(_getActive);
	mux->falseChan->addUpstream(_getActive);
}

void LC::addDependence(vector<Channel*> _initDepend, vector<Channel*> _updateDepend)
{
	mux->falseChan->addUpstream(_initDepend);
	mux->trueChan->addUpstream(_updateDepend);
}

void LC::init()
{
	getEnd->noDownstream = 1;
	getEnd->downstream = { nullptr };

	sendEnd->noUpstream = 1;
	sendEnd->upstream = { nullptr };

	getEnd->enable = 1;
	sendEnd->enable = 1;
	loopVar->enable = 1;

	loopVar->isCond = 1;
}

void LC::getEndUpdate()
{
	getEnd->get(1);

	if (!getEnd->channel.empty() && getEnd->channel.back().lastOuter)
		getLastOuter.push_back(1);

	if (!getEnd->channel.empty())
	{
		//getEnd->channel.pop_front();
		++loopEnd;
	}
}

void LC::sendEndUpdate()
{
	if (sendEnd->valid && sendEnd->enable)
	{
		sendEnd->pop();
		sendEnd->statusUpdate();
	}

	if (!loopNumQ.empty() && loopEnd == loopNumQ.front()/*(loopEnd == loopNumQ.front() || !getLastOuter.empty())*/)
	{
		sendEnd->get(1);
		loopNumQ.pop_front();
		loopEnd = 0;

		if (!getLastOuter.empty())
			getLastOuter.pop_front();
	}
}

void LC::loopUpdate()
{
	// cond valid, signify cond will sends out a data
	if (loopVar->valid)
	{
		loopNum++;
		if (sel == 0)
		{
			loopNumQ.push_back(loopNum);
			loopNum = 0;
		}
	}

	// Add last tag
	if (!loopVar->channel.empty() && sel == 0)
	{
		loopVar->channel.front().last = 1;  // If loop 3 times, there will be i = 0,1,2(last = 1!)
	}

	if (!loopVar->getLast.empty())
	{
		loopVar->channel.back().lastOuter = 1;  // When current LC get a last from outer loop
		loopVar->getLast.pop_front();
	}
}

void LC::selUpdate(bool newSel)
{
	if (mux->muxSuccess)
	{
		sel = newSel;
	}
}

void LC::lcUpdate(bool newSel)
{
	selUpdate(newSel);
	//initSelUpdate();  //update initSel
	loopUpdate();  //Update cond channel
	getEndUpdate();  //Update getEnd
	sendEndUpdate();  //Update sendEnd
}


// class LcDGSF
LcDGSF::LcDGSF(ChanDGSF* _loopVar, ChanDGSF* _getEnd, ChanDGSF* _sendEnd, uint _graphSize) : 
	LC(_loopVar, _getEnd, _sendEnd), loopVar(_loopVar)/*, getEnd(_getEnd), sendEnd(_sendEnd)*/, graphSize(_graphSize)
{
	init();
}

void LcDGSF::loopUpdate()
{
	// cond valid, signify cond will sends out a data
	if (loopVar->valid)
	{
		loopNum++;
		if (sel == 0)
		{
			loopNumQ.push_back(loopNum);
			loopNum = 0;
		}
	}

	// add last tag
	if (!loopVar->channel.empty() && sel == 0)
	{
		loopVar->channel.front().last = 1;  // If loop 3 times, there will be i = 0,1,2,3(last = 1!)
		////cond->last = 1;
		//// store loopNum and reset it
		////++loopNum;  // for additional loop with last tag
		//loopNumQ.push_back(loopNum);
		//loopNum = 0;
	}

	// When 1)loopNum = graphSize, or 2)current loop is over, set graphSwitch to 1
	if (loopNum == graphSize || (!loopVar->channel.empty() && loopVar->channel.front().last == 1))
	{
		loopVar->channel.front().graphSwitch = 1;
	}

	if (!loopVar->getLast.empty())
	{
		loopVar->channel.back().lastOuter = 1;  // When current LC get a last from outer loop
		loopVar->getLast.pop_front();
	}

	//// update initSel
	//initSelUpdate();
}