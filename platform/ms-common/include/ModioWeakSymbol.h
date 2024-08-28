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
	inline void modio_profile_stub(...) {}
	__declspec(selectany) void (*modio_profile_stub_ptr)(...) = modio_profile_stub;
}

#define MODIO_WEAK(Name) __pragma(comment(linker, "/alternatename:" #Name "=modio_profile_stub"));
