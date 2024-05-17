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
/// @brief Wrapper header that enforces both the inclusion of our defines as well as SDKDDKVer.h to ensure asio gets the
/// correct windows version
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioDefines.h"

#ifdef MODIO_PLATFORM_UNREAL
	#define MODIO_WRAPPING_ASIO
	#include MODIO_UNREAL_PLATFORM_PREAMBLE

namespace asio
{
	namespace detail
	{
		template<typename Exception>
		void throw_exception(const Exception& e)
		{
			checkf(false, TEXT("Asio threw a exception with the message %hs"), e.what());
		}

	} // namespace detail
} // namespace asio

	#pragma push_macro("ASIO_NO_TYPEID")
#ifndef ASIO_NO_TYPEID
	#define ASIO_NO_TYPEID 1
#endif

	#pragma push_macro("ASIO_NO_EXCEPTIONS")
#ifndef ASIO_NO_EXCEPTIONS
	#define ASIO_NO_EXCEPTIONS 1
#endif

	#pragma push_macro("ASIO_DONT_USE_PAUSE")
#ifndef ASIO_DONT_USE_PAUSE
	#define ASIO_DONT_USE_PAUSE
#endif

	#include <asio.hpp>
	#pragma pop_macro("ASIO_DONT_USE_PAUSE")
	#pragma pop_macro("ASIO_NO_EXCEPTIONS")
	#pragma pop_macro("ASIO_NO_TYPEID")

	#include MODIO_UNREAL_PLATFORM_EPILOGUE
	#undef MODIO_WRAPPING_ASIO
#elif defined(_WIN32)
	// Set the proper SDK version before including asio
	#include <SDKDDKVer.h>
namespace asio
{
	namespace detail
	{
		template<typename Exception>
		void throw_exception(const Exception& e)
		{}

	} // namespace detail
} // namespace asio

	#pragma push_macro("ASIO_NO_TYPEID")
#ifndef ASIO_NO_TYPEID
	#define ASIO_NO_TYPEID 1
#endif

	// Note asio includes Windows.h.
	#include <asio.hpp>
	#pragma pop_macro("ASIO_NO_TYPEID")

	// Ensure that we are linking against Winhttp that we require on Windows
	#pragma comment(lib, "Winhttp.lib")
#else // _WIN32
	#ifdef ASIO_NO_EXCEPTIONS

namespace asio
{
	namespace detail
	{
		template<typename Exception>
		void throw_exception(const Exception& e)
		{}
	} // namespace detail
} // namespace asio

	#endif
	#pragma push_macro("ASIO_DONT_USE_PAUSE")
#ifndef ASIO_DONT_USE_PAUSE
	#define ASIO_DONT_USE_PAUSE
#endif

  //#pragma push_macro("ASIO_DISABLE_SOCKETS")
  //#define ASIO_DISABLE_SOCKETS 1
  //#pragma push_macro("ASIO_DISABLE_FD_SET")

//#define ASIO_DISABLE_FD_SET 1
MODIO_DIAGNOSTIC_PUSH

// Disable spurious warning about deprecated allocator<void> specialisation
MODIO_ALLOW_DEPRECATED_SYMBOLS

	#include <asio.hpp>

MODIO_DIAGNOSTIC_POP
	//#pragma pop_macro("ASIO_DISABLE_FD_SET")
	//#pragma pop_macro("ASIO_DISABLE_SOCKETS")
	#pragma pop_macro("ASIO_DONT_USE_PAUSE")
#endif //_WIN32
