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

#include "http/HttpRequestImplementation.h"
#include "ios/AppleHttpRequest.h"
#include "ios/HttpSharedState.h"
MODIO_DISABLE_WARNING_PUSH
MODIO_DISABLE_WARNING_SIGNED_UNSIGNED_INTEGER_CONVERSION
#include <httpparser/response.h>
MODIO_DISABLE_WARNING_POP
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/timer/ModioTimer.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>

		class ReadHttpResponseHeadersOp
		{
			ModioAsio::coroutine CoroutineState {};
			std::shared_ptr<HttpRequestImplementation> Request {};
			std::weak_ptr<HttpSharedState> SharedState {};
			Modio::Detail::Timer StatusTimer {};

		public:
			ReadHttpResponseHeadersOp(std::shared_ptr<HttpRequestImplementation> Request,
									  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			ReadHttpResponseHeadersOp(ReadHttpResponseHeadersOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState)),
				  StatusTimer(std::move(Other.StatusTimer))
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t /*BytesLastRead*/ = 0)
			{
				using State = Modio::Detail::Apple::HttpRequest::State;

				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					// Spin until headers arrive, the task fails, or it gets cancelled.
					while (true)
					{
						{
							State CurrentState = Request->AppleRequest->GetState();
							if (CurrentState == State::Failed)
							{
								Modio::ErrorCode Err = Request->AppleRequest->GetError();
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"Request failed before headers received: {}",
															Err.message());
								Self.complete(Err);
								return;
							}
							if (CurrentState == State::Cancelled)
							{
								Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
								return;
							}
							if (CurrentState == State::HeadersReceived ||
								CurrentState == State::Complete)
							{
								break;
							}
						}

						StatusTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
						yield StatusTimer.WaitAsync(std::move(Self));
					}

					{
						Request->ResponseCode = Request->AppleRequest->GetResponseCode();
						Request->ParsedResponseHeaders.statusCode = Request->ResponseCode;
						auto Headers = Request->AppleRequest->GetResponseHeaders();
						Request->ParsedResponseHeaders.headers.reserve(Headers.size());
						for (auto& KV : Headers)
						{
							Request->ParsedResponseHeaders.headers.emplace_back(
								httpparser::Response::HeaderItem {std::move(KV.first), std::move(KV.second)});
						}
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Response Headers received OK with response code: {}",
												Request->ResponseCode);
					Self.complete({});
					return;
				}
			}
		};

#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
