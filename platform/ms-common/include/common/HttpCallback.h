/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

#include "common/HttpSharedState.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include <winhttp.h>

static void __stdcall ModioWinhttpStatusCallback(HINTERNET InternetHandle, DWORD_PTR Context, DWORD InternetStatus,
												 LPVOID StatusInformation, DWORD StatusInformationLength)
{
	DWORD_PTR AlternativeContext = 0;
	DWORD Length = sizeof(DWORD_PTR);
	bool Success = WinHttpQueryOption(InternetHandle, WINHTTP_OPTION_CONTEXT_VALUE, &AlternativeContext, &Length);
	auto err = GetLastError();
	if (Context)
	{
		HttpSharedStateBase* SharedState = reinterpret_cast<HttpSharedStateBase*>(Context);
		WinHTTPCallbackStatus StatusCode = static_cast<WinHTTPCallbackStatus>(InternetStatus);
		if (StatusCode == WinHTTPCallbackStatus::DataAvailable)
		{
			std::uint64_t Value = *(DWORD*) StatusInformation;
			SharedState->SetHandleStatus(InternetHandle, StatusCode, (void*) Value, StatusInformationLength);
		}
		else
		{
			// Got crash on shared state is nullptr
			SharedState->SetHandleStatus(InternetHandle, StatusCode, StatusInformation, StatusInformationLength);
		}
		/*Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "Handle {:x}, status {:x}\r\n",
							(unsigned long long) InternetHandle, (unsigned long) InternetStatus);*/
	}
	else
	{
		// throw;
	}
}
