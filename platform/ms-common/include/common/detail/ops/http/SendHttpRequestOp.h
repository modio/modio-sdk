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
#include "common/http/HttpRequestImplementation.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include <memory>

#include <asio/yield.hpp>

class SendHttpRequestOp
{
	asio::coroutine CoroutineState;
	std::shared_ptr<HttpRequestImplementation> Request;
	std::weak_ptr<HttpSharedStateBase> SharedState;
	std::unique_ptr<asio::steady_timer> SendTimer;
	std::unique_ptr<std::string> Payload;
public:
	SendHttpRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
					  std::weak_ptr<HttpSharedStateBase> SharedState)
		: Request(Request),
		  SharedState(SharedState)
	{}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		std::shared_ptr<HttpSharedStateBase> PinnedState = SharedState.lock();
		if (PinnedState == nullptr || PinnedState->IsClosing())
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}
		auto ContextPtr = PinnedState.get();
		reenter(CoroutineState)
		{
			{
				Modio::ErrorCode InitStatus;
				PinnedState->InitializeRequest(Request, InitStatus);
				if (InitStatus)
				{
					Self.complete(InitStatus);
					return;
				}
			}
			for (Modio::Detail::HttpRequestParams::Header CurrentHeader : Request->Parameters.GetHeaders())
			{
				std::string FormattedHeader = fmt::format("{}: {}\r\n", CurrentHeader.first, CurrentHeader.second);
				if (!WinHttpAddRequestHeaders(Request->RequestHandle, (LPCWSTR) UTF8ToWideChar(FormattedHeader).c_str(),
											  (DWORD) -1L, WINHTTP_ADDREQ_FLAG_ADD))
				{
					auto err = GetLastError();
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"Adding request headers generated system error code {}", err);
					
					Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
					return;
				}
			}

			if (Request->Parameters.GetUrlEncodedPayload())
			{
				Payload = std::make_unique<std::string>(Request->Parameters.GetUrlEncodedPayload().value());
				if (!WinHttpSendRequest(Request->RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										(void*) Payload->c_str(), (DWORD) Payload->length(), (DWORD) Payload->length(),
										(DWORD_PTR) ContextPtr))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"sending request received system error code {}", GetLastError());
					
					Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
					return;
				}
			}
			else
			{
				if (!WinHttpSendRequest(Request->RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										WINHTTP_NO_REQUEST_DATA, 0, Request->Parameters.GetPayloadSize(), (DWORD_PTR) ContextPtr))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"sending request received system error code {}", GetLastError());

					Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
					return;
				}
			}

			if (SendTimer == nullptr)
			{
				SendTimer = std::make_unique<asio::steady_timer>(Modio::Detail::Services::GetGlobalContext());
			}

			while (PinnedState->PeekHandleStatus(Request->RequestHandle) == WinHTTPCallbackStatus::Waiting)
			{
				SendTimer->expires_after(std::chrono::milliseconds(1));
				yield SendTimer->async_wait(std::move(Self));
			}

			switch (PinnedState->FetchAndClearHandleStatus(Request->RequestHandle))
			{
				case WinHTTPCallbackStatus::RequestError:
					Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
					return;
				default:
					Self.complete(Modio::ErrorCode {});
					return;
			}
		}
	}
};
#include <asio/unyield.hpp>
