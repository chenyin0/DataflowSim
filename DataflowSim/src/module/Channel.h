/*
Module Channel:

1. Channels are inserted between each basic block
	{input channels} -> Basic block -> {output/input channels} -> Basic block

2. Data update its cycles once been pushed in the input channels;

3. Output channels receive the data only when:
	1) All of the input channels are ready;
	2) The cycles of Data in input channels head is not less than the system clock;

4. Channels update cycle:
	new cycle = max{input1_cycle, input2_cycle, ..., system cycle} + channel_cycle

5. Channel pop rule:
	1) for inner most variety(lifetime = 1): enable == 1 && downstream_bp == 0;
	2) for outer loop variety(lifetime = loop cycle): innerloop == end && enable == 1 && downstream_bp == 0;

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
		Channel(vector<Channel*> _upstream, vector<Channel*> _downstream, uint _size, int speedup, uint cycle);

		// channel get data from the program variable
		void get(int data);
		
		// assign channel value to the program variable
		int assign(Channel* c);
	
	private:
		void initial();
		//Data cycleUpdate();
		bool checkUpstream();
		bool checkClk(uint clk);
		void pushChannel(int data, uint clk);
		void push(int data); // push data and update cycle
		void bpUpdate();
		void statusUpdate();
		void sendActive();
		void pop(); // pop the data in the channel head

	public:
		deque<Data> channel;
		bool bp;
		bool valid; // only if all the consumer channels is not full, channel is valid
		bool enable;  // active channel
		uint size;	// channel size
		uint cycle; // channel execute cycle

		int speedup;  // parallelism parameter
		int currId;	// current threadID

		// channel in branch
		bool branchMode; // signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
		bool isCond; // signify current channel stores condition value of the branch
		bool channelCond; // current channel is True channel or False channel

		// current channel need to active others
		bool sendActiveMode;

		vector<Channel*> upstream;
		vector<Channel*> downstream;
		vector<Channel*> activeStream;  // active next basic block in DGSF(switch sub-graph)
		vector<deque<Data>> req;
	};
}