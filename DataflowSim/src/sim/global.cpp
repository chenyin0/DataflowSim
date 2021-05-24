#include "./global.h"
#include "../define/Para.h"

using namespace DFSim;

/* App name list
SimpleFlowTest
MemoryTest
Gemm
Bfs
*/
App_name Global::app_name = App_name::Lud;
string Global::file_path = "./resource/" + App_name_convert::toString(app_name) + "/";

//** Define Arch
#ifdef Base
ArchType Global::arch = ArchType::Base;
#endif
#ifdef DGSF
ArchType Global::arch = ArchType::DGSF;
#endif
#ifdef SGMF
ArchType Global::arch = ArchType::SGMF;
#endif
//**