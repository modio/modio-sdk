#pragma once

#ifdef MODIO_DLL
	#ifdef MODIO_BUILD_DLL
		/*Enabled as "export" while compiling the dll project*/
		#define DLLEXPORT __declspec(dllexport)
	#else
		/*Enabled as "import" in the Client side for using already created dll file*/
		#define DLLEXPORT __declspec(dllimport)
	#endif
#elif !MODIO_PLATFORM_UNREAL
	#define DLLEXPORT
#endif


#ifndef MODIO_SEPARATE_COMPILATION
	#define MODIOSDK_API DLLEXPORT inline
#else
	#define MODIOSDK_API DLLEXPORT
#endif

