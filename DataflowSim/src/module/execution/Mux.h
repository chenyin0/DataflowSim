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
#include "../../define/Define.hpp"
#include "../../define/Para.h"
#include "./Channel.h"

namespace DFSim
{
	class Mux
	{
	public:
		//Mux(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream);
		Mux(ChanBase* _trueChan, ChanBase* _falseChan, ChanBase* _outChan);
		virtual ~Mux();
		void addPort(vector<Channel*> _trueChanUpstream, vector<Channel*> _falseChanUpstream, vector<Channel*> _outChanDownstream);
		void muxUpdate(bool sel);
		int mux(int trueData, int falseData, bool sel);
	protected:
		virtual bool checkUpstream(vector<Channel*>& upstream);
		virtual void recordDataStatus(vector<Channel*>& upstream);
		virtual void pushOutChan();
	protected:
		virtual void bpUpdate();
	public:
		ModuleType moduleTypr = ModuleType::Mux;
		uint moduleId;
		bool sel = 0;
		bool muxSuccess = 0; // If current cycle mux works, set muxSuccess to 1; Used for LC which initial value is a constant
		/*Channel* trueChan = new Channel(2, 0);
		Channel* falseChan = new Channel(2, 0);
		Channel* outChan = new Channel(2, 0);*/
		ChanBase* trueChan;
		ChanBase* falseChan;
		ChanBase* outChan;
	protected:
		Data upstreamDataStatus;
	};


	//class MuxLc : public Mux
	//{
	//public:
	//	MuxLc(vector<Channel*> _trueChan, vector<Channel*> _falseChan, vector<Channel*> _downstream);
	//private:
	//	bool checkUpstream(vector<Channel*>& upstream) override;
	//	//void bpUpdate() override;
	//};


	class MuxSGMF : public Mux
	{
	public:
		//MuxSGMF(vector<ChanSGMF*> _trueChan, vector<ChanSGMF*> _falseChan, vector<ChanSGMF*> _downstream);
		MuxSGMF(ChanSGMF* _trueChan, ChanSGMF* _falseChan, ChanSGMF* _outChan);
		~MuxSGMF();
		//void addPort(vector<ChanSGMF*> _trueChanUpstream, vector<ChanSGMF*> _falseChanUpstream, vector<ChanSGMF*> _outChanDownstream);
		//void muxUpdate(bool sel) override;
	protected:
		//bool checkUpstream(vector<Channel*>& upstream) override;
		void bpUpdate() override;
		void recordDataStatus(vector<Channel*>& upstream) override;  // Add tag inherit
		void pushOutChan() override;

	public:
		bool isLcMux = 0;  // Indicate the mux a LcMux
		//ChanSGMF* trueChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // Single channel
		//ChanSGMF* falseChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // Single channel
		//ChanSGMF* outChan = new ChanSGMF(INPUT_BUFF_SIZE, 0, 1);  // Single channel
		ChanSGMF* trueChan;  // Single channel
		ChanSGMF* falseChan;  // Single channel
		ChanSGMF* outChan;  // Single channel
	};


	//class MuxSGMFLc : public MuxSGMF
	//{
	//public:
	//	MuxSGMFLc(vector<ChanSGMF*> _trueChan, vector<ChanSGMF*> _falseChan, vector<ChanSGMF*> _downstream);
	////private:
	////	bool checkUpstream(vector<Channel*>& upstream) override;
	//};
}