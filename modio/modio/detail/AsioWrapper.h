#pragma once
/// @brief Wrapper header that enforces both the inclusion of our defines as well as SDKDDKVer.h to ensure asio gets the correct windows version
#include "modio/detail/ModioDefines.h"

#ifdef MODIO_PLATFORM_UNREAL
	#if PLATFORM_WINDOWS
		#include <SDKDDKVer.h>
		// Wrap asio in the UE4 windows platform support headers.
		#include "Windows/AllowWindowsPlatformTypes.h"
		#include "Windows/PreWindowsApi.h"
		#include "Windows/AllowWindowsPlatformAtomics.h"
	#endif
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