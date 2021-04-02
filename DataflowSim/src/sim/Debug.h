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
            if (debug_mode != Debug_mode::Turn_off)
            {
                return _output_file;
            }
            else
            {
                return _null_file;
            }
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
        void vecPrint<int>(const string name, const vector<int>& data)
        {
            if (debug_mode != Debug_mode::Turn_off)
            {
                _output_file << std::endl;
                _output_file << name << ": " << std::endl;
                for (auto i : data)
                {
                    _output_file << i << " ";
                }
                _output_file << std::endl;
            }
        }

        // Only print the last "num" elements
        template <typename T>
        void vecPrint(const string name, const vector<T>& vec, const uint num)
        {
        }

        template <>
        void vecPrint<int>(const string name, const vector<int>& data, const uint num)  
        {
            if (debug_mode != Debug_mode::Turn_off)
            {
                _output_file << std::endl;
                _output_file << name << ": " << std::endl;
                uint size = data.size();
                if (size > 0)
                {
                    uint init_index = size - 1;
                    for (size_t i = 0; i < num; ++i)
                    {
                        if (init_index >= i)
                        {
                            _output_file << data[init_index - i] << " ";
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                _output_file << std::endl;
            }
        }

        static void throwError(const string errorDescrip, const string fileName, const uint lineNum);


    public:
        Debug_mode debug_mode = Debug_mode::Print_detail;

    private:
        std::ofstream _output_file;
        std::ofstream _null_file;  // Used in Print_off, not print to log_file

        // Format parameter
        uint MemSys_SetWidth = 7;  // Default set 7
    };
}