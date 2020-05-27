#include "./Registry.h"
#include "../sim/Debug.h"

using namespace DFSim;

uint Registry::moduleId = 0;
vector<RegistryTable> Registry::registryTable;

Registry::Registry()
{
}

Registry::~Registry()
{
	for (auto& entry : registryTable)
	{
		if (entry.chanPtr != nullptr)
		{
			delete entry.chanPtr;
		}
		if (entry.lcPtr != nullptr)
		{
			delete entry.lcPtr;
		}
		if (entry.muxPtr != nullptr)
		{
			delete entry.muxPtr;
		}
	}
}

int Registry::registerChan(Channel* chan)
{
	if (Registry::registryTable.size() != Registry::moduleId)
	{
		Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
		return -1;
	}
	else
	{
		RegistryTable entry;
		entry.chanPtr = chan;
		entry.moduleId = Registry::moduleId;
		registryTable.push_back(entry);
		++Registry::moduleId;

		return Registry::moduleId;
	}
}

int Registry::registerLc(Lc* lc)
{
	if (Registry::registryTable.size() != Registry::moduleId)
	{
		Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
		return -1;
	}
	else
	{
		RegistryTable entry;
		entry.lcPtr = lc;
		entry.moduleId = Registry::moduleId;
		registryTable.push_back(entry);
		++Registry::moduleId;

		return Registry::moduleId;
	}
}

int Registry::registerMux(Mux* mux)
{
	if (Registry::registryTable.size() != Registry::moduleId)
	{
		Debug::throwError("Registry moduleId is not equal to regisTable entryId!", __FILE__, __LINE__);
		return -1;
	}
	else
	{
		RegistryTable entry;
		entry.muxPtr = mux;
		entry.moduleId = Registry::moduleId;
		registryTable.push_back(entry);
		++Registry::moduleId;

		return Registry::moduleId;
	}
}

void Registry::tableInit()
{
	initLastTagQueue();

}

void Registry::initLastTagQueue()
{
	for (auto& entry : registryTable)
	{
		if (entry.chanPtr != nullptr)
		{
			Channel* chan = entry.chanPtr;
			if (chan->keepMode)
			{
				chan->lastTagQueue.resize(chan->downstream.size());  // Resize a queue for each downstream channel
				for (size_t i = 0; i < chan->lastTagQueue.size(); ++i)
				{
					uint _moduleId = chan->downstream[i]->moduleId;
					deque<bool> queue;
					chan->lastTagQueue[i] = make_pair(_moduleId, queue);
				}
			}
		}
	}
}