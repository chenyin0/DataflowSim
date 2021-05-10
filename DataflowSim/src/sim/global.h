#pragma once
#include "../define/Define.hpp"
#include "../module/EnumType.h"

namespace DFSim
{
    class Global 
    {
    public:
        static App_name app_name;
        static string file_path;
        static ArchType arch;
    };
}