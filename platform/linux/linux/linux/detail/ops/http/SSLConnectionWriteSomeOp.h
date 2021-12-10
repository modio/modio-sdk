/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "linux/HttpSharedState.h"
#include "mbedtls/ssl.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SSLConnectionWriteSomeOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			std::unique_ptr<asio::steady_timer> PollTimer;
			Modio::Detail::DynamicBuffer Payload;
			int WriteCount = 0;

		public:
			SSLConnectionWriteSomeOp(std::shared_ptr<HttpRequestImplementation> Request,
									 Modio::Detail::DynamicBuffer Payload, std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState),
				  Payload(Payload)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), 0);
					return;
				}

				reenter(CoroutineState)
				{
					{
						// Make things easier by writing only the contents of the first internal chunk of the
						// dynamicBuffer
						WriteCount = mbedtls_ssl_write(&Request->SSLContext, Payload.begin()->Data(),
													   Payload.begin()->GetSize());
						while (WriteCount == MBEDTLS_ERR_SSL_WANT_READ || WriteCount == MBEDTLS_ERR_SSL_WANT_WRITE)
						{
							if (!PollTimer)
							{
								PollTimer = std::make_unique<asio::steady_timer>(
									Modio::Detail::Services::GetGlobalContext().get_executor());
							}
							PollTimer->expires_after(std::chrono::milliseconds(1));
							yield PollTimer->async_wait(std::move(Self));
							WriteCount = mbedtls_ssl_write(&Request->SSLContext, Payload.begin()->Data(),
														   Payload.begin()->GetSize());
						}

						if (WriteCount >= 0)
						{
							Self.complete({}, WriteCount);
							return;
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection), 0);
						}
					}
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename SSLWriteCallback>
		auto SSLConnectionWriteSomeAsync(std::shared_ptr<HttpRequestImplementation> Request,
										 Modio::Detail::DynamicBuffer Payload, std::weak_ptr<HttpSharedState> HttpState,
										 SSLWriteCallback&& OnWriteComplete)
		{
			return asio::async_compose<SSLWriteCallback, void(Modio::ErrorCode, std::size_t)>(
				SSLConnectionWriteSomeOp(Request, Payload, HttpState), OnWriteComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio