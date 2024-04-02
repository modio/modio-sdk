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
#include "android/detail/ops/http/SSLConnectionWriteSomeOp.h"
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
		class SSLConnectionWriteOp
		{
			asio::coroutine CoroutineState;
			std::weak_ptr<HttpSharedState> SharedState;
			Modio::Detail::DynamicBuffer Payload;
			std::shared_ptr<HttpRequestImplementation> Request;

		public:
			SSLConnectionWriteOp(std::shared_ptr<HttpRequestImplementation> Request,
								 Modio::Detail::DynamicBuffer Payload, std::weak_ptr<HttpSharedState> SharedState)
				: SharedState(SharedState),
				  Payload(std::move(Payload)),
				  Request(Request)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t BytesLastWritten = 0)
			{
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();
				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					while (!ec && Payload.size() > 0)
					{
						yield SSLConnectionWriteSomeAsync(Request, Payload, SharedState, std::move(Self));
						Payload.consume(BytesLastWritten);
					}
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else
					{
						Self.complete({});
						return;
					}
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename SSLWriteCallback>
		auto SSLConnectionWriteAsync(std::shared_ptr<HttpRequestImplementation> Request,
									 Modio::Detail::DynamicBuffer Payload, std::weak_ptr<HttpSharedState> HttpState,
									 SSLWriteCallback&& OnWriteComplete)
		{
			return asio::async_compose<SSLWriteCallback, void(Modio::ErrorCode)>(
				SSLConnectionWriteOp(Request, Payload, HttpState), OnWriteComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio