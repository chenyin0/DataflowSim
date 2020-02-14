#pragma once
#include "../define/Define.hpp"

namespace DFSim
{
	struct Data
	{
		int value;
		bool valid;
		bool cond;
		bool last;
		uint cycle;

		Data() 
		{
			value = 0;
			valid = 0;
			cond = 1;
			last = 0;
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