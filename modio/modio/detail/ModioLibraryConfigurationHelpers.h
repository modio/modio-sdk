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

#include "modio/detail/HedleyWrapper.h"

#ifndef MODIO_SEPARATE_COMPILATION
	#define MODIOSDK_API inline
#elif defined(MODIO_DLL_EXPORT)
	#define MODIOSDK_API HEDLEY_PUBLIC
#elif defined(MODIO_DLL_IMPORT)
	#define MODIOSDK_API HEDLEY_IMPORT
#else
	#define MODIOSDK_API
#endif

