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
#include "modio/detail/ModioDefines.h"

#ifdef MODIO_PLATFORM_UNREAL

	#include "Misc/AssertionMacros.h"

	#if PLATFORM_WINDOWS
		#include <SDKDDKVer.h>
		// Wrap asio in the UE4 windows platform support headers.
		#include "Windows/AllowWindowsPlatformTypes.h"
		#include "Windows/PreWindowsApi.h"
		#include "Windows/AllowWindowsPlatformAtomics.h"
	#endif

	namespace asio
	{
		namespace detail
		{
			template<typename Exception>
			void throw_exception(const Exception& e)
			{
				checkf(false, TEXT("Asio threw a exception with the message %s"), *e.what());
			}

		} // namespace detail
	} // namespace asio

	#pragma push_macro("ASIO_NO_EXCEPTIONS")
	#define ASIO_NO_EXCEPTIONS 1
	#include <asio.hpp>
	#pragma pop_macro("ASIO_NO_EXCEPTIONS")
	#if PLATFORM_WINDOWS
		#include "Windows/PostWindowsApi.h"
		#include "Windows/HideWindowsPlatformTypes.h"
		#include "Windows/HideWindowsPlatformAtomics.h"
  // Ensure that we are linking against Winhttp that we require on Windows
		#pragma comment(lib, "Winhttp.lib")
	#endif
#elif defined(_WIN32)
	// Set the proper SDK version before including asio
	#include <SDKDDKVer.h>
	// Note asio includes Windows.h.
	#include <asio.hpp>
	// Ensure that we are linking against Winhttp that we require on Windows
	#pragma comment(lib, "Winhttp.lib")
#else // _WIN32
	#include <asio.hpp>
#endif //_WIN32