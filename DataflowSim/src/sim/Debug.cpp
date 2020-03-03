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
	for (auto i : channel->channel)
	{
		_output_file << i.value << ":" << i.cycle << ":" << i.last;
#ifdef DGSF
		_output_file << ":" << i.graphSwitch;
#endif
		_output_file << " ";
		//_output_file << "D" << i.value << ":" << "C" << i.cycle << ":" << "L" << i.last << " ";
	}
	_output_file << std::endl;
}