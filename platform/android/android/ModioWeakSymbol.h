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

extern "C"
{
#ifdef __clang__
	inline void modio_profile_stub(...) {};
#else
	static inline void modio_profile_stub(...) {};
#endif
}

#define MODIO_WEAK(Name) __attribute__((weak, alias("modio_profile_stub")))
