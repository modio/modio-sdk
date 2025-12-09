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
#include "common/UTF16Support.h"
#include "common/http/HttpRequestImplementation.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/timer/ModioTimer.h"

#include <asio/yield.hpp>
#include <memory>

class ReadHttpResponseHeadersOp
{
	std::weak_ptr<HttpSharedStateBase> SharedState {};
	std::shared_ptr<HttpRequestImplementation> Request {};
	ModioAsio::coroutine CoroutineState {};
	Modio::Detail::Timer Timer {};

public:
	ReadHttpResponseHeadersOp(std::shared_ptr<HttpRequestImplementation> Request,
							  std::weak_ptr<HttpSharedStateBase> SharedState)
		: SharedState(SharedState),
		  Request(Request)
	{}

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode MODIO_UNUSED_ARGUMENT(ec) = {})
	{
		MODIO_PROFILE_SCOPE(ReadHttpResponseHeaders);

		std::shared_ptr<HttpSharedStateBase> PinnedState = SharedState.lock();
		if (PinnedState == nullptr || PinnedState->IsClosing())
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}

		reenter(CoroutineState)
		{
			if (!WinHttpReceiveResponse(Request->RequestHandle, nullptr))
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
											"ReceiveResponse returned system error code {}", GetLastError());

				Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
				return;
			}

			while (PinnedState->PeekHandleStatus(Request->RequestHandle) == WinHTTPCallbackStatus::Waiting)
			{
				Timer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
				yield Timer.WaitAsync(std::move(Self));
			}

			switch (PinnedState->FetchAndClearHandleStatus(Request->RequestHandle))
			{
				case WinHTTPCallbackStatus::RequestError:
					Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
					return;
				case WinHTTPCallbackStatus::Waiting:
				case WinHTTPCallbackStatus::HeadersAvailable:
				case WinHTTPCallbackStatus::DataAvailable:
				case WinHTTPCallbackStatus::ReadComplete:
				case WinHTTPCallbackStatus::SendRequestComplete:
				case WinHTTPCallbackStatus::WriteComplete:
				default:

				{
					DWORD StatusCode = 0;
					DWORD BufferSize = sizeof(StatusCode);
					if (WinHttpQueryHeaders(
							Request->RequestHandle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
							WINHTTP_HEADER_NAME_BY_INDEX, &StatusCode, &BufferSize, WINHTTP_NO_HEADER_INDEX))
					{
						Request->ResponseCode = StatusCode;
						ParseHeadersInResponseBuffer();
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"Unable to retrieve response headers for HTTP request");
						Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
						return;
					}
				}
					Self.complete(Modio::ErrorCode {});
					return;
			}
		}
	}

	bool ParseHeadersInResponseBuffer()
	{
		DWORD DWSize = 0;
		// For reference: https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
		bool Result = false;
		WinHttpQueryHeaders(Request->RequestHandle, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, nullptr,
							&DWSize, WINHTTP_NO_HEADER_INDEX);

		// Allocate memory for the buffer.
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			Modio::Detail::Buffer LPOutBuffer(DWSize);

			// Now, use WinHttpQueryHeaders to retrieve the header.
			Result = WinHttpQueryHeaders(Request->RequestHandle, WINHTTP_QUERY_RAW_HEADERS_CRLF,
										 WINHTTP_HEADER_NAME_BY_INDEX, LPOutBuffer.begin(), &DWSize, WINHTTP_NO_HEADER_INDEX);

			if (Result == false) 
			{
				return false;
			}

			std::string ParseBuffer;
			bool SkipOther = true;
			
			// It is necessary to "skip" one other because the buffer returns as a "wide char" which is
			// 16 bits unitcode, double the size of the char
			for (auto val : LPOutBuffer) 
			{
				SkipOther = !SkipOther;
					
				if (SkipOther == true)
				{ 
					continue;
				}
				ParseBuffer += char(val);
			}
				
			Request->ResponseHeaders = Modio::Detail::String::ParseRawHeaders(ParseBuffer);
		}

		return Result;
	}
};

#include <asio/unyield.hpp>