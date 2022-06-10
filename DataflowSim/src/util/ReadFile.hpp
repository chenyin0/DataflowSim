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
    };
}