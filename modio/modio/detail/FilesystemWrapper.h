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

#ifdef MODIO_PLATFORM_NO_LSTAT
	#include <sys/stat.h>
extern "C" inline int lstat(const char* path, struct stat* outStat)
{
	return ::stat(path, outStat);
}

#endif

#ifdef MODIO_PLATFORM_UNREAL

	#include MODIO_UNREAL_PLATFORM_PREAMBLE
	#include "modio/detail/ModioCompilerMacros.h"

	DISABLE_WARNING_PUSH
	DISABLE_WARNING_OPERATOR_OPERATION

	#include "ghc/filesystem.hpp"

	DISABLE_WARNING_POP

	#include MODIO_UNREAL_PLATFORM_EPILOGUE

#else // _WIN32
	#include "ghc/filesystem.hpp"
#endif //_WIN32
