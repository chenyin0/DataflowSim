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

7. If a channel is set to keepMode, it must has at least one inner loop downstream to clear its keepMode
    (Inner loop's loopVar can clear the keepMode)

8. Recommand:
    chanBase/chanDGSF/chanSGMF use assign(Channel* chan) rather than assign(uint64_t bufferId);
    lse MUST use assign()!!!

//////////////
Develop log:
    
TODO:
    20.05.15: (Done: Add drainMode for outer-loop channel)
    1. Channel need add trigger signal/trigMode, this mode is complementary with keepMode!
    (e.g. inner loop executes many times then trigger outer loop execute once)

    20.06.10:
    2. Whether ChanSGMF should support parallelism; 
    (Remember to modify the push buffer behavior for whose upstream is in keepMode in pushBuffer() ) 

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
        Channel(uint64_t _size, uint64_t _cycle);
        Channel(uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        Channel(string _moduleName, uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        virtual ~Channel();
        void initial();
        virtual void addUpstream(const vector<Channel*>& _upStream);
        void addDownstream(const vector<Channel*>& _dowmStream);
        virtual vector<int> get(const vector<int>& data);  // Return {pushSuccess, pushData, popSuccess, popData}
        virtual vector<int> get();  // Return {pushSuccess, pushData, popSuccess, popData}
        virtual int assign(uint64_t bufferId);
        virtual int assign(Channel* chan);  // Suggest use this assign function
        virtual uint64_t assign();
        virtual vector<int> pop() = 0;
        virtual vector<int> push(int data, uint64_t bufferId); // Push data and update cycle; Return {pushSuccess, pushData}
        //virtual bool checkSend(Data _data, Channel* upstream) = 0;
        virtual bool checkSend(Data data, Channel* upstream);
        //virtual bool checkSend(Data _data, Channel* _upstream) { return 0; }
        virtual void statusUpdate() = 0;
        //virtual int assign();
        virtual void sendLastTag();  // Send last tag to all the upstream channels in keepMode
        int getChanId(Channel* chan);  // Get the index of a channel in the upstream vector(It is also the index of chanBuffer, bp, lastPopVal vector)

#ifdef DEBUG_MODE
        inline uint64_t getCurrId() const
        {
            return currId;
        }
#endif

    protected:
        //virtual void checkConnect(const vector<int>& inputData);  // Check upstream and downstream can't be empty
        //virtual void checkConnect();  // Check upstream and downstream can't be empty
        virtual bool checkUpstream(uint64_t bufferId);
        virtual void pushBuffer(int data, uint64_t bufferId);
        virtual void bpUpdate();
        //virtual void bpUpdate() = 0;
        virtual void parallelize();  // Emulate hardware parallel loop unrolling
        virtual void funcUpdate();
        int aluUpdate();
        int(*fp)(vector<int*>& operand_) = nullptr;
        //int funcUpdate();
        bool checkGetLastData(uint64_t bufferId) const;

    public:
        string moduleName;
        string masterName = "None";  // If this module affiliates to a upper module, store the name of it. Default value = "None";
        ModuleType moduleTypr = ModuleType::Channel;
        uint64_t moduleId;  // RegisterTable Id
        vector<deque<Data>> chanBuffer;  // Store the input data of each upstream
        deque<Data> channel;
        //bool bp = 0;
        deque<bool> bp;  // Each inputFifo's bp (replace vector<bool>)
        vector<pair<uint64_t, deque<bool>>> lastTagQueue;  // For keepMode channel: vec<downstream chan>, pair<chanId, lastTag>
        //deque<bool> getLast; // Signify has gotten a data with last tag;
        deque<bool> getTheLastData;  // Signify whether each chanBuffer has received the last data, and won't receive a data anymore(Only used in DGSF now)
        //deque<bool> produceLast;  // Only used by loopVar, signify loopVar has generated a last tag
        bool valid = 0; // Only if all the consumer channels is not full, channel is valid
        bool enable = 1;  // Active channel

        // Channel in branch
        bool isCond = 0; // Signify current channel stores condition value of the branch (e.g. channel.element.cond)
        bool branchMode = 0; // Signify current channel is in branch divergency head (e.g. data -> T channel or F channel)
        bool channelCond = 1; // Current channel is True channel or False channel

        bool isFeedback = 0;  // Signify current channel is in a feedback loop (e.g. loopVar, or sum[i+1] = sum[1] + a)
        bool isLoopVar = 0;  // Signify current channel is a loopVar of Lc

        bool noUpstream = 0;
        bool noDownstream = 0;

        bool keepMode = 0;  // Keep data lifetime in the interface of inner and outer loop, data invalid only when inner loop over
        vector<uint64_t> keepModeDataCnt;  // Record the number of same cycle data has been received from a keepMode upstream
        bool drainMode = 0;  // Outer-loop channels in drainMode push data in chanBuffer only when inner-loop is over(the data is with last flag)

        vector<Channel*> upstream;  // If no upstream, push a nullptr in vector head
        vector<Channel*> downstream;  // If no downstream, push a nullptr in vector head

        uint64_t size;    // chanBuffer size
        uint64_t cycle; // Channel execute cycle
        uint64_t speedup = 1;  // Parallelism parameter
        vector<int> lastPopVal;  // Record last data poped by each chanBuffer
        vector<int> buffer2Alu;  // Store ChanBuffer.front().value
        ChanType chanType;
        Alu_op aluOp = Alu_op::Nop;

        int value = 0;  // Channel value, for config code
        
        // Performance profiling
        vector<uint64_t> chanBufferDataCnt;  // Record the number of data has been pushed into each chanBuffer
        uint64_t chanDataCnt = 0;  // Record the number of data has been sent from channel
        uint64_t activeCnt = 0;
        uint64_t activeClkCnt = 0;
        uint64_t subgraphId = 0;
        bool isPhysicalChan = 1;
        bool pushChannelSuccess = 0;  // For counting channel utilization

    protected:
        //uint64_t size;    // chanBuffer size
        //uint64_t cycle; // Channel execute cycle
        //uint64_t speedup;  // Parallelism parameter
        int currId = 1;    // Current threadID, start at 1
        //uint64_t chanClk = 0;  // Used for func parallelize()
        //vector<int> lastPopVal;  // Record last data poped by each chanBuffer
        vector<int*> operand;  // Point operand to corresponding aluInput[bufferId] in Registry according to the opcode
        vector<int> aluInput = { 1,1,1 };
        vector<int> constant;  // Store constant
    };


/* ChanBase:
 
    Base channel for other channel class

*/
    class ChanBase : public Channel
    {
    public:
        ChanBase(uint64_t _size, uint64_t _cycle);
        ChanBase(uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        ChanBase(string _moduleName, uint64_t _size, uint64_t _cycle, uint64_t _speedup);

        // Channel get data from the program variable
        //virtual vector<int> get(int data);  // Return {pushSuccess, pushData, popSuccess, popData}
        //virtual vector<int> get(vector<int> data);  // Return {pushSuccess, pushData, popSuccess, popData}

        // Assign channel value to the program variable
        //virtual int assign(uint64_t bufferId);
        //virtual int assign();

        virtual void statusUpdate() override;
        //bool checkSend(Data data, Channel* upstream) override;
        virtual vector<int> pop() override; // Pop the data in the channel head; Return {popSuccess, popData}

    protected:
        //vector<int> lastPopVal;  // Record last data poped by each chanBuffer
        //bool lastCycleValid = 0;

        void initial();
        //virtual void checkConnect();  // Check upstream and downstream can't be empty
        bool popLastCheck();
        virtual vector<int> popChannel(bool popReady, bool popLastReady);
        //virtual void updateCycle(bool popReady, bool popLastReady); // Update cycle in keepMode
        //bool checkUpstream(uint64_t bufferId) override;
        //virtual void pushBuffer(int data, uint64_t bufferId);
        virtual void pushChannel();
        virtual void updateDataStatus(Data& data);
        virtual bool checkDataMatch();
        //vector<int> push(int data, uint64_t bufferId); // Push data and update cycle; Return {pushSuccess, pushData}
        //void bpUpdate() override;

    public:
        //ArchType archType = ArchType::Base;
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
        ChanDGSF(uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        ChanDGSF(string _moduleName, uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        ~ChanDGSF();

    private:
        void initial();
        vector<int> push(int data, uint64_t bufferId) override;
        void pushBuffer(int data, uint64_t bufferId) override;
        void bpUpdate() override;
        //virtual bool checkGetLastData(uint64_t bufferId);
        //vector<int> popChannel(bool popReady, bool popLastReady) override;
        //void statusUpdate() override;
        //void checkGraphSwitch();
        //void sendActive();
        //void parallelize() override;

    public:
        //uint64_t speedup;  // Parallelism parameter
        //int currId;    // Current threadID
        //bool sendActiveMode = 0;  // Current channel need to active others
        bool pushBufferEnable;
        bool& popChannelEnable = enable;
        vector<ChanDGSF*> activeStream;  // Active next basic block in DGSF(switch sub-graph)

        //ArchType archType = ArchType::DGSF;

    //private:
        //vector<uint64_t> chanBufferDataCntLast;  // Record last data cnt for each chanBuffer
    };


/* ChanSGMF
    
    Support tag-based OoO execution

class ChanSGMF usage:

*/
    class ChanSGMF : public ChanBase
    {
    public:
        ChanSGMF(uint64_t _size, uint64_t _cycle);
        ChanSGMF(string moduleName, uint64_t _size, uint64_t _cycle);
        //ChanSGMF(uint64_t _size, uint64_t _cycle, uint64_t _bundleSize);
        void init();
        ~ChanSGMF();
        //void checkConnect() override;
        //void addUpstream(const vector<vector<Channel*>>& _upstreamBundle);
        //void addUpstream(const vector<Channel*>& _upstream) override;
        //void addDownstream(const vector<vector<Channel*>>& _downstreamBundle);
        //vector<int> get(vector<int> data);  // For noUpstream channel, vector data for multi-inPort: Din1, Din2, Bin...
        vector<int> get();  // vector data for multi-inPort: Din1, Din2, Bin...
        //vector<int> get(int data);  // For single channel (only Din1)
        vector<int> get(vector<int> data, uint64_t tag);  // For no upstream channel (limit no upstream channel must be a single channel)
        int assign(uint64_t bufferId) override;
        int assign(Channel* chan) override;
        vector<int> popChannel(bool popReady, bool popLastReady) override;
        //void updateCycle(bool popReady, bool popLastReady) override;

    protected:
        vector<int> push(int data, uint64_t bufferId, uint64_t tag);  // Push data into corresponding channel
        bool checkUpstream(uint64_t bufferId, uint64_t tag);
        void pushBuffer(int data, uint64_t bufferId, uint64_t tag);
        //void pushChannel(int data, uint64_t chanId, uint64_t tag);
        void pushChannel(uint64_t tag);
        void statusUpdate() override;
        void shiftDataInChanBuffer();
        void checkTagMatch();
        bool checkSend(Data _data, Channel* _upstream) override;
        //virtual void sendLastTag() override;

    public:
        //vector<deque<Data>> chanBundle;  // channel[0]: Din1, channel[1]: Din2
        deque<Data> matchQueue;  // Store the data which passes tag matching
        //vector<vector<Channel*>> upstreamBundle;  // { { Din1's upstream }, { Din2's upstream } ... }
        //vector<vector<ChanSGMF*>> downstreamBundle;  // { { to whose Din1 }, { to whose Din2 } ...}
        //deque<Data> popFifo;  // Store match ready data
        uint64_t tagSize = TAG_SIZE;  // Number of tags
        bool tagUpdateMode = 0;  // In this mode, update the data's tag when it been pushed in channel (Used in loopVar or loop feedback)

        //ArchType archType = ArchType::SGMF;

    //private:
        //uint64_t chanBundleSize = CHANNEL_BUNDLE_SIZE;  // Channel number in bundle (Din1, Din2)
        //uint64_t tagSize = TAG_SIZE;  // Number of tags
        //bool isLoopVar = 0;  // If the chanSGMF is loopVar, it need to update tag when a data is pushed.
    };


/* ChanPartialMux

    Support partial data 

class ChanPartialMux usage:
    
    1. The first channel of upstream must in isCond mode
    2. Only check active path's data status in checkDataMatch()
*/
    class ChanPartialMux : public ChanBase
    {
    public:
        ChanPartialMux(uint64_t _size, uint64_t _cycle);
        ChanPartialMux(uint64_t _size, uint64_t _cycle, uint64_t _speedup);
        ChanPartialMux(string _moduleName, uint64_t _size, uint64_t _cycle, uint64_t _speedup);

    private:
        void initial();
        virtual bool checkDataMatch() override;
        virtual void updateDataStatus(Data& data) override;
        virtual vector<int> popChannel(bool popReady, bool popLastReady) override;
    };
}