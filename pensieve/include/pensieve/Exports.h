#pragma once

#ifdef OS_WINDOWS
#ifdef PNSV_DLL
    #define API_PNSV __declspec(dllexport)
#else
	#define API_PNSV __declspec(dllimport)
#endif
#endif

#ifdef OS_LINUX
    #define API_PNSV
#endif