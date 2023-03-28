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

#include "modio/detail/hedley.h"

#ifndef MODIO_NODISCARD
	#define MODIO_NODISCARD HEDLEY_WARN_UNUSED_RESULT
#endif

#ifndef MODIO_NORETURN
	#define MODIO_NORETURN HEDLEY_NO_RETURN
#endif

#ifndef MODIO_UNREACHABLE
	#define MODIO_UNREACHABLE HEDLEY_UNREACHABLE
#endif

#ifndef MODIO_DIAGNOSTIC_PUSH
	#define MODIO_DIAGNOSTIC_PUSH HEDLEY_DIAGNOSTIC_PUSH
#endif

#ifndef MODIO_DIAGNOSTIC_POP
	#define MODIO_DIAGNOSTIC_POP HEDLEY_DIAGNOSTIC_POP
#endif

#ifndef MODIO_ALLOW_UNUSED_FUNCTIONS
	#define MODIO_ALLOW_UNUSED_FUNCTIONS HEDLEY_DIAGNOSTIC_DISABLE_UNUSED_FUNCTION
#endif

#ifndef MODIO_ALLOW_DEPRECATED_SYMBOLS
	#define MODIO_ALLOW_DEPRECATED_SYMBOLS HEDLEY_DIAGNOSTIC_DISABLE_DEPRECATED
#endif

#ifndef MODIO_DEPRECATED
	#if MODIO_DISABLE_ALL_DEPRECATIONS
		#define MODIO_DEPRECATED(...)
	#else
		#define MODIO_DEPRECATED(Since, Replacement) HEDLEY_DEPRECATED_FOR(Since,Replacement)
	#endif
#endif

#ifndef MODIO_FALL_THROUGH
	#define MODIO_FALL_THROUGH HEDLEY_FALL_THROUGH
#endif