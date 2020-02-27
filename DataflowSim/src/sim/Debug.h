#pragma once
#include "../define/Define.hpp"
#include "../module/ClkSys.h"
#include "../module/Channel.h"

namespace DFSim
{
	class Debug
	{
	public:
		Debug(string fileName);

		~Debug();

		std::ofstream& getFile()
		{
			return _output_file;
		}

		void chanPrint(const string name, const Channel* channel);

		template <typename T>
		void vecPrint(const string name, const vector<T>& vec)
		{
		}

		template <>
		void vecPrint<int>(string name, const vector<int>& data)
		{
			_output_file << std::endl;
			_output_file << name <<": "<< std::endl;
			for (auto i : data)
			{
				_output_file << i << " ";
			}
			_output_file << std::endl;
		}


	private:
		std::ofstream _output_file;
	};
}