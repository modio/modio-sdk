/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *   
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "linux/HttpSharedState.h"
#include "linux/detail/ops/http/SSLConnectionReadSomeOp.h"
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
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t BytesLastRead = -1)
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
					if (Request->ResponseDataBuffer.size())
					{
						Request->ResponseBodyReceivedLength = Request->ResponseDataBuffer.size();
						// Clear out the destination
						ResponseBuffer.Clear();
						// While we have any prebuffered response data from when we were looking for the response
						// headers, stuff that data into the response

						if (Modio::Optional<std::size_t> ExpectedLength = Request->GetContentLength())
						{
							while (Modio::Optional<Buffer> PrebufferedResponseData =
									   Request->ResponseDataBuffer.TakeInternalBuffer())
							{
								ResponseBuffer.AppendBuffer(PrebufferedResponseData.take().value());
							}

							if (Request->ResponseBodyReceivedLength >= ExpectedLength.value())
							{
								Self.complete(Modio::make_error_code(Modio::GenericError::EndOfFile));
								return;
							}
						}
						else
						{
							Self.complete(HandleChunkedEncoding(Request->ResponseDataBuffer, ResponseBuffer));
							return;
						}
						Self.complete({});
						return;
					}
					else
					{
						yield SSLConnectionReadSomeAsync(Request, Request->ResponseDataBuffer, SharedState,
														 std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}
						else
						{
							
							Request->ResponseBodyReceivedLength += BytesLastRead;
							if (Modio::Optional<std::size_t> ExpectedLength = Request->GetContentLength())
							{
								while (Modio::Optional<Buffer> PrebufferedResponseData =
										   Request->ResponseDataBuffer.TakeInternalBuffer())
								{
									ResponseBuffer.AppendBuffer(PrebufferedResponseData.take().value());
								}

								if (Request->ResponseBodyReceivedLength >= ExpectedLength.value())
								{
									Self.complete(Modio::make_error_code(Modio::GenericError::EndOfFile));
									return;
								}
							}
							else
							{
								Self.complete(HandleChunkedEncoding(Request->ResponseDataBuffer, ResponseBuffer));
								return;
							}
							Self.complete({});
							return;
						}
					}
				}
			}

			// TODO: @modio-nx detect EOF condition in chunked encoding
			Modio::ErrorCode HandleChunkedEncoding(Modio::Detail::DynamicBuffer InChunkedData,
												   Modio::Detail::DynamicBuffer ParsedData)
			{
				// Do we have any unfiltered data left?
				while (InChunkedData.size())
				{
					// If we were at a chunk boundary earlier
					if (Request->CurrentChunkSizeRemaining == 0)
					{
						// Search the first 1kb approx of our response
						Modio::Detail::Buffer SearchBuffer(std::min<std::size_t>(1000, InChunkedData.size()));
						Modio::Detail::BufferCopy(SearchBuffer, InChunkedData);
						// Pattern matches any hex number followed by an optional semicolon and other characters until a
						// CRLF
						std::regex ChunkSizePattern("^(\r\n)?([a-fA-f0-9]+)(;.*?)?\r\n");
						std::cmatch ChunkSizeMatches;
						// if the buffer begins with a chunk header
						if (std::regex_search( (const char*) SearchBuffer.begin(), (const char*)SearchBuffer.end(),
											  ChunkSizeMatches, ChunkSizePattern))
						{
							// extract the chunk size from the header
							std::string MatchString = ChunkSizeMatches[2].str();
							Request->CurrentChunkSizeRemaining = strtoull(MatchString.c_str(), nullptr, 16);
							// todo: @modio-nx handle zero length chunk, that should indicate EOF
							if (Request->CurrentChunkSizeRemaining == 0)
							{
								return Modio::make_error_code(Modio::GenericError::EndOfFile);
							}
							// Advance/consume/skip past the chunk header so we're only copying actual response data
							std::size_t ChunkHeaderLength = ChunkSizeMatches[0].second - ChunkSizeMatches[0].first;
							InChunkedData.consume(ChunkHeaderLength);

						}
						else
						{
							// we didn't have enough data remaining in the buffer for a full chunk header, so don't
							// process any more data
							return {};
						}
					}
					// Either we have just calculated CurrentChunkSizeRemaining, or we already had some data remaining
					// in the current chunk. Copy as much as we can, either up to the chunk boundary or the maximum
					// available data, whichever is lower
					std::size_t NumBytesToCopy = std::min(InChunkedData.size(), Request->CurrentChunkSizeRemaining);
					Modio::Detail::Buffer AvailableData(NumBytesToCopy);
					Modio::Detail::BufferCopy(AvailableData, InChunkedData);
					ParsedData.AppendBuffer(std::move(AvailableData));
					InChunkedData.consume(NumBytesToCopy);
						Request->CurrentChunkSizeRemaining -= NumBytesToCopy;
					if (Request->CurrentChunkSizeRemaining == 0)
					{
						InChunkedData.consume(2);
					}
				}
				return {};
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio