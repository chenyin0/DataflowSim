#pragma once
#include "../../../../DRAMSim2/src/MultiChannelMemorySystem.h"
#include "./Spm.h"
#include "../execution/Lse.h"

namespace DFSim
{
	class Lse;

	class MemSystem
	{
	public:
		MemSystem();
		~MemSystem();

		// Lse
		uint registerLse(Lse* _lse);  // Register Lse, return Id in LseRegistry
		//bool checkReqQueueIsFull();
		bool addTransaction(MemReq _req);  // Add transaction to MemSys (Interface function for Lse)

		// SPM
		void send2Spm();
		void getFromSpm();

		void MemSystemUpdate();

		static void power_callback(double a, double b, double c, double d) {}  // Unused

	private:
		void sendBack2Lse();

	public:
		DRAMSim::MultiChannelMemorySystem* mem = nullptr;  // DRAM
		Spm* spm = nullptr;
		vector<Lse*> lseRegistry;
		vector<MemReq> reqQueue;
	private:
		uint sendPtr = 0;  // SendPtr for reqQueue, round-robin
	};


	//class DramInterface
	//{
	//public:
	//	void read_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
	//	void write_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
	//};
}