#pragma once
#include "../util/util.hpp"
#include "../define/Define.hpp"
#include "../module/execution/Channel.h"
#include "../module/execution/Lc.h"
#include "../module/execution/Mux.h"

/*
Module Register

TODO:
1. Unify chanPtr, lcPtr, muxPtr to modulePtr

*/

namespace DFSim
{
	struct RegistryTable
	{
		Channel* chanPtr = nullptr;
		Lc* lcPtr = nullptr;
		Mux* muxPtr = nullptr;
		uint moduleId = 0;
	};

	class Registry
	{
	public:
		Registry();
		~Registry();
		static int registerChan(Channel* chan);
		static int registerLc(Lc* lc);
		static int registerMux(Mux* mux);
		void tableInit();

	private:
		void initLastTagQueue();  // Initial last tag vec for keepMode channel

	private:
		static uint moduleId;
		static vector<RegistryTable> registryTable;
	};

}