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
#include "common/HttpSharedState.h"
#include "common/HttpCallback.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include <winhttp.h>

#include <asio/yield.hpp>
class InitializeHttpOp
{
public:
	InitializeHttpOp(std::wstring UserAgentString, std::shared_ptr<HttpSharedStateBase> SharedState)
		: SharedState(SharedState),
		  UserAgentString(UserAgentString)
	{}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		reenter(CoroutineState)
		{
			HINTERNET CurrentSession;

			CurrentSession = WinHttpOpen(UserAgentString.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
										 WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);

			if (GetLastError() != ERROR_SUCCESS)
			{
				Self.complete(Modio::make_error_code(Modio::HttpError::HttpNotInitialized));
				return;
			}

			*SharedState = HttpSharedStateBase(CurrentSession);
			WinHttpSetOption(CurrentSession, WINHTTP_OPTION_CONTEXT_VALUE, SharedState.get(), sizeof(std::uintptr_t));
			WinHttpSetStatusCallback(CurrentSession, (WINHTTP_STATUS_CALLBACK) &ModioWinhttpStatusCallback,
									 WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING, 0);

			Self.complete(Modio::ErrorCode {});
		}
	}

private:
	asio::coroutine CoroutineState;
	std::shared_ptr<HttpSharedStateBase> SharedState;
	std::wstring UserAgentString;
};
#include <asio/unyield.hpp>
