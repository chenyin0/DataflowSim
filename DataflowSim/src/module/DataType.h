#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
	struct Data
	{
		Data() {}

		void reset()
		{
			value = 0;
			valid = 0;
			cond = 0;
			last = 0;
			cycle = 0;
		}

		int value = 0;
		bool valid = 0;
		bool cond = 1;
		bool last = 0;  // loop last
		bool lastOuter = 0; // outer loop last flag
		uint cycle = 0;

		// DGSF
		bool graphSwitch = 0;

		// SGMF
		uint tag = 0;
		//uint chanBundleAddr = 0;  // In multi-cycle channel, record the actual addr of the data in chanBundles
	};
}