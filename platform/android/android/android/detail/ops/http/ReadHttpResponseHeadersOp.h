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
#include "httpparser/httpresponseparser.h"
#include "httpparser/response.h"
#include "android/HttpSharedState.h"
#include "android/detail/ops/http/SSLConnectionReadSomeOp.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include <memory>
#include <regex>
#include <vector>
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

		public:
			ReadHttpResponseHeadersOp(std::shared_ptr<HttpRequestImplementation> Request,
									  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{}

			ReadHttpResponseHeadersOp(ReadHttpResponseHeadersOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState))
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t BytesLastRead = -1)
			{
				MODIO_PROFILE_SCOPE(ReadHttpResponseHeaders);

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
						yield SSLConnectionReadSomeAsync(Request, Request->ResponseDataBuffer, SharedState,
														 std::move(Self));
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
				if (Result == httpparser::HttpResponseParser::ParsingCompleted ||
					(Result == httpparser::HttpResponseParser::ParsingIncompleted && ParsedResponse.content.size() > 0))
				{
					Request->ParsedResponseHeaders = ParsedResponse;
					Request->ResponseCode = ParsedResponse.statusCode;
					std::cmatch Matches;
					std::regex doubleNewLine("\r\n\r\n");
					std::regex_search((const char*) LinearBuffer.begin(), (const char*) LinearBuffer.end(), Matches,
									  doubleNewLine);

					// Consume amount of data the headers used up so ResponseDataBuffer is only the body
					Request->ResponseDataBuffer.consume(LinearBuffer.GetSize() - Matches.suffix().length());
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"expecting {0} bytes in response, total",
												Request->GetContentLength().value_or(0));
					return true;
				}
				else
				{
					// Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
					// 							"Incomplete parse, data: {}", LinearBuffer.Data());
					return false;
				}
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio