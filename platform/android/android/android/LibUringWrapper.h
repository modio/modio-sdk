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

// Unreal has some imports that define these values, then they are 
// refefined in the liburing header. For that reason, this wrapper
// helps to avoid those warnings/errors.
	#ifdef RWF_HIPRI
	#undef RWF_HIPRI
	#endif

	#ifdef RWF_DSYNC
	#undef RWF_DSYNC
	#endif

	#ifdef RWF_SYNC
	#undef RWF_SYNC
	#endif

	#ifdef RWF_NOWAIT
	#undef RWF_NOWAIT
	#endif

	#ifdef RWF_APPEND
	#undef RWF_APPEND
	#endif
#endif

#include "liburing.h"
