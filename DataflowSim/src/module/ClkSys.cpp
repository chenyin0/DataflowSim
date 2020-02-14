#include "ClkSys.h"

namespace DFSim
{
	uint ClkDomain::_clk = 0;


	uint ClkDomain::getClk()
	{
		return _clk;
	}

	void ClkDomain::selfAdd()
	{
		_clk++;
	}

	void ClkDomain::setClk(uint clk_)
	{
		_clk = clk_;
	}

	void ClkDomain::initClk()
	{
		setClk(0);
	}

	void ClkDomain::clkUpdate()
	{
		if (clkStall.empty())
		{
			selfAdd();
		}

		resetClkStall();
	}

	void ClkDomain::addClkStall()
	{
		clkStall.push_back(1);
	}

	void ClkDomain::resetClkStall()
	{
		clkStall.clear();
	}
}