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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class ReadSomeResponseBodyOp
		{
			ModioAsio::coroutine CoroutineState {};
			std::shared_ptr<HttpRequestImplementation> Request {};
			std::weak_ptr<HttpSharedState> SharedState {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};

		public:
			ReadSomeResponseBodyOp(std::shared_ptr<HttpRequestImplementation> Request,
								   Modio::Detail::DynamicBuffer ResponseBuffer,
								   std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState),
				  ResponseBuffer(ResponseBuffer)
			{}

			ReadSomeResponseBodyOp(ReadSomeResponseBodyOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState)),
				  ResponseBuffer(std::move(Other.ResponseBuffer))
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
					yield ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
										  std::move(Self));

					ResponseBuffer.Clear();

					// Drain up to 512KB at a time
					{
						static constexpr std::size_t MaxDrainBytes = 512 * 1024;
						std::size_t Drained = Request->AppleRequest->DrainResponseBody(ResponseBuffer, MaxDrainBytes);
						Request->ResponseBodyReceivedLength += Drained;
					}

					{
						State CurrentState = Request->AppleRequest->GetState();
						if (CurrentState == State::Failed)
						{
							Modio::ErrorCode Err = Request->AppleRequest->GetError();
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"ReadSomeResponseBodyOp failure: {}", Err.message());
							Self.complete(Err);
							return;
						}
						if (CurrentState == State::Cancelled)
						{
							Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
							return;
						}
						// EOF: task complete AND nothing left buffered. Signal end-of-file
						// so the calling code knows the body is fully consumed.
						if (CurrentState == State::Complete &&
							!Request->AppleRequest->HasBufferedBody())
						{
							Self.complete(Modio::make_error_code(Modio::GenericError::EndOfFile));
							return;
						}
					}

					Self.complete({});
					return;
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
