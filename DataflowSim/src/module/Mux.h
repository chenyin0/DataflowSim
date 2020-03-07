/*
Module Mux

1. Module Mux is used for 1) LC mux; 2) Branch mux

*/
/*
Usage:
	Example code:
	if(sel) 
	{
		sum = a + b;
	}
	else 
	{
		sum = a + c;
	}

	// Code
	Channel* A, B, C, Out, Sum;
	Out->addDownstream(Sum);

	int a = A->assign();
	int b = B->assign();
	int c = C->assign();
	int trueData = a + b;
	int falseData = a + c;
	bool sel = f(...);

	Mux* mux0 = new Mux({A, B}, {A, C}, Out);
	mux0->muxUpdate(sel);
	int sum = mux0->mux(trueData, falseData, sel);
	Sum->get(sum);
*/

#pragma once
#include "../define/Define.hpp"
#include "./Channel.h"

namespace DFSim
{
	class Mux
	{
	public:
		Mux(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream);
		void muxUpdate(bool sel);
		int mux(int trueData, int falseData, bool sel);
	private:
		virtual bool checkUpstream(vector<Channel*>& upstream);
	//protected:
		virtual void bpUpdate();
	public:
		bool muxSuccess = 0; // If current cycle mux works, set muxSuccess to 1; Used for LC which initial value is a constant
		Channel* trueChan = new Channel(2, 0);
		Channel* falseChan = new Channel(2, 0);
		Channel* outChan = new Channel(2, 0);
	};

	class MuxLC : public Mux
	{
	public:
		MuxLC(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream);
	private:
		bool checkUpstream(vector<Channel*>& upstream) override;
		void bpUpdate() override;
	};
}