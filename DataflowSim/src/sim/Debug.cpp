#include "Debug.h"

using namespace DFSim;

Debug::Debug(string fileName)
{
	_output_file.open(fileName);

	if (_output_file.is_open())
	{
		std::cout << "file open" << std::endl;
		_output_file << DFSim::ClkDomain::getInstance()->getCurrentSystemTime() << std::endl;
	}
}

Debug::~Debug()
{
	_output_file.close();
}

void Debug::chanPrint(const string name, const Channel* channel)
{
	_output_file << std::endl;
	_output_file << name << " valid: " << channel->valid;
	_output_file << "  ";
	_output_file<< "bp: "<< channel->bp/* << std::endl*/;
	_output_file << "  ";
	_output_file << "getLast: " << !channel->getLast.empty();
	_output_file << "  ";
	_output_file << "en: " << channel->enable << std::endl;

	//switch (channel->archType)
	//{
	//	case ArchType::Base:
	//	{
	//		for (auto i : channel->channel)
	//		{
	//			_output_file << "d" << i.value << ":" << "c" << i.cycle << ":" << "l" << i.last;
	//			_output_file << " ";
	//		}
	//		_output_file << std::endl;
	//	}
	//	case ArchType::DGSF:
	//	{
	//		for (auto i : channel->channel)
	//		{
	//			_output_file << "d" << i.value << ":" << "c" << i.cycle << ":" << "l" << i.last;
	//			_output_file << ":" << "g" << i.graphSwitch;
	//			_output_file << " ";
	//		}
	//		_output_file << std::endl;
	//	}
	//	case ArchType::SGMF:
	//	{
	//		for(auto chan:channel->)
	//	}
	//}

	for (auto i : channel->channel)
	{
//#ifdef SGMF
//		_output_file << "v" << i.valid << ":" << "t" << i.tag << ":";
//#endif
		_output_file << "d" << i.value << ":" << "c" << i.cycle << ":" << "l" << i.last;
#ifdef DGSF
		_output_file << ":" << "g" << i.graphSwitch;
#endif
		_output_file << " ";
		//_output_file << "D" << i.value << ":" << "C" << i.cycle << ":" << "L" << i.last << " ";
	}
	_output_file << std::endl;
}

void Debug::chanPrint(const string name, const ChanSGMF* channel)
{
	_output_file << std::endl;
	_output_file << "**" << name << " valid: " << channel->valid;
	_output_file << "  ";
	_output_file << "bp: " << channel->bp/* << std::endl*/;
	_output_file << "  ";
	_output_file << "getLast: " << !channel->getLast.empty();
	_output_file << "  ";
	_output_file << "en: " << channel->enable << std::endl;

	for (size_t i = 0; i < channel->chanBundle.size(); ++i)
	{
		auto chan = channel->chanBundle[i];

		if (!chan.empty())
		{
			_output_file << "Chan" << i << ":" << std::endl;
			_output_file << "\t";
		}

		for (auto data : chan)
		{
			if (data.valid)
			{
				_output_file <</* "v" << data.valid << ":" <<*/ "t" << data.tag << ":";
				_output_file << "d" << data.value << ":" << "c" << data.cycle << ":" << "l" << data.last;
				_output_file << " ";
			}
		}

		if (!chan.empty() && i != channel->chanBundle.size() - 1)
		{
			_output_file << std::endl;
		}
	}

	_output_file << std::endl;
	_output_file << "matchQ:" << std::endl;
	_output_file << "\t";
	for (auto data : channel->matchQueue)
	{
		if (data.valid)
		{
			_output_file << "t" << data.tag << ":";
			_output_file <</* "d" << data.value << ":" <<*/ "c" << data.cycle << ":" << "l" << data.last;
			_output_file << " ";
		}
	}

	_output_file << std::endl;
	_output_file << "channel:" << std::endl;
	_output_file << "\t";
	for (auto data : channel->channel)
	{
		_output_file << "t" << data.tag << ":";
		_output_file <</* "d" << data.value << ":" <<*/ "c" << data.cycle << ":" << "l" << data.last;
		_output_file << " ";
	}
	_output_file << std::endl;
}

void Debug::throwError(string errorDescrip, string fileName, uint lineNum)
{
	uint clk = DFSim::ClkDomain::getInstance()->getClk();
	std::cout << std::endl;
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	std::cout << "Error: " << errorDescrip << std::endl;
	std::cout << "Clock: " << clk << std::endl;
	std::cout << "Line: " << lineNum << std::endl;
	std::cout << "File name: " << fileName << std::endl;
	DEBUG_ASSERT(false);
}