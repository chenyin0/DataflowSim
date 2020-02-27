#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
	struct Data
	{
		int value;
		bool valid;
		bool cond;
		bool last;  // loop last
		bool lastOuter; // outer loop last flag
		bool graphSwitch;
		uint cycle;

		Data() 
		{
			value = 0;
			valid = 1;
			cond = 1;
			last = 0;
			lastOuter = 0;
			graphSwitch = 0;
			cycle = 0;
		}

		void reset()
		{
			value = 0;
			valid = 0;
			cond = 0;
			last = 0;
			cycle = 0;
		}
	};
}