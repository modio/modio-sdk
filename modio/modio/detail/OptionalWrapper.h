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
#ifdef MODIO_PLATFORM_UNREAL
	#pragma warning(push)
	#pragma warning(disable : 4583)
	#pragma warning(disable : 4582)
	#include "tl/optional.hpp"
	#pragma warning(pop)
#else
	#include "tl/optional.hpp"
#endif