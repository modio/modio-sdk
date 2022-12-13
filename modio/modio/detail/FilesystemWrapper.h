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
	#if defined(MODIO_PLATFORM_CUSTOM_FS)
		#pragma push_macro("GHC_OS_CUSTOM")
		#define GHC_OS_CUSTOM

		#include "ghc/filesystem.hpp"

		#include "file/FileSystemStubs.h"

		#pragma pop_macro("GHC_OS_CUSTOM")
	#else
		#include "ghc/filesystem.hpp"
	#endif
namespace Modio
{
	namespace filesystem = ghc::filesystem;
} // namespace Modio

DISABLE_WARNING_POP

	#include MODIO_UNREAL_PLATFORM_EPILOGUE

#elif defined(MODIO_PLATFORM_CUSTOM_FS)

	// Backport of std::filesystem to support C++14/C++11, with special defines for providing platform stubs in a
	// separate file
	#pragma push_macro("GHC_OS_CUSTOM")
	#define GHC_OS_CUSTOM

	#include "ghc/filesystem.hpp"

	#include "file/FileSystemStubs.h"

	#pragma pop_macro("GHC_OS_CUSTOM")
namespace Modio
{
	namespace filesystem = ghc::filesystem;
} // namespace Modio

#elif defined(MODIO_USE_STD_FILESYSTEM)

	#include <filesystem>

namespace Modio
{
	namespace filesystem = std::filesystem;
} // namespace Modio
#else

	// Backport of std::filesystem to support C++14/C++11
	#include "ghc/filesystem.hpp"
namespace Modio
{
	namespace filesystem = ghc::filesystem;
} // namespace Modio
#endif
