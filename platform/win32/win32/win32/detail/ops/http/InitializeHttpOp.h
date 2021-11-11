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
#include "common/HttpCallback.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include <winhttp.h>

#include <asio/yield.hpp>
class InitializeHttpOp
{
public:
	InitializeHttpOp(std::wstring UserAgentString, std::shared_ptr<HttpSharedState> SharedState)
		: SharedState(SharedState),
		  UserAgentString(UserAgentString)
	{}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		reenter(CoroutineState)
		{
			HINTERNET CurrentSession;

			CurrentSession = WinHttpOpen(UserAgentString.c_str(), WINHTTP_ACCESS_TYPE_NO_PROXY,
										 WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);

			if ((DWORD Err = GetLastError()) != ERROR_SUCCESS)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
											"InitializeHttp open received system error code {}", Err);
				Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
				return;
			}

			*SharedState = HttpSharedState(CurrentSession);
			auto SharedStatePtr = SharedState.get();
			auto set =
				WinHttpSetOption(CurrentSession, WINHTTP_OPTION_CONTEXT_VALUE, SharedStatePtr, sizeof(std::uintptr_t));
			if (!set)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
											"initialize http set option received system error code {}", GetLastError());
			};
			if (WinHttpSetStatusCallback(CurrentSession, &ModioWinhttpStatusCallback,
										 WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING,
										 0) == WINHTTP_INVALID_STATUS_CALLBACK)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
											"initialize http set callback received system error code {}", GetLastError());
			}
			Self.complete(Modio::ErrorCode {});
		}
	}

private:
	asio::coroutine CoroutineState;
	std::shared_ptr<HttpSharedState> SharedState;
	std::wstring UserAgentString;
};
#include <asio/unyield.hpp>
