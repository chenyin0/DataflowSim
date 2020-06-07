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
		entry.moduleType = ModuleType::Channel;
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
		entry.moduleType = ModuleType::Lc;
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
		entry.moduleType = ModuleType::Mux;
		registryTable.push_back(entry);
		++Registry::moduleId;

		return Registry::moduleId;
	}
}

void Registry::tableInit()
{
	initChannel();
}

void Registry::initLastTagQueue(Channel* _chan)
{
	//for (auto& entry : registryTable)
	//{
	//	if (entry.chanPtr != nullptr)
	//	{
	//		Channel* chan = entry.chanPtr;
	//		if (chan->keepMode)
	//		{
	//			chan->lastTagQueue.resize(chan->downstream.size());  // Resize a queue for each downstream channel
	//			for (size_t i = 0; i < chan->lastTagQueue.size(); ++i)
	//			{
	//				uint _moduleId = chan->downstream[i]->moduleId;
	//				deque<bool> queue;
	//				chan->lastTagQueue[i] = make_pair(_moduleId, queue);
	//			}
	//		}
	//	}
	//}

	if (_chan->keepMode)
	{
		_chan->lastTagQueue.resize(_chan->downstream.size());  // Resize a queue for each downstream channel
		for (size_t i = 0; i < _chan->lastTagQueue.size(); ++i)
		{
			uint _moduleId = _chan->downstream[i]->moduleId;
			deque<bool> queue;
			_chan->lastTagQueue[i] = make_pair(_moduleId, queue);
		}
	}
}

void Registry::initChanBuffer(Channel* _chan)
{
	if (!_chan->noUpstream)
	{
		uint upstreamSize = _chan->upstream.size();
		_chan->chanBuffer.resize(upstreamSize);
		_chan->chanBufferDataCnt.resize(upstreamSize);

		// If channel is SGMF, resize each buffer of chanBuffer
		if (_chan->chanType == ChanType::Chan_SGMF)
		{
			for (auto& buffer : _chan->chanBuffer)
			{
				buffer.resize(_chan->size);
			}
		}
	}
	else
	{
		_chan->chanBuffer.resize(1);
		_chan->chanBufferDataCnt.resize(1);
		if (_chan->chanType == ChanType::Chan_SGMF)
		{
			for (auto& buffer : _chan->chanBuffer)
			{
				buffer.resize(_chan->size);
			}
		}
	}

}

void Registry::initBp(Channel* _chan)
{
	if (!_chan->noUpstream)
	{
		uint upstreamSize = _chan->upstream.size();
		_chan->bp.resize(upstreamSize);
	}
	else
	{
		_chan->bp.resize(1);
	}
}

void Registry::initLastPopVal(Channel* _chan)
{
	if (!_chan->noUpstream)
	{
		uint upstreamSize = _chan->upstream.size();
		_chan->lastPopVal.resize(upstreamSize);
	}
	else
	{
		_chan->lastPopVal.resize(1);
	}
}

void Registry::initChannel()
{
	for (auto& entry : registryTable)
	{
		if (entry.moduleType == ModuleType::Channel)
		{
			initChanBuffer(entry.chanPtr);
			initLastTagQueue(entry.chanPtr);
			initBp(entry.chanPtr);
			initLastPopVal(entry.chanPtr);
		}
	}
}

