#pragma once
#include "../define/define.hpp"

using namespace std;

namespace DFSim
{
    class ReadFile
    {
    public:
        template<typename Type>
        static void readFile2UnifiedVector(vector<Type>& vec, ifstream& infile)
        {
            //uint k = 0;
            string temp_str;
            while (getline(infile, temp_str))
            {
                Type temp_data = std::stoi(temp_str);
                //vec[k] = temp_data;
                vec.push_back(temp_data);
                //Simulator::Array::MemoryData::getInstance()->setInitialized(k);
                //k++;
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
        }
    };
}