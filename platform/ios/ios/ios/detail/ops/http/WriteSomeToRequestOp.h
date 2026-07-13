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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/timer/ModioTimer.h"
#include <cstring>
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class WriteSomeToRequestOp
		{
			std::shared_ptr<HttpRequestImplementation> Request {};
			Modio::Detail::Buffer DataToWrite;
			std::size_t BytesWritten = 0;
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::Timer StatusTimer {};
			std::weak_ptr<HttpSharedState> SharedState {};

		public:
			WriteSomeToRequestOp(std::shared_ptr<HttpRequestImplementation> Request,
								 Modio::Detail::Buffer DataToWrite,
								 std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  DataToWrite(std::move(DataToWrite)),
				  SharedState(SharedState)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
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
					while (BytesWritten < DataToWrite.GetSize())
					{
						{
							State CurrentState = Request->AppleRequest->GetState();
							if (CurrentState == State::Failed)
							{
								Modio::ErrorCode Err = Request->AppleRequest->GetError();
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"Upload failed mid-write: {}", Err.message());
								Self.complete(Err);
								return;
							}
							if (CurrentState == State::Cancelled)
							{
								Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
								return;
							}
						}

						if (Request->AppleRequest->CanAcceptBodyBytes())
						{
							// Attempt a push inside an inner scope so the locals
							// don't escape it. If the push consumes any bytes,
							// we continue to the top of the loop without ever
							// reaching a yield in this scope.
							bool MadeProgress = false;
							{
								const std::size_t Remaining = DataToWrite.GetSize() - BytesWritten;
								std::size_t Accepted = Request->AppleRequest->WriteRequestBody(
									DataToWrite.Data() + BytesWritten, Remaining);
								if (Accepted > 0)
								{
									BytesWritten += Accepted;
									Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
																"Pushed {} body bytes ({} of {})",
																Accepted, BytesWritten, DataToWrite.GetSize());
									MadeProgress = true;
								}
							}
							if (MadeProgress)
							{
								continue;
							}
							// Stream reported space but accepted nothing
							// Fall through to the yield below to retry next tick.
						}

						// Either no buffer capacity, or the write accepted 0 bytes.
						// Yield to the polling timer and try again. This yield sits
						// in the while-loop scope, which has no preceding locals,
						// so the switch-case expansion is safe.
						StatusTimer.ExpiresAfter(Modio::Detail::Constants::Configuration::PollInterval);
						yield StatusTimer.WaitAsync(std::move(Self));
					}

					Self.complete({});
					return;
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
