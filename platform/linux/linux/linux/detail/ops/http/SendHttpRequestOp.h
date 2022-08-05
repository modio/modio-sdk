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
#include "linux/HttpSharedState.h"
#include "linux/detail/ops/http/SSLConnectionWriteOp.h"
#include "linux/detail/ops/http/WaitForSSLHandshakeOp.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SendHttpRequestOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			std::unique_ptr<asio::steady_timer> SendTimer;
			Modio::Detail::DynamicBuffer Payload;

		public:
			SendHttpRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
							  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(SendHttpRequest);
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					{
						Modio::ErrorCode InitStatus;
						PinnedState->InitializeRequest(Request, InitStatus);
						if (InitStatus)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
														"Init Request Failed");
							Self.complete(InitStatus);
							return;
						}
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "Sending request",
													Request->GetParameters().GetFormattedResourcePath());
					}

					yield WaitForSSLHandshakeAsync(Request, PinnedState, std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					// Add user-agent header
					// generate payload/http data here, making sure the request does not perform URLEncoding
					Payload.AppendBuffer(Request->GetParameters().GetRequestBuffer(false));
					{
						Modio::Detail::Buffer End(2);
						End[0] = '\r';
						End[1] = '\n';
						Payload.AppendBuffer(std::move(End));
					}

					yield SSLConnectionWriteAsync(Request, Payload, SharedState, std::move(Self));
					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Request send failed, buffer dump {0}",
													(const char*) Payload.begin()->Data());
						Self.complete(ec);
						return;
					}

					Self.complete({});
					return;
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio