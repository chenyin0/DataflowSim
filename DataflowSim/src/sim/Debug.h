#pragma once
#include "../define/Define.hpp"
#include "../module/ClkSys.h"
#include "../module/execution/Channel.h"
#include "../module/execution/Lse.h"

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
		void chanPrint(const string name, const ChanSGMF* channel);
		void lsePrint(const string _name, const Lse* _lse);
		void memSysPrint(const MemSystem* _memSys);

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

		static void throwError(string errorDescrip, string fileName, uint lineNum);

	private:
		std::ofstream _output_file;
	};
}