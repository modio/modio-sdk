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

// Stubs for Posix extensions that are not available on some platforms, used by parts of FMT we don't use anyways
#ifdef MODIO_USE_FMT_POSIX_STUBS

#pragma push_macro("FMT_FORCE_FALLBACK_FILE")

#ifndef FMT_FORCE_FALLBACK_FILE
		#define FMT_FORCE_FALLBACK_FILE
#endif

inline void flockfile(FILE* fp)
{
	_Lockfile(fp);
}
inline void funlockfile(FILE* fp)
{
	_Unlockfile(fp);
}

inline int getc_unlocked(FILE* fp)
{
	return fgetc(fp);
}

inline int putc_unlocked(int i, FILE* fp)
{
	return fputc(i, fp);
}

#endif

#ifndef MODIO_USE_CUSTOM_FMT
	#pragma push_macro("FMT_HEADER_ONLY")
	#ifndef FMT_HEADER_ONLY
		#define FMT_HEADER_ONLY
	#endif
#endif

#ifdef MODIO_PLATFORM_UNREAL

	#include MODIO_UNREAL_PLATFORM_PREAMBLE
	#pragma push_macro("check")
	#undef check
	#if PLATFORM_WINDOWS || PLATFORM_XBOXONEGDK || PLATFORM_XSX || PLATFORM_XB1
		#pragma warning(push)
		#pragma warning(disable : 4583)
		#pragma warning(disable : 4582)
		#pragma warning(disable : 4265)
		MODIO_DISABLE_WARNING_UNREACHABLE_CODE
	#endif
	#include <type_traits>
	#pragma push_macro("_LIBCPP_VERSION")
	#undef _LIBCPP_VERSION

	#include "fmt/chrono.h"
	#include "fmt/format.h"
	#include "fmt/printf.h"
	#include "fmt/ranges.h"
	#include "fmt/xchar.h"
	#if PLATFORM_WINDOWS || PLATFORM_XBOXONEGDK || PLATFORM_XSX || PLATFORM_XB1
		#pragma warning(pop)
	#endif
	#pragma pop_macro("_LIBCPP_VERSION")
	#pragma pop_macro("check")
	#include MODIO_UNREAL_PLATFORM_EPILOGUE

#else

	MODIO_DISABLE_WARNING_PUSH
	MODIO_DISABLE_WARNING_ASSIGNMENT_IMPLICITLY_DELETED
	MODIO_DISABLE_WARNING_CONSTRUCTOR_IMPLICITLY_DELETED
	MODIO_DISABLE_WARNING_COPY_CONSTRUCTOR_IMPLICITLY_DELETED
	MODIO_DISABLE_WARNING_CPP98_COMPAT
	MODIO_DISABLE_WARNING_DUPLICATE_ENUM
	MODIO_DISABLE_WARNING_EXIT_TIME_DESTRUCTORS
	MODIO_DISABLE_WARNING_FUNCTION_NOT_INLINED
	MODIO_DISABLE_WARNING_GLOBAL_CONSTRUCTORS
	MODIO_DISABLE_WARNING_L2R_EVALUATION_ORDER_INITIALIZER
	MODIO_DISABLE_WARNING_MISSING_NORETURN
	MODIO_DISABLE_WARNING_MOVE_ASSIGNMENT_IMPLICITLY_DELETED
	MODIO_DISABLE_WARNING_MOVE_CONSTRUCTOR_IMPLICITLY_DELETED
	MODIO_DISABLE_WARNING_PRE_CPP14_COMPAT
	MODIO_DISABLE_WARNING_RESERVED_IDENTIFIER
	MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION
	MODIO_DISABLE_WARNING_SPECTRE_MITIGATION
	MODIO_DISABLE_WARNING_STRUCTURE_PADDING
	MODIO_DISABLE_WARNING_SWITCH_ENUM
	MODIO_DISABLE_WARNING_UNSAFE_BUFFER_USAGE
	MODIO_DISABLE_WARNING_UNREACHABLE_CODE
	MODIO_DISABLE_WARNING_UNUSED_FUNCTION
	#pragma push_macro("_LIBCPP_VERSION")
	#undef _LIBCPP_VERSION
	#include "fmt/chrono.h"
	#include "fmt/format.h"
	#include "fmt/printf.h"
	#include "fmt/ranges.h"
	#include "fmt/xchar.h"
	#pragma pop_macro("_LIBCPP_VERSION")
	MODIO_DISABLE_WARNING_POP

#endif

#ifndef MODIO_USE_CUSTOM_FMT
	#pragma pop_macro("FMT_HEADER_ONLY")
#endif

#ifdef MODIO_USE_FMT_POSIX_STUBS
	#pragma pop_macro("FMT_FORCE_FALLBACK_FILE")
#endif

