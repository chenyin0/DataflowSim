/*
Module LC

1. loop controller

*/

/*
Usage:
1. Declare:
	LC* lc0 = &LC();
	lc0->addPort({getAct}, {sendAct}, {getEnd}, {sendEnd});

2. Code insert:
	i = lc->mux(initial value, i);
	flag = i < boundary;  // => if(i < boundary)
	lc->cond->get(flag);
	lc->lcUpdate();
	i_channel->get(i);
	i = i_channel.assign() -> update; // e.g. i = i_channel.assign() + 1 (to implement i++);
*/

#pragma once
#include "../define/Define.hpp"
#include "./DataType.h"
#include "../define/Para.h"
#include "./Channel.h"

namespace DFSim
{
	class LC
	{
	public:
		//LC(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);
		LC(Channel* _cond, Channel* _getEnd, Channel* _sendEnd);
		void lcUpdate();
		int mux(int init, int update);  // select initial loop variable(e.g. i=0) or updated loop variable(e.g. i++)
		void addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);

	protected:
		void init();
		void initSelUpdate();
		void getEndUpdate();
		void sendEndUpdate();
		virtual void condUpdate();

	private:
	public:
		bool initSel = 1; // trig = 1, select initial loop variable; trig = 0, select updated loop variable;
		bool lastInitSel = 1;  // record last cycle initSel when cond disable
		uint loopNum = 0; // loop number
		uint loopEnd = 0; // the number of finished loop
		uint currLoopId = 0; // if current loop Id == loop number, loop end
		deque<uint> loopNumQ; // store each loop number
		deque<bool> getLastOuter;  // LC->cond gets a last from outer loop

	public:
		//Channel* getEnd = new Channel(2, 0);
		//Channel* sendEnd = new Channel(2, 0);
		//Channel* cond = new Channel(2, 0);  // 1) get condition result; 2) used as getActive and sendActive;
		Channel* getEnd;
		Channel* sendEnd;
		Channel* cond;  // 1) get condition result; 2) used as getActive and sendActive;
	};

/*
LcDGSF usage:

1. set graphSize after declaring; (graphSize is equal to BRAM bank depth) 

*/

	// LC for DGSF
	class LcDGSF : public LC
	{
	public:
		LcDGSF(ChanDGSF* _cond, ChanDGSF* _getEnd, ChanDGSF* _sendEnd, uint _graphSize);
		//void addPort(vector<ChanDGSF*> _getActive, vector<ChanDGSF*> _sendActive, vector<ChanDGSF*> _getEnd, vector<ChanDGSF*> _sendEnd);
		void condUpdate() override;
		uint graphSize;

	public:
		ChanDGSF* getEnd;
		ChanDGSF* sendEnd;
		ChanDGSF* cond;  // 1) get condition result; 2) used as getActive and sendActive;
	};
}