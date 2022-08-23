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
#include "macos/HttpSharedState.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include <memory>
#include <regex>
#include <vector>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>

		// kCFStreamPropertyHTTPResponseHeader is deprecated and causes warnings. Suppressing those warnings here.
		MODIO_DIAGNOSTIC_PUSH
		MODIO_ALLOW_DEPRECATED_SYMBOLS

		class ReadHttpResponseHeadersOp
		{
			asio::coroutine CoroutineState;
			std::shared_ptr<HttpRequestImplementation> Request;
			std::weak_ptr<HttpSharedState> SharedState;
			std::unique_ptr<asio::steady_timer> PollTimer;

			/**
			 * https://stackoverflow.com/questions/28860033/convert-from-cfurlref-or-cfstringref-to-stdstring
			 * Converts a CFString to a UTF-8 std::string if possible.
			 *
			 * @param input A reference to the CFString to convert.
			 * @return Returns a std::string containing the contents of CFString converted to UTF-8. Returns
			 *  an empty string if the input reference is null or conversion is not possible.
			 */
			static std::string CFStringToStdString(CFStringRef StrRef)
			{
				if (auto fastCString = CFStringGetCStringPtr(StrRef, kCFStringEncodingUTF8))
				{
					return std::string(fastCString);
				}

				// For some weird reason, some CFStrings can not transform directly to std::string, that's
				// why this extra code executes considering a UTF16Lenght instead.
				auto UTF16Length = CFStringGetLength(StrRef);
				auto MaxUTF8Len = CFStringGetMaximumSizeForEncoding(UTF16Length, kCFStringEncodingUTF8);
				// Use Modio Buffer to certify the length and memory location
				Modio::Detail::Buffer ConvertedBuff(MaxUTF8Len);
				// Store the CFString into the Buffer
				CFStringGetCString(StrRef, (char*) ConvertedBuff.Data(), MaxUTF8Len, kCFStringEncodingUTF8);
				// Transform the buffer to a std::string
				std::string Converted(ConvertedBuff.Data(), ConvertedBuff.Data() + UTF16Length);
				return Converted;
			}

			// The context is the HttpRequestImplementation that has all the HTTP Request headers
			// This is a helper function that iterates over all Key/Value pairs inside the
			// CFDictionaryRef from the original request.
			static void CopyHeaders(const void* KeyPtr, const void* ValuePtr, void* Context)
			{
				// Cast from void* to CFStringRef as know from the CFDictionary
				CFStringRef Key = (CFStringRef) KeyPtr;
				CFStringRef Value = (CFStringRef) ValuePtr;

				HttpRequestImplementation* Request = (HttpRequestImplementation*) Context;
				// It needs to translate between CFString to std::string
				std::string KeyStr = CFStringToStdString(Key);
				std::string ValueStr = CFStringToStdString(Value);

				if (KeyStr == "" || ValueStr == "")
				{
					// Some CFString do not translate well to std::string, avoid those here
					return;
				}

				// Transform those std::string into a HeaderItem
				httpparser::Response::HeaderItem HeaderIt = {KeyStr, ValueStr};
				// Place them in the Request.
				Request->ParsedResponseHeaders.headers.push_back(HeaderIt);
			}

		public:
			ReadHttpResponseHeadersOp(std::shared_ptr<HttpRequestImplementation> Request,
									  std::weak_ptr<HttpSharedState> SharedState)
				: Request(Request),
				  SharedState(SharedState)
			{
				PollTimer =
					std::make_unique<asio::steady_timer>(Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			ReadHttpResponseHeadersOp(ReadHttpResponseHeadersOp&& Other)
				: CoroutineState(std::move(Other.CoroutineState)),
				  Request(std::move(Other.Request)),
				  SharedState(std::move(Other.SharedState)),
				  PollTimer(std::move(Other.PollTimer))
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
					// Loop while the stream receives bytes or its status is not an error
					while (Request->ReadStreamHasBytes() == false)
					{
						if (Request->ReadStreamStatus() == kCFStreamStatusError ||
							Request->ReadStreamStatus() == kCFStreamStatusNotOpen)
						{
							// A defensive conditional for a case that might occur when the CFStream
							// is deallocated between async calls.
							if (Request->ReadStream != NULL)
							{
								CFStreamError StreamError = CFReadStreamGetError(Request->ReadStream);
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"CFStream open failure, error code {0}", StreamError.error);
							}
							else
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"CFStream was deallocated during request");
							}

							Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
							return;
						}

						PollTimer->expires_after(Modio::Detail::Constants::Configuration::PollInterval);
						yield PollTimer->async_wait(std::move(Self));
					}

					// In case of POST, response headers in the HTTP request are embeded within the ReadStream
					if (Request->GetParameters().GetTypedVerb() == Verb::POST ||
						Request->GetParameters().GetTypedVerb() == Verb::PUT)
					{
						// Read a portion of the streaming bytes into the ResponseDataBuffer, at this stage we know that
						// it has bytes because previously while was true
						while (ParseHeadersInResponseBuffer() == false &&
							   Request->ReadStreamStatus() != kCFStreamStatusAtEnd)
						{
							// In some cases, it is possible that the ReadStream has not "ended" but the Stream
							// remains open. If the CFReadStreamRead is called without bytes, it would wait for
							// a looooong time
							if (Request->ReadStreamHasBytes() == true)
							{
								// Read bytes into the ResponseDataBuffer
								constexpr CFIndex MaxBuffer = 64 * 1024;
								Modio::Detail::Buffer LinearBuffer(MaxBuffer);
								CFIndex ReadBytes =
									CFReadStreamRead(Request->ReadStream, LinearBuffer.Data(), MaxBuffer);

								if (ReadBytes <= 0)
								{
									// The stream could not be read or an error has ocurred.
									break;
								}

								Request->ResponseDataBuffer.AppendBuffer(LinearBuffer.CopyRange(0, ReadBytes));
								BytesLastRead += ReadBytes;
							}

							// Yield execution to avoid taking too much processing while waiting for the ReadStream to
							// arrive
							PollTimer->expires_after(Modio::Detail::Constants::Configuration::PollInterval);
							yield PollTimer->async_wait(std::move(Self));
						}
					}
					// With other HTTP verbs (ex. GET, DELETE), HTTPMessage parses the headers
					else
					{
						// From the stream, parse the Headers and StatusCode
						CFHTTPMessageRef HTTPResponse = (CFHTTPMessageRef) CFReadStreamCopyProperty(
							Request->ReadStream, kCFStreamPropertyHTTPResponseHeader);
						if (HTTPResponse == NULL)
						{
							// This case could happen when the connection did not establish.
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"ReadHTTPResponseHeadersOp failure with the connection");
							Self.complete(Modio::make_error_code(Modio::HttpError::CannotOpenConnection));
							return;
						}

						CFDictionaryRef Headers = CFHTTPMessageCopyAllHeaderFields(HTTPResponse);
						CFIndex StatusCode = CFHTTPMessageGetResponseStatusCode(HTTPResponse);
						// Use an auxiliary function to parse the HTTP Headers into the Request
						CFDictionaryApplyFunction(Headers, CopyHeaders, (void*) Request.get());
						Request->ResponseCode = StatusCode;
						Request->ParsedResponseHeaders.statusCode = StatusCode;
					}

					// Check if any errors occured during read
					if (Request->ReadStreamStatus() == kCFStreamStatusError)
					{
						// Reading the stream produced an error, report that to the caller.
						CFStreamError StreamError = CFReadStreamGetError(Request->ReadStream);
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"ReadHTTPResponseHeadersOp failure with error code {}",
													StreamError.error);
						Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Response Headers received OK with response code: {}",
                                                Request->ResponseCode);
					// Already processed the response code and removed header data from the response buffer so just
					// return no error code
					Self.complete({});
					return;
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
												"Bytes remaining after consumption: {}, while expecting {} bytes total in response",
												Request->ResponseDataBuffer.size(), Request->GetContentLength().value_or(0));
					return true;
				}
				else
				{
					return false;
				}
			}
		};
		// Re-allow deprecation warnings
		MODIO_DIAGNOSTIC_POP

#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
