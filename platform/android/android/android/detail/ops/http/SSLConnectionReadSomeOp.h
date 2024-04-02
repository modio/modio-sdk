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
#include "android/HttpSharedState.h"
#include "mbedtls/ssl.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/timer/ModioTimer.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SSLConnectionReadSomeOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			Modio::Detail::Timer StatusTimer;
			Modio::Detail::DynamicBuffer ReadBuffer;
			int ReadCount;
			Modio::Detail::Buffer ReadChunk;

		public:
			SSLConnectionReadSomeOp(std::shared_ptr<HttpRequestImplementation> Request,
									Modio::Detail::DynamicBuffer ReadBuffer, std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState),
				  ReadBuffer(ReadBuffer),
				  ReadCount(0),
				  ReadChunk(512 * 1024)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(SSLReadSome);
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), 0);
					return;
				}

				reenter(CoroutineState)
				{
					// Read into a dedicated pre-allocated buffer
					{
						MODIO_PROFILE_SCOPE(mbedtls_ssl_read);
						ReadCount = mbedtls_ssl_read(&Request->SSLContext, ReadChunk.Data(), ReadChunk.GetSize());
					}
					MODIO_PROFILE_PUSH(readsome_poll);
					while (ReadCount == MBEDTLS_ERR_SSL_WANT_READ || ReadCount == MBEDTLS_ERR_SSL_WANT_WRITE)
					{
						StatusTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
						yield StatusTimer.WaitAsync(std::move(Self));
						{
							MODIO_PROFILE_SCOPE(mbedtls_ssl_read);
							ReadCount = mbedtls_ssl_read(&Request->SSLContext, ReadChunk.Data(), ReadChunk.GetSize());
						}
					}
					MODIO_PROFILE_POP();
					if (ReadCount > 0)
					{
						MODIO_PROFILE_SCOPE(SSLReadSomeCopyData);
						// Copy the exact number of bytes received into a new buffer and append that to the read buffer
						ReadBuffer.AppendBuffer(ReadChunk.CopyRange(ReadChunk.begin(), ReadChunk.begin() + ReadCount));
						
						//// The section below could help for logging purposes
						// {
						// 	Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
						// 								"SSL Read bytes {0}", ReadCount);
						// 	Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
						// 								"Output: {}", ReadChunk.Data());
						// }

						Self.complete({}, ReadCount);
						return;
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"SSL Read failure {0}", ReadCount);
						Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection), 0);
						return;
					}
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename SSLReadCallback>
		auto SSLConnectionReadSomeAsync(std::shared_ptr<HttpRequestImplementation> Request,
										Modio::Detail::DynamicBuffer ReadBuffer,
										std::weak_ptr<HttpSharedState> HttpState, SSLReadCallback&& OnReadComplete)
		{
			return asio::async_compose<SSLReadCallback, void(Modio::ErrorCode, std::size_t)>(
				SSLConnectionReadSomeOp(Request, ReadBuffer, HttpState), OnReadComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
