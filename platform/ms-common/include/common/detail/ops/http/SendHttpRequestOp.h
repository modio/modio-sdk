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
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/timer/ModioTimer.h"
#include <memory>

#include <asio/yield.hpp>

class SendHttpRequestOp
{
	asio::coroutine CoroutineState {};
	std::shared_ptr<HttpRequestImplementation> Request {};
	std::weak_ptr<HttpSharedStateBase> SharedState {};
	Modio::Detail::Timer SendTimer {};
	std::unique_ptr<std::string> Payload {};

public:
	SendHttpRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
					  std::weak_ptr<HttpSharedStateBase> SharedState)
		: Request(Request),
		  SharedState(SharedState)
	{}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode MODIO_UNUSED_ARGUMENT(ec) = {})
	{
		MODIO_PROFILE_SCOPE(SendHttpRequest);

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

			{
				MODIO_PROFILE_SCOPE(AddRequestHeaders);
				for (Modio::Detail::HttpRequestParams::Header CurrentHeader : Request->Parameters.GetHeaders())
				{
					std::string FormattedHeader = fmt::format("{}: {}\r\n", CurrentHeader.first, CurrentHeader.second);
					if (!WinHttpAddRequestHeaders(Request->RequestHandle,
												  LPCWSTR(UTF8ToWideChar(FormattedHeader).c_str()),
												  std::numeric_limits<DWORD>::max(),
												  WINHTTP_ADDREQ_FLAG_ADD))
					{
						auto err = GetLastError();
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"Adding request headers generated system error code {}", err);

						Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
						return;
					}
				}
			}

			if (Request->Parameters.GetUrlEncodedPayload())
			{
				MODIO_PROFILE_SCOPE(WinHttpSendRequest);
				Payload = std::make_unique<std::string>(Request->Parameters.GetUrlEncodedPayload().value());
				if (!WinHttpSendRequest(Request->RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										LPVOID(Payload->c_str()), DWORD(Payload->length()), DWORD(Payload->length()),
										DWORD_PTR(ContextPtr)))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"sending request received system error code {}", GetLastError());

					Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
					return;
				}
			}
			else
			{
				MODIO_PROFILE_SCOPE(WinHttpSendRequest);
				if (!WinHttpSendRequest(Request->RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										WINHTTP_NO_REQUEST_DATA, 0, Request->Parameters.GetPayloadSize(),
										DWORD_PTR(ContextPtr)))
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"sending request received system error code {}", GetLastError());

					Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
					return;
				}
			}

			while (PinnedState->PeekHandleStatus(Request->RequestHandle) == WinHTTPCallbackStatus::Waiting)
			{
				SendTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
				yield SendTimer.WaitAsync(std::move(Self));
			}

			switch (PinnedState->FetchAndClearHandleStatus(Request->RequestHandle))
			{
				case WinHTTPCallbackStatus::RequestError:
					Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
					return;

				case WinHTTPCallbackStatus::Waiting:
				case WinHTTPCallbackStatus::DataAvailable:
				case WinHTTPCallbackStatus::ReadComplete:
				case WinHTTPCallbackStatus::SendRequestComplete:
				case WinHTTPCallbackStatus::WriteComplete:
				case WinHTTPCallbackStatus::HeadersAvailable:
				default:
					Self.complete(Modio::ErrorCode {});
					return;
			}
		}
	}
};
#include <asio/unyield.hpp>
