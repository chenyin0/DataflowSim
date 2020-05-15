#pragma once
#include "./Channel.h"
#include "../mem/MemSystem.h"

namespace DFSim
{
	class MemSystem;

	class Lse : public Channel
	{
	public:
		Lse(uint _size, uint _cycle, MemSystem* _memSys);
		~Lse();

		void get(bool _isWrite, uint _addr);  // Load/store addr
		// Trigger mode (e.g. inner loop executes many times then trigger outer loop execute once,
		// or upper pe updates local reg many times then sends the data to lse once)
		void get(bool _isWrite, uint _addr, uint& trig_cnt);  
		uint assign();  // Return corresponding addr
		bool checkSend(Data _data, Channel* upstream) override;
		void callback(MemReq _req);  // Callback func for MemSys

	protected:
		bool checkUpstream() override;
		bool push(bool _isWrite, uint _addr);  // Push memory access request into reqQueue
		//void push(bool _isWrite, uint _addr, bool& trigger);  // Trigger mode
		void pushReqQ(bool _isWrite, uint _addr);
		void pop();  // Pop memory request from reqQueue when the data has sent to the downstream channel
		void statusUpdate() override;
		bool sendReq(MemReq _req);  // Call MemSystem's addTransaction func
		void bpUpdate() override;

	public:
		// Store memory request; As for load back data, push the corresponding reqQueueIndex into the deque<Data> channel;
		// MemReq: keep req; Data: keep status;
		vector<pair<MemReq, Data>> reqQueue;  

	protected:
		uint lseId;
		uint lastPopVal = 0;
		uint sendPtr = 0;
		MemSystem* memSys = nullptr;
	};
}