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
	Mux mux0 = new Mux({lc->var}, {empty or other channel*}, {lc->var});

	int i = init val;
	int i = mux0->mux(i, init val, lc->sel);
	mux0->muxUpdate(lc->sel);
	lc->loopVar->get(i);
	i = lc->loopVar.assign() -> update; // e.g. i = lc->loopVar.assign() + 1 (to implement i++);
	lc->sel = i < boundary;  // => if(i < boundary)
	lc->lcUpdate();
*/

#pragma once
#include "../define/Define.hpp"
#include "./DataType.h"
#include "../define/Para.h"
#include "./Channel.h"
#include "./Mux.h"

namespace DFSim
{
	class LC
	{
	public:
		//LC(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);
		LC(Channel* _var, Channel* _getEnd, Channel* _sendEnd);
		void lcUpdate(bool newSel);
		void selUpdate(bool newSel);
		//int mux(int init, int update);  // Select initial loop variable(e.g. i=0) or updated loop variable(e.g. i++)
		void addPort(vector<Channel*> _getActive, vector<Channel*> _sendActive, vector<Channel*> _getEnd, vector<Channel*> _sendEnd);
		// initDepend: external initial loop var; updateDepend: for variable loop boundary and variable loop updating
		void addDependence(vector<Channel*> _initDepend, vector<Channel*> _updateDepend);  
	protected:
		void init();
		//void initSelUpdate();
		void getEndUpdate();
		void sendEndUpdate();
		virtual void loopUpdate();

	private:
	public:
		bool sel = 0;  // Sel for loop control, default set to select initial value
		//bool initSel = 1; // trig = 1, select initial loop variable; trig = 0, select updated loop variable;
		uint loopNum = 0; // loop number
		uint loopEnd = 0; // the number of finished loop
		uint currLoopId = 0; // If current loop Id == loop number, loop end
		deque<uint> loopNumQ; // Store each loop number
		deque<bool> getLastOuter;  // LC->cond gets a last from outer loop

	public:
		Channel* getEnd;
		Channel* sendEnd;
		Channel* loopVar;  // 1) Get condition result; 2) Used as getActive and sendActive;
		MuxLC* mux = new MuxLC({ loopVar }, { }, { loopVar });
	};


/*
LcDGSF usage:

1. set graphSize after declaring; (graphSize is equal to BRAM bank depth) 

*/
	// LC for DGSF
	class LcDGSF : public LC
	{
	public:
		LcDGSF(ChanDGSF* _loopVar, ChanDGSF* _getEnd, ChanDGSF* _sendEnd, uint _graphSize);
		//void addPort(vector<ChanDGSF*> _getActive, vector<ChanDGSF*> _sendActive, vector<ChanDGSF*> _getEnd, vector<ChanDGSF*> _sendEnd);
		void loopUpdate() override;
		uint graphSize;

	public:
		//ChanDGSF* getEnd;
		//ChanDGSF* sendEnd;
		ChanDGSF* loopVar;  // 1) Get condition result; 2) Used as getActive and sendActive;
	};
}