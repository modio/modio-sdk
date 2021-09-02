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

#include "common/http/HttpRequestImplementation.h"
#include "modio/core/ModioBuffer.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include <memory>

#include <asio/yield.hpp>

class ReadSomeResponseBodyOp
{
	std::shared_ptr<HttpRequestImplementation> Request;
	Modio::Detail::DynamicBuffer DataBuffer;
	std::unique_ptr<Modio::Detail::Buffer> ResponseChunkBuffer;
	std::uintmax_t BufferSize;
	std::pair<std::uintptr_t, std::uintmax_t> ExtendedStatus;
	std::weak_ptr<HttpSharedStateBase> SharedState;
	asio::coroutine CoroutineState;
	std::unique_ptr<asio::steady_timer> SendTimer;
	bool bReadComplete = false;
	WinHTTPCallbackStatus Status = WinHTTPCallbackStatus::Waiting;

public:
	ReadSomeResponseBodyOp(std::shared_ptr<HttpRequestImplementation> Request, Modio::Detail::DynamicBuffer DataBuffer,
						   std::weak_ptr<HttpSharedStateBase> SharedState)
		: Request(Request),
		  DataBuffer(DataBuffer),
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
		reenter(CoroutineState)
		{
			if (!WinHttpQueryDataAvailable(Request->RequestHandle, NULL))
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
											"query data available received system error code {}", GetLastError());
				Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
				return;
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

			{
				Status = PinnedState->FetchAndClearHandleStatus(Request->RequestHandle);
				if (Status == WinHTTPCallbackStatus::RequestError)
				{
					Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
					return;
				}
				else if (Status == WinHTTPCallbackStatus::DataAvailable)
				{
					ExtendedStatus = PinnedState->GetExtendedStatus(Request->RequestHandle);
					if (ExtendedStatus.first != 0)
					{
						BufferSize = std::max<std::uintmax_t>(ExtendedStatus.first, 512 * 1024);
						ResponseChunkBuffer = std::make_unique<Modio::Detail::Buffer>(BufferSize);
						WinHttpReadData(Request->RequestHandle, ResponseChunkBuffer->begin(), BufferSize, NULL);
						while (PinnedState->PeekHandleStatus(Request->RequestHandle) == WinHTTPCallbackStatus::Waiting)
						{
							SendTimer->expires_after(std::chrono::milliseconds(1));
							yield SendTimer->async_wait(std::move(Self));
						}
						if (PinnedState->PeekHandleStatus(Request->RequestHandle) ==
							WinHTTPCallbackStatus::ReadComplete)
						{
							bReadComplete = true;
						}
						if (PinnedState->FetchAndClearHandleStatus(Request->RequestHandle) ==
							WinHTTPCallbackStatus::RequestError)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
							return;
						}
						else
						{
							ExtendedStatus = PinnedState->GetExtendedStatus(Request->RequestHandle);
							BufferSize = ExtendedStatus.second;
							Modio::Detail::Buffer ActualData = ResponseChunkBuffer->CopyRange(0, BufferSize);
							DataBuffer.AppendBuffer(std::move(ActualData));
							Self.complete(Modio::ErrorCode {});
							return;
						}
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::GenericError::EndOfFile));
						return;
					}
				}
				else if (Status == WinHTTPCallbackStatus::ReadComplete)
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::EndOfFile));
					return;
				}
			}
		}
	}
};
#include <asio/unyield.hpp>
