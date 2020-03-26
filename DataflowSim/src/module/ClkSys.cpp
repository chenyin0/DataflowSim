#include "ClkSys.h"

namespace DFSim
{
	uint ClkDomain::_clk = 0;
	bool ClkDomain::clkAdd = 0;
	deque<bool> ClkDomain::clkStall = {};

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
			clkAdd = 1;
		}
		else
		{
			clkAdd = 0;
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

	bool ClkDomain::checkClkAdd()
	{
		return clkAdd;
	}

	const string ClkDomain::getCurrentSystemTime()
	{
		struct tm ptm;
		time_t tt;
		time(&tt);		
		localtime_s(&ptm, &tt);
		char date[60] = { 0 };
		sprintf_s(date, "%d-%02d-%02d      %02d:%02d:%02d",
			(int)ptm.tm_year + 1900, (int)ptm.tm_mon + 1, (int)ptm.tm_mday,
			(int)ptm.tm_hour, (int)ptm.tm_min, (int)ptm.tm_sec);
		return std::string(date);
	}
}