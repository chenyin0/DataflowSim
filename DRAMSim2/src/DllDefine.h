#pragma once

#ifdef DRAMSIM_EXPORT
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif