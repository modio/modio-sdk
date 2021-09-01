/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

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

