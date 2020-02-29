#include "LC.h"

using namespace DFSim;

LC::LC()
{
	init();
}

void LC::addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd)
{
	cond->addUpstream(_getActive);
	cond->addDownstream(_sendActive);
	getEnd->addUpstream(_getEnd);
	sendEnd->addDownstream(_sendEnd);
}

void LC::init()
{
	getEnd->noDownstream = 1;
	getEnd->downstream = { nullptr };

	sendEnd->noUpstream = 1;
	sendEnd->upstream = { nullptr };

	getEnd->enable = 1;
	sendEnd->enable = 1;
	cond->enable = 1;

	cond->isCond = 1;
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

	if (!loopNumQ.empty() && (loopEnd == loopNumQ.front() || !getLastOuter.empty()))
	{
		sendEnd->get(1);
		loopNumQ.pop_front();
		loopEnd = 0;

		if (!getLastOuter.empty())
			getLastOuter.pop_front();
	}
}

void LC::initSelUpdate()
{
	if (initSel && !cond->channel.empty())
	{
		initSel = 0;
	}

	if (!cond->channel.empty() && cond->channel.back().last)
	{
		initSel = 1;  // reset select initial loop variable, restart a new loop
	}
}

void LC::condUpdate()
{
	// add last tag
	if (!cond->channel.empty() && cond->channel.back().value == 0)
	{
		cond->channel.back().last = 1;  // if loop 3 times, there will be i = 0,1,2,3(last = 1!)
		//cond->last = 1;
		// store loopNum and reset it
		++loopNum;  // for additional loop with last tag
		loopNumQ.push_back(loopNum);
		loopNum = 0;

	}
	else
	{
		loopNum++; // accumulate and record the number of the launched loop 
		//cond->last = 0;
	}

	if (!cond->getLast.empty())
	{
		cond->channel.back().lastOuter = 1;  // when current LC get a last from outer loop
		cond->getLast.pop_front();
	}

	// update initSel
	initSelUpdate();
}

void LC::lcUpdate()
{
	//initSelUpdate();  //update initSel
	condUpdate();  //update cond channel
	getEndUpdate();  //update getEnd
	sendEndUpdate();  //update sendEnd
}

int LC::mux(int init, int update)
{
	return initSel ? init : update;
}