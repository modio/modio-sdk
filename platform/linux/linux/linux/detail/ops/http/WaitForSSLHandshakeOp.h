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
#include "linux/HttpSharedState.h"
#include "mbedtls/ssl.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class WaitForSSLHandshakeOp
		{
		public:
			WaitForSSLHandshakeOp(std::shared_ptr<HttpRequestImplementation> Request, std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				int HandshakeStatus = 0;
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				reenter(CoroutineState)
				{
					while ((HandshakeStatus = mbedtls_ssl_handshake(&Request->SSLContext)))
					{
						if (HandshakeStatus == MBEDTLS_ERR_SSL_WANT_READ ||
							HandshakeStatus == MBEDTLS_ERR_SSL_WANT_WRITE)
						{
							if (PollTimer == nullptr)
							{
								PollTimer = std::make_unique<asio::steady_timer>(
									Modio::Detail::Services::GetGlobalContext().get_executor());
							}
							PollTimer->expires_after(std::chrono::milliseconds(1));
							yield PollTimer->async_wait(std::move(Self));
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Error {} received when trying to establish SSL handshake",
														HandshakeStatus);
							Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
							return;
						}
					}

					Self.complete({});
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;
			std::unique_ptr<asio::steady_timer> PollTimer;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
		};

		template<typename SSLHandshakeCallback>
		auto WaitForSSLHandshakeAsync(std::shared_ptr<HttpRequestImplementation> Request,
									  std::weak_ptr<HttpSharedState> HttpState,
									  SSLHandshakeCallback&& OnHandshakeComplete)
		{
			return asio::async_compose<SSLHandshakeCallback, void(Modio::ErrorCode)>(
				WaitForSSLHandshakeOp(Request, HttpState), OnHandshakeComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio