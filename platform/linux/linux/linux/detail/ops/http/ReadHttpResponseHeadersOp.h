/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *   
 */

#pragma once

#include "http/HttpRequestImplementation.h"
#include "httpparser/httpresponseparser.h"
#include "httpparser/response.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "linux/HttpSharedState.h"
#include "linux/detail/ops/http/SSLConnectionReadSomeOp.h"
#include <memory>
#include <vector>
#include <regex>
namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class ReadHttpResponseHeadersOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			std::unique_ptr<asio::steady_timer> PollTimer;

		public:
			ReadHttpResponseHeadersOp(std::shared_ptr<HttpRequestImplementation> Request,
								  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			ReadHttpResponseHeadersOp(ReadHttpResponseHeadersOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState)),
				  PollTimer(std::move(Other.PollTimer))
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
					while (!ec && !ParseHeadersInResponseBuffer() && BytesLastRead != 0)
					{
						yield SSLConnectionReadSomeAsync(Request, Request->ResponseDataBuffer,
														 SharedState, std::move(Self));
					}
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Response Headers received OK");
						// Already processed the response code and removed header data from the response buffer so just
						// return no error code
						Self.complete({});
						return;
					}
				}
			}
			bool ParseHeadersInResponseBuffer()
			{
				if (Request->ResponseDataBuffer.size() == 0)
				{
					return false;
				}
				Modio::Detail::Buffer LinearBuffer(Request->ResponseDataBuffer.size());
				Modio::Detail::BufferCopy(LinearBuffer, Request->ResponseDataBuffer);
				httpparser::HttpResponseParser Parser;
				httpparser::Response ParsedResponse {};
				httpparser::HttpResponseParser::ParseResult Result =
					Parser.parse(ParsedResponse, (const char*) LinearBuffer.begin(), (const char*) LinearBuffer.end());
				if (Result == httpparser::HttpResponseParser::ParsingCompleted ||( Result == httpparser::HttpResponseParser::ParsingIncompleted && ParsedResponse.content.size() > 0))
				{
					
					Request->ParsedResponseHeaders = ParsedResponse;
					Request->ResponseCode = ParsedResponse.statusCode;
					std::cmatch Matches;
					std::regex doubleNewLine("\r\n\r\n");
					std::regex_search((const char*) LinearBuffer.begin(), (const char*)LinearBuffer.end(),Matches, doubleNewLine);

					// Consume amount of data the headers used up so ResponseDataBuffer is only the body
					Request->ResponseDataBuffer.consume(LinearBuffer.GetSize() - Matches.suffix().length());
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"expecting {0} bytes in response, total", Request->GetContentLength().value_or(0));
					return true;
				}
				else
				{
					return false;
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio