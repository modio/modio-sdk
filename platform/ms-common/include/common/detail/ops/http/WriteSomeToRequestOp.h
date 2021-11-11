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
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class WriteSomeToRequestOp
		{
		public:
			WriteSomeToRequestOp(std::shared_ptr<HttpRequestImplementation> Request, Modio::Detail::Buffer DataToWrite,
								 std::weak_ptr<HttpSharedStateBase> SharedState)
				: Request(Request),
				  DataToWrite(std::move(DataToWrite)),
				  SharedState(SharedState) {};

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
					if (!WinHttpWriteData(Request->RequestHandle, DataToWrite.Data(), (DWORD)DataToWrite.GetSize(), NULL))
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http, "http write data returned system error code {}", GetLastError());
						Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
						return;
					}
					while (PinnedState->PeekHandleStatus(Request->RequestHandle) == WinHTTPCallbackStatus::Waiting)
					{
						if (Timer == nullptr)
						{
							Timer = std::make_unique<asio::steady_timer>(Modio::Detail::Services::GetGlobalContext());
							Timer->expires_after(std::chrono::milliseconds(1));
							yield Timer->async_wait(std::move(Self));
						}

					}

					switch (PinnedState->FetchAndClearHandleStatus(Request->RequestHandle))
					{
						case WinHTTPCallbackStatus::RequestError:
						//TODO: @modio-core Log the error here?
							Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
							return;
						default:
							Self.complete(Modio::ErrorCode {});
							return;
					}
				}
			}

		private:
			std::shared_ptr<HttpRequestImplementation> Request;
			Modio::Detail::Buffer DataToWrite;
			asio::coroutine CoroutineState;
			std::unique_ptr<asio::steady_timer> Timer;
			std::weak_ptr<HttpSharedStateBase> SharedState;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>