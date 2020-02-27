/*
Module Channel:

1. Channels are inserted between each basic block
	{input channels} -> Basic block -> {output/input channels} -> Basic block

2. Data update its cycles at once been pushed in the input channels;

3. Channel is valid only when:
	1) Be enable
	2) Not empty
	3) Data cycle <= system clk
	4) All of the downstream channel is not full(BP!=1)

4. Downstream channels receive the data only when:
	1) All of the input channels are valid;

5. Channels update cycle:
	new cycle = max{input1_cycle, input2_cycle, ..., system cycle} + channel_cycle

6. Channel pop rule:
	1) for inner most variety(lifetime = 1): 
		last cycle's valid == 1(signify has sent a data successfully)
	2) for outer loop variety(lifetime = loop cycle): 
		last cycle's valid == 1 && all the downstream has gotten a data which last == 1;

//////////////
Develop log:
	
*/

/*
Usage: 
	Need additionally set the parameters below in manual, after constructing a class object:
	1. bool branchMode; // signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
	2. bool isCond; // signify current channel stores condition value of the branch
	3. bool channelCond; // current channel is True channel or False channel
	4. sendActiveMode;  // current channel need to send active signal to others to swith sub-graph
	5. vector<Channel> activeStream;
	6. bool enable; // in DGSF, the beginning channel need be set to 1 in manual; in other archetecture, set defaultly;
	7. vector<Channel*> upstream  // if no upstream, set noUpstream; 
								  // for constructing const variable;
	8. vector<Channel*> downstream  // if no downstream, set noDownstream;
	9. bool keepMode; // keep data lifetime in interact between inner/outer loop;
*/

#pragma once
#include "../define/Define.hpp"
#include "./DataType.h"
#include "../define/Para.h"

namespace DFSim
{
	class Channel 
	{
	public:
		//Channel();
		Channel(uint _size, uint speedup, uint cycle);

		void addUpstream(const vector<Channel*>& _upStream);
		void addDownstream(const vector<Channel*>& _dowmStream);

		// channel get data from the program variable
		vector<int> get(int data);  // return {pushSuccess, pushData, popSuccess, popData}
		
		// assign channel value to the program variable
		//int assign(Channel* c);
		int assign();

		bool checkUpstream();
	
	private:
		int lastVal = 0;  // only use for the data which last = 1, replace this data's value to lastVal 
		bool lastCycleValid = 0;
		//deque<bool> hasSent;  // whenever channel status is valid, push a tag in this queue

		void initial();
		//Data cycleUpdate();
		void checkConnect();  // check upstream and downstream can't be empty
		//bool checkUpstream();
		//bool checkClk(uint clk);
		void pushChannel(int data, uint clk);
		vector<int> push(int data); // push data and update cycle; Return {pushSuccess, pushData}
		void bpUpdate();
	public:
		void statusUpdate();
		//void collectBp();
	private:
		void sendActive();
	public:
		vector<int> pop(); // pop the data in the channel head; Return {popSuccess, popData}

	public:
		deque<Data> channel;
		bool bp;
		deque<bool> getLast; // signify has gotten a data with last tag;
		bool valid = 0; // only if all the consumer channels is not full, channel is valid
		bool enable;  // active channel
		uint size;	// channel size
		uint cycle; // channel execute cycle

		uint speedup;  // parallelism parameter
		int currId;	// current threadID

		// channel in branch
		bool branchMode; // signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
		bool isCond; // signify current channel stores condition value of the branch (lc->cond)
		bool channelCond; // current channel is True channel or False channel

		// current channel need to active others
		bool sendActiveMode = 0;

		bool noUpstream = 0;
		bool noDownstream = 0;

		//deque<bool> hasReceived; // current channel has received a data;
		bool keepMode = 0;  // keep data lifetime in the interface of inner and outer loop, data invalid only when inner loop over

		vector<Channel*> upstream;  // if no upstream, push a nullptr in vector head
		vector<Channel*> downstream;  // if no downstream, push a nullptr in vector head
		vector<Channel*> activeStream;  // active next basic block in DGSF(switch sub-graph)
		//vector<deque<Data>> req;
	};
}