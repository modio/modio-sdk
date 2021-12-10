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
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
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
			std::unique_ptr<asio::steady_timer> PollTimer;
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
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), 0);
					return;
				}

				reenter(CoroutineState)
				{
					// Read into a dedicated pre-allocated buffer
					ReadCount = mbedtls_ssl_read(&Request->SSLContext, ReadChunk.Data(), ReadChunk.GetSize());
					while (ReadCount == MBEDTLS_ERR_SSL_WANT_READ || ReadCount == MBEDTLS_ERR_SSL_WANT_WRITE)
					{
						if (!PollTimer)
						{
							PollTimer = std::make_unique<asio::steady_timer>(
								Modio::Detail::Services::GetGlobalContext().get_executor());
						}
						PollTimer->expires_after(std::chrono::milliseconds(1));
						yield PollTimer->async_wait(std::move(Self));
						ReadCount =
							mbedtls_ssl_read(&Request->SSLContext, ReadChunk.Data(), ReadChunk.GetSize());
					}

					if (ReadCount > 0)
					{
						// Copy the exact number of bytes received into a new buffer and append that to the read buffer
						ReadBuffer.AppendBuffer(ReadChunk.CopyRange(ReadChunk.begin(), ReadChunk.begin() + ReadCount));
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