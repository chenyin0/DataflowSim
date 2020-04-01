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

#pragma once
#include "../../define/Define.hpp"
#include "../DataType.h"
#include "../../define/Para.h"

namespace DFSim
{
/* Channel:

	1. Base class for all the execution channels and Lse(regard Lse as a kind of channel, 
	in order to intergrate Lse with execution channels by up/downstream mechanism);

	2. Class Channel still needs to be refined. (Remove the variable of execution(e.g. branchMode, isFeedback ...) to class ChanBase);

class Channel usage:
	Need additionally set the parameters below in manual, after constructing a class object:
	1. bool branchMode; // signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
	2. bool isCond; // signify current channel stores condition value of the branch
	3. bool channelCond; // current channel is True channel or False channel
	4. vector<Channel*> upstream  // if no upstream, set noUpstream;
								  // for constructing const variable;
	5. vector<Channel*> downstream  // if no downstream, set noDownstream;
	6. bool keepMode; // keep data lifetime in interact between inner/outer loop;
	7. bool isFeedback  // In a feedback loop
	8. bool isLoopVar  // loopVar in Lc
*/
	class Channel 
	{
	public:
		Channel(uint _size, uint _cycle);
		virtual ~Channel();
		virtual void addUpstream(const vector<Channel*>& _upStream);
		void addDownstream(const vector<Channel*>& _dowmStream);
		virtual bool checkSend(Data _data, Channel* upstream) = 0;
		//virtual bool checkSend(Data _data, Channel* _upstream) { return 0; }
		virtual void statusUpdate() = 0;
		//virtual int assign();

	protected:
		virtual void checkConnect();  // Check upstream and downstream can't be empty
		virtual bool checkUpstream() = 0;
		virtual void bpUpdate() = 0;

	public:
		deque<Data> channel;
		bool bp = 0;
		deque<bool> getLast; // Signify has gotten a data with last tag;
		bool valid = 0; // Only if all the consumer channels is not full, channel is valid
		bool enable = 1;  // Active channel

		// Channel in branch
		bool branchMode = 0; // Signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
		bool isCond = 0; // Signify current channel stores condition value of the branch (e.g. channel.element.cond)
		bool channelCond = 1; // Current channel is True channel or False channel

		bool isFeedback = 0;  // Signify current channel is in a feedback loop (e.g. loopVar, or sum[i+1] = sum[1] + a)
		bool isLoopVar = 0;  // Signify current channel is a loopVar of Lc

		bool noUpstream = 0;
		bool noDownstream = 0;

		bool keepMode = 0;  // Keep data lifetime in the interface of inner and outer loop, data invalid only when inner loop over

		vector<Channel*> upstream;  // If no upstream, push a nullptr in vector head
		vector<Channel*> downstream;  // If no downstream, push a nullptr in vector head

	protected:
		uint size;	// Channel size
		uint cycle; // Channel execute cycle
	};


/* ChanBase:
 
	Base channel for other channel class

*/
	class ChanBase : public Channel
	{
	public:
		ChanBase(uint _size, uint _cycle);

		// Channel get data from the program variable
		virtual vector<int> get(int data);  // Return {pushSuccess, pushData, popSuccess, popData}

		// Assign channel value to the program variable
		virtual int assign();

		virtual void statusUpdate() override;
		bool checkSend(Data data, Channel* upstream) override;
		virtual vector<int> pop(); // Pop the data in the channel head; Return {popSuccess, popData}

	protected:
		int lastPopVal = 0;  // Record last data poped by channel
		//bool lastCycleValid = 0;

		void initial();
		//virtual void checkConnect();  // Check upstream and downstream can't be empty
		bool popLastCheck();
		virtual vector<int> popChannel(bool popReady, bool popLastReady);
		virtual void updateCycle(bool popReady, bool popLastReady); // Update cycle in keepMode
		bool checkUpstream() override;
		virtual void pushChannel(int data);
		vector<int> push(int data); // Push data and update cycle; Return {pushSuccess, pushData}
		void bpUpdate() override;

	public:
		ArchType archType = ArchType::Base;
	};


/* ChanDGSF:

	Support sub-graph switch

class ChanDGSF usage:
	1. sendActiveMode;  // current channel need to send active signal to others to swith sub-graph
	2. vector<Channel> activeStream;
	3. bool enable;   // in DGSF, the beginning channel and the channels within a basic block need be set to 1 in manual;
					  // in other archetecture, set defaultly;
*/
	class ChanDGSF : public ChanBase
	{
	public:
		ChanDGSF(uint _size, uint _cycle, uint _speedup);
		~ChanDGSF();
		void statusUpdate() override;

	private:
		vector<int> popChannel(bool popReady, bool popLastReady);
		void sendActive();

	public:
		uint speedup;  // Parallelism parameter
		int currId;	// Current threadID
		bool sendActiveMode = 0;  // Current channel need to active others
		vector<ChanDGSF*> activeStream;  // Active next basic block in DGSF(switch sub-graph)

		ArchType archType = ArchType::DGSF;
	};


/* ChanSGMF
	
	Support tag-based OoO execution

class ChanSGMF usage:

*/
	class ChanSGMF : public ChanBase
	{
	public:
		ChanSGMF(uint _size, uint _cycle);
		ChanSGMF(uint _size, uint _cycle, uint _bundleSize);
		void init();
		~ChanSGMF();
		void checkConnect() override;
		void addUpstream(const vector<vector<Channel*>>& _upstreamBundle);
		void addUpstream(const vector<Channel*>& _upstream) override;
		//void addDownstream(const vector<vector<Channel*>>& _downstreamBundle);
		vector<int> get(vector<int> data);  // vector data for multi-inPort: Din1, Din2, Bin...
		vector<int> get(int data);  // For single channel (only Din1)
		vector<int> get(int data, uint tag);  // For no upstream channel (limit no upstream channel must be a single channel)
		int assign(uint chanId);
		vector<int> popChannel(bool popReady, bool popLastReady) override;
		void updateCycle(bool popReady, bool popLastReady) override;
		vector<int> push(int data, uint chanId, uint tag);  // Push data into corresponding channel
		bool checkUpstream(uint chanId); 
		void pushChannel(int data, uint chanId, uint tag);

		void statusUpdate() override;
		bool checkSend(Data _data, Channel* _upstream) override;

	public:
		vector<deque<Data>> chanBundle;  // channel[0]: Din1, channel[1]: Din2
		deque<Data> matchQueue;  // Store the data pass tag matching
		vector<vector<Channel*>> upstreamBundle;  // { { Din1's upstream }, { Din2's upstream } ... }
		//vector<vector<ChanSGMF*>> downstreamBundle;  // { { to whose Din1 }, { to whose Din2 } ...}
		//deque<Data> popFifo;  // Store match ready data

		bool tagUpdateMode = 0;  // In this mode, update the data's tag when it been pushed in channel (Used in loopVar or loop feedback)

		ArchType archType = ArchType::SGMF;

	private:
		uint chanBundleSize = CHANNEL_BUNDLE_SIZE;  // Channel number in bundle (Din1, Din2)
		uint chanSize;
		//bool isLoopVar = 0;  // If the chanSGMF is loopVar, it need to update tag when a data is pushed.
	};
}