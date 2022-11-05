#pragma once
#include "../define/define.hpp"

using namespace std;

namespace DFSim
{
    class ReadFile
    {
    public:
        /*template<typename Type>
        static void readFile2UnifiedVector(vector<Type>& vec, ifstream& infile)
        {
            string temp_str;
            while (getline(infile, temp_str))
            {
                Type temp_data = std::stoi(temp_str);
                vec.push_back(temp_data);
            }
        }

        template<typename Type>
        static void readFile(vector<Type>& vec, const string& filePath)
        {
            ifstream infile;
            infile.open(filePath);
            DEBUG_ASSERT(infile.is_open());
            ReadFile::readFile2UnifiedVector(vec, infile);
            infile.close();
        }*/

        template<class Container>
        static void readFile2Container(typename Container& container, ifstream& infile)
        {
            string temp_str;
            while (getline(infile, temp_str))
            {
                typename Container::value_type temp_data = std::stoi(temp_str);
                container.push_back(temp_data);
            }
        }

        template<class Container>
        static void readFile(typename Container& container, const string& filePath)
        {
            ifstream infile;
            infile.open(filePath);
            DEBUG_ASSERT(infile.is_open());
            ReadFile::readFile2Container(container, infile);
            infile.close();
        }

        template<class Container>
        static void readFileByColumn(typename Container& container, const string& filePath)
        {
            ifstream infile;
            infile.open(filePath);
            DEBUG_ASSERT(infile.is_open());
            vector<string> vec;
            string temp;
            while (getline(infile, temp))
            {
                vec.push_back(temp);
            }
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                istringstream is(*it);                   
                string s;
                uint col = 0;
                while (is >> s)                          //以空格为界，把istringstream中数据取出放入到依次s中
                {
                    uint tmp = std::stoi(s.c_str());
                    if (col > container.size() - 1)
                    {
                        // Column size exceeds to the size of input container
                        DEBUG_ASSERT(false)
                    }
                    container[col].push_back(tmp);
                    col++;
                }
            }
            infile.close();
        }

        template<class Container>
        static bool readFileByColumn_blocked(typename Container& container, const string& filePath, const uint& block_line_size, uint& line_id)
        {
            ifstream infile;
            infile.open(filePath);
            DEBUG_ASSERT(infile.is_open());
            vector<string> vec;
            string temp;
            uint read_line_cnt = 0;
            uint line_ptr = 0;  // Record line ptr
            bool begin_read = false;
            while (getline(infile, temp))
            {
                if (line_ptr == line_id || begin_read)
                {
                    vec.push_back(temp);
                    ++line_id;
                    ++read_line_cnt;
                    begin_read = true;
                    if (line_id - line_ptr >= block_line_size)  // If read out a block, finish
                    {
                        break;
                        begin_read = false;
                    }
                }
                else
                {
                    ++line_ptr;
                }
            }
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                istringstream is(*it);
                string s;
                uint col = 0;
                while (is >> s)
                {
                    uint tmp = std::stoi(s.c_str());
                    if (col > container.size() - 1)
                    {
                        // Column size exceeds to the size of input container
                        DEBUG_ASSERT(false)
                    }
                    container[col].push_back(tmp);
                    col++;
                }
            }
            infile.close();

            bool file_read_complete = read_line_cnt < block_line_size ? 1 : 0;

            return file_read_complete;
        }
    };
}