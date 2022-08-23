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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include <cstdlib>
#include <memory>
#include <vector>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class ReadSomeResponseBodyOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			// The max size of the temp buffer "ReadChunk"
			constexpr static signed long ReadChunkSize = 512 * 1024;
			// A temporal buffer that reads from the CFStream
			Modio::Detail::Buffer ReadChunk;

		public:
			ReadSomeResponseBodyOp(std::shared_ptr<HttpRequestImplementation> Request,
								   Modio::Detail::DynamicBuffer ResponseBuffer,
								   std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState),
				  ResponseBuffer(ResponseBuffer),
				  ReadChunk(ReadChunkSize)
			{}

			ReadSomeResponseBodyOp(ReadSomeResponseBodyOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState)),
				  ResponseBuffer(std::move(Other.ResponseBuffer)),
				  ReadChunk(ReadChunkSize)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t BytesLastRead = 0)
			{
				std::shared_ptr<HttpSharedState> PinnedState = SharedState.lock();

				if (PinnedState == nullptr || PinnedState->IsClosing())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));
					/*Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Reading response body",
												Request->GetParameters().GetFormattedResourcePath());*/

					// Clear out the destination buffer
					ResponseBuffer.Clear();

					// In case some data remained within Request->ResponseDataBuffer, return those bytes first
					if (Request->ResponseDataBuffer.size())
					{
						Request->ResponseBodyReceivedLength = Request->ResponseDataBuffer.size();
						// While we have any prebuffered response data from when we were looking for the response
						// headers, stuff that data into the response

						while (Modio::Optional<Buffer> PrebufferedResponseData = Request->ResponseDataBuffer.TakeInternalBuffer())
                        {
                            ResponseBuffer.AppendBuffer(PrebufferedResponseData.take().value());
                        }
					}
					// In some cases, it is possible that the ReadStream has not "ended" but the Stream
					// remains open. If the CFReadStreamRead is called without bytes, it would wait for
					// a looooong time
					else if (Request->ReadStreamHasBytes() == true)
					{
						// Read bytes into the ResponseDataBuffer
						CFIndex ReadBytes = CFReadStreamRead(Request->ReadStream, ReadChunk.Data(), ReadChunkSize);

						if (ReadBytes <= 0)
						{
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Error, Modio::LogCategory::Http,
								"ReadSomeResponseBodyOp stream could not be read or an error has ocurred");
							Modio::make_error_code(Modio::HttpError::RequestError);
							break;
						}

						BytesLastRead = ReadBytes;
						Request->ResponseBodyReceivedLength += ReadBytes;

						ResponseBuffer.AppendBuffer(ReadChunk.CopyRange(0, BytesLastRead));
					}
					
                    // We need to know signal to SDK's upper layers that there are no more bytes available
                    // in the ResponseBody. Using this method, we avoid using the function "HandleChunkedEncoding"
                    if (Request->ReadStreamStatus() == kCFStreamStatusAtEnd)
					{
						ec = Modio::make_error_code(Modio::GenericError::EndOfFile);
					}

					if (ec)
					{
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
