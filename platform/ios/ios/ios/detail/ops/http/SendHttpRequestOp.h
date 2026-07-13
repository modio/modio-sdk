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
#include "ios/HttpSharedState.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SendHttpRequestOp
		{
			ModioAsio::coroutine CoroutineState {};
			std::shared_ptr<HttpRequestImplementation> Request {};
			std::weak_ptr<HttpSharedState> SharedState {};

		public:
			SendHttpRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
							  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
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
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Sending request: {}",
													Request->GetParameters().GetFormattedResourcePath());
					}

					// Kick off the NSURLSessionTask. From here, response headers and
					// body bytes will arrive on the delegate queue. The SDK reads them
					// out via the subsequent ReadHttpResponseHeadersOp /
					// ReadSomeResponseBodyOp / WriteSomeToRequestOp polling.
					//
					// Any transport-level failure (DNS, TLS, connection refused)
					// surfaces as a state transition to Failed and is picked up by
					// the next op.
					Request->AppleRequest->Start();

					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio