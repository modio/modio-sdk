/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma comment(lib, "ws2_32.lib")

#include "common/HttpSharedState.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include <winhttp.h>

MODIO_DISABLE_WARNING_PUSH
MODIO_DISABLE_WARNING_UNREFERENCED_FUNCTION

static void __stdcall ModioWinhttpStatusCallback(HINTERNET InternetHandle, DWORD_PTR MODIO_UNUSED_ARGUMENT(Context),
												 DWORD InternetStatus,
	LPVOID StatusInformation, DWORD StatusInformationLength) noexcept
{
	MODIO_PROFILE_SCOPE(WinhttpCallback);

	// Can't use try-catch when we're building for UE (which has exceptions disabled)
	#ifndef MODIO_PLATFORM_UNREAL
	try
	{
	#endif

		// There's only ever a single SharedStateHolder for the entire life of the program
		std::shared_ptr<HttpSharedStateBase> SharedState = SharedStateHolder::Get().SharedStatePtr.lock();

		if (SharedState)
		{
			WinHTTPCallbackStatus StatusCode = static_cast<WinHTTPCallbackStatus>(InternetStatus);
			if (InternetStatus & DWORD(WinHTTPCallbackStatus::DataAvailable))
			{
				std::uint64_t Value = *reinterpret_cast<DWORD*>(StatusInformation);
				SharedState->SetHandleStatus(InternetHandle, StatusCode, reinterpret_cast<void*>(Value), StatusInformationLength);
			}
			else
			{
				if (InternetStatus & DWORD(WinHTTPCallbackStatus::RequestError))
				{
					WINHTTP_ASYNC_RESULT* Result = static_cast<WINHTTP_ASYNC_RESULT*>(StatusInformation);
					Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Http,
												"Function {:x} returned error code {:x}\r\n",
												Result->dwResult, Result->dwError);
				}

				SharedState->SetHandleStatus(InternetHandle, StatusCode, StatusInformation, StatusInformationLength);
			}
		}
	#ifndef MODIO_PLATFORM_UNREAL
	}
	catch (...)
	{
		// prevent UB by catching any errors thrown within a C callback
	}
	#endif
}

MODIO_DISABLE_WARNING_POP
