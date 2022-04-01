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
#include "macos/HttpSharedState.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class WriteSomeToRequestOp
		{
			std::shared_ptr<HttpRequestImplementation> Request;
			Modio::Detail::Buffer DataToWrite;
			asio::coroutine CoroutineState;
			std::unique_ptr<asio::steady_timer> PollTimer;
			std::weak_ptr<HttpSharedState> SharedState;

		public:
			WriteSomeToRequestOp(std::shared_ptr<HttpRequestImplementation> Request, Modio::Detail::Buffer DataToWrite,
								 std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  DataToWrite(std::move(DataToWrite)),
				  SharedState(SharedState) {};

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
					while (Request->WriteStreamStatus() != kCFStreamStatusError)
					{
						if (Request->WriteStreamAcceptsBytes() == true)
						{
							// Prepare the body of the request
							std::size_t DataSize = DataToWrite.GetSize();
							CFDataRef DataRef =
								CFDataCreate(kCFAllocatorDefault, (const UInt8*) DataToWrite.Data(), DataSize);
							CFStringRef Str =
								CFStringCreateWithBytes(kCFAllocatorDefault, CFDataGetBytePtr(DataRef),
														CFDataGetLength(DataRef), kCFStringEncodingUTF8, false);
							CFIndex WrittenBytes = CFWriteStreamWrite(Request->WriteStream, CFDataGetBytePtr(DataRef),
																	  CFDataGetLength(DataRef));
							CFRelease(DataRef);

							if (WrittenBytes <= 0)
							{
								CFStreamError StreamError = CFWriteStreamGetError(Request->WriteStream);
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"Error Writting to HTTPS POST connection, error code {}",
															StreamError.error);
								ec = Modio::make_error_code(Modio::HttpError::CannotOpenConnection);
								Self.complete(ec);
								CFWriteStreamClose(Request->WriteStream);
								return;
							}
							else if (WrittenBytes < DataSize)
							{
								// This case means that the stream could not receive all the bytes the payload has, it
								// needs to repeat the process
								Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
															"Partial write of {} to POST connection", WrittenBytes);

								continue;
							}
							else
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
															"Written {} to POST connection", WrittenBytes);
								Self.complete({});
								return;
							}
						}
						else
						{
							if (!PollTimer)
							{
								PollTimer = std::make_unique<asio::steady_timer>(
									Modio::Detail::Services::GetGlobalContext().get_executor());
							}

							PollTimer->expires_after(std::chrono::milliseconds(1));
							yield PollTimer->async_wait(std::move(Self));
						}
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
												"Check again POST connection");
					Self.complete(Modio::make_error_code(Modio::HttpError::ResourceNotAvailable));
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
