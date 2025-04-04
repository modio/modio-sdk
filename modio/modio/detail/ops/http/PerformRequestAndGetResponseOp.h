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

#include "modio/cache/ModioCacheService.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioLogService.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/http/PerformRequestImpl.h"
#include "modio/detail/http/ResponseError.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include "modio/http/ModioHttpService.h"
#include <functional>
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class PerformRequestAndGetResponseOp : public Modio::Detail::BaseOperation<PerformRequestAndGetResponseOp>
		{
			Modio::StableStorage<Modio::Detail::HttpRequest> Request;
			asio::coroutine Coroutine;
			Modio::Detail::DynamicBuffer ResultBuffer;
			Modio::Detail::CachedResponse AllowCachedResponse;
			std::unique_ptr<PerformRequestImpl> Impl;

			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			} State;

		public:
			PerformRequestAndGetResponseOp(Modio::Detail::DynamicBuffer Response,
										   Modio::Detail::HttpRequestParams RequestParams,
										   Modio::Detail::OperationQueue::Ticket RequestTicket,
										   Modio::Detail::CachedResponse AllowCachedResponse)
			{
				Request = std::make_shared<Modio::Detail::HttpRequest>(RequestParams);
				Impl = std::make_unique<PerformRequestImpl>(std::move(RequestTicket));
				ResultBuffer = Response;
				// Don't allow cache hits in the case of a endpoint of type != GET
				this->AllowCachedResponse = Request->Parameters().GetTypedVerb() == Modio::Detail::Verb::GET
												? AllowCachedResponse
												: Modio::Detail::CachedResponse::Disallow;
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(PerformRequest);
				using namespace Modio::Detail;
				// Temporary storage for the request so we don't invoke undefined behaviour via Move
				Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;
				constexpr std::size_t ChunkOfBytes = 64 * 1024;
				std::size_t MaxBytesToRead = 0;

				if (Impl->RequestTicket.WasCancelled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(Coroutine)
				{
					yield Impl->RequestTicket.WaitForTurnAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Only allow the cache on get endpoints so we don't accidentally cache post requests
					// Hit the cache and return a cached response if it exists, skipping the actual request
					// TODO: @Modio-core perhaps we want to move this cache check higher in the operation hierarchy
					// So that we don't even begin the operation if the cached response exists?
					if (AllowCachedResponse == Modio::Detail::CachedResponse::Allow)
					{
						MODIO_PROFILE_SCOPE(RequestCache);
						Modio::Optional<Modio::Detail::DynamicBuffer> CachedResponse =
							Services::GetGlobalService<CacheService>().FetchFromCache(
								Request->Parameters().GetFormattedResourcePath());
						if (CachedResponse)
						{
							ResultBuffer.CopyBufferConfiguration(CachedResponse.value());
							BufferCopy(ResultBuffer, CachedResponse.value());

							Self.complete({});
							return;
						}
					}

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					yield Request->SendAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}
					else if (Request->Parameters().ContainsFormData())
					{
						while ((Impl->PayloadElement = Request->Parameters().TakeNextPayloadElement()))
						{
							// Write the header for the field in the form data
							{
								{
									std::string PayloadContentFilename;
									if (Impl->PayloadElement->second.PType == PayloadContent::PayloadType::File)
									{
										PayloadContentFilename =
											fmt::format("; filename=\"{}\"",
														Modio::ToModioString(Impl->PayloadElement->second.PathToFile->filename().u8string()));
									}
									std::string PayloadContentHeader =
										fmt::format("\r\n--{}\r\nContent-Disposition: form-data; name=\"{}\"{}\r\n\r\n",
													Modio::Detail::HttpRequestParams::GetBoundaryHash(),
													Impl->PayloadElement->first, PayloadContentFilename);
									Impl->HeaderBuf =
										std::make_unique<Modio::Detail::Buffer>(PayloadContentHeader.size());
									std::copy(PayloadContentHeader.begin(), PayloadContentHeader.end(),
											  Impl->HeaderBuf->begin());
								}

								yield Request->WriteSomeAsync(std::move(*Impl->HeaderBuf), std::move(Self));
								if (ec)
								{
									Self.complete(ec);
									return;
								}
							}

							// Write the form data itself to the connection, either looping through the file data (for a
							// file field) or just writing the in-memory data (for string fields)
							if (Impl->PayloadElement->second.PType == PayloadContent::PayloadType::File)
							{
								Impl->CurrentPayloadFileBytesRead = Modio::FileSize(0);

								Impl->CurrentPayloadFile = std::make_unique<Modio::Detail::File>(
									Impl->PayloadElement->second.PathToFile.value(), Modio::Detail::FileMode::ReadOnly);

								while (Impl->CurrentPayloadFileBytesRead < Impl->CurrentPayloadFile->GetFileSize())
								{
									// In case a file is less than ChunkOfBytes, it will read only the necessary number
									// of bytes.
									MaxBytesToRead = (ChunkOfBytes + Impl->CurrentPayloadFileBytesRead <
													  Impl->CurrentPayloadFile->GetFileSize())
														 ? ChunkOfBytes
														 : Impl->CurrentPayloadFile->GetFileSize() -
															   Impl->CurrentPayloadFileBytesRead;
									yield Impl->CurrentPayloadFile->ReadAsync(MaxBytesToRead, Impl->PayloadFileBuffer,
																			  std::move(Self));
									Impl->CurrentPayloadFileBytesRead +=
										Modio::FileSize(Impl->PayloadFileBuffer.size());
									if (ec)
									{
										Self.complete(ec);
										return;
									}
									while (Impl->PayloadFileBuffer.size())
									{
										yield Request->WriteSomeAsync(
											std::move(Impl->PayloadFileBuffer.TakeInternalBuffer().value()),
											std::move(Self));
										if (ec)
										{
											Self.complete(ec);
											return;
										}
									}
								}
							}
							else if (Impl->PayloadElement->second.RawBuffer.has_value() &&
									 (*(Impl->PayloadElement->second.RawBuffer)).GetSize())
							{
								yield Request->WriteSomeAsync(std::move(*Impl->PayloadElement->second.RawBuffer),
															  std::move(Self));
								if (ec)
								{
									Self.complete(ec);
									return;
								}
							}
						}

						/// Write the final boundary onto the wire
						{
							std::string FinalPayloadBoundary =
								fmt::format("\r\n--{}\r\n", Modio::Detail::HttpRequestParams::GetBoundaryHash());
							Impl->HeaderBuf = std::make_unique<Modio::Detail::Buffer>(FinalPayloadBoundary.size());
							std::copy(FinalPayloadBoundary.begin(), FinalPayloadBoundary.end(),
									  Impl->HeaderBuf->begin());
						}
						yield Request->WriteSomeAsync(std::move(*Impl->HeaderBuf), std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}
					}

					yield Request->ReadResponseHeadersAsync(std::move(Self));

					Modio::Detail::SDKSessionData::ClearLastValidationError();

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					if (Modio::Optional<std::uint32_t> RetryAfter = Request->GetRetryAfter())
					{
						Modio::Detail::SDKSessionData::MarkAsRateLimited(std::uint32_t(RetryAfter.value()));
					}

					while (!ec)
					{
						// Read in a chunk from the response
						yield Request->ReadSomeFromResponseBodyAsync(State.ResponseBodyBuffer, std::move(Self));
						if (ec && ec != make_error_code(Modio::GenericError::EndOfFile))
						{
							Self.complete(ec);
							return;
						}

						// Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single
						// call so make sure we steal all of them
						while ((CurrentBuffer = State.ResponseBodyBuffer.TakeInternalBuffer()))
						{
							if (CurrentBuffer.has_value())
							{
								ResultBuffer.AppendBuffer(std::move(CurrentBuffer.value()));
							}
							else
							{
								// In case the current buffer does not have a value, break the loop
								break;
							}
						}
					}
					// After this State.ResponseBodyBuffer is considered dead
					std::uint32_t ResponseCode = Request->GetResponseCode();

					// Additional if guarding as this logging is extra slow, so don't want to incur any overhead if
					// someone don't include this trace data
#if MODIO_TRACE_DUMP_RESPONSE
					if (Services::GetGlobalService<Modio::Detail::LogService>().GetLogLevel() <= Modio::LogLevel::Trace)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Received response code {} of size {}:", ResponseCode,
													ResultBuffer.size());
						for (const auto& Buffer : ResultBuffer)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "{}",
														std::string(Buffer.begin(), Buffer.end()));
						}
					}
#endif
					if (ResponseCode < 200 || ResponseCode > 204)
					{
						// Servers massively overloaded, so we return a http page instead
						if (ResponseCode == 502)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"mod.io servers overloaded, please try again later");

							Self.complete(Modio::make_error_code(Modio::HttpError::ServersOverloaded));
							return;
						}

						// Inspect the error ref, only treat it as an error if it isn't a no-op
						Modio::Optional<ResponseError> Error = TryMarshalResponse<ResponseError>(ResultBuffer);
						if (Error.has_value())
						{
							// We got a response, but we were not able to marshall it successfully.
							if (Error->Code == -1)
							{
								Modio::Detail::Logger().Log(
									Modio::LogLevel::Warning, Modio::LogCategory::Http,
									"Non 200-204 response received which was unable to be marshaled successfully.");
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
								return;
							}

							Modio::ErrorCode ErrRef =
								Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef));
							if (Error->ExtendedErrorInformation.has_value())
							{
								Modio::Detail::SDKSessionData::SetLastValidationError(
									Error->ExtendedErrorInformation.value());
							}
							if (ErrRef == Modio::ApiError::ExpiredOrRevokedAccessToken)
							{
								Self.complete(Modio::make_error_code(Modio::UserAuthError::StatusAuthTokenInvalid));
								return;
							}
							if (ErrRef != Modio::ErrorConditionTypes::ApiErrorRefSuccess &&
								// No need to log rate-limited response out
								ErrRef != Modio::ApiError::Ratelimited)
							{
								Modio::Detail::Buffer ResponseBuffer(ResultBuffer.size());
								Modio::Detail::BufferCopy(ResponseBuffer, ResultBuffer);

								// Hate doing this copy but this really should be only happening in exceptional
								// circumstances and we want to avoid dragging in fmt string_view
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
															"Non 200-204 response received: {}",
															std::string(ResponseBuffer.begin(), ResponseBuffer.end()));
							}
							// Return the error-ref regardless, defer upwards to Subscribe/Unsubscribe etc to handle as
							// success
							Self.complete(ErrRef);
							return;
						}
						else
						{
							// we have a raw HTTP response error but no error ref (ie probably we have a cloudflare
							// error)
							// Self.complete(Modio::make_error_code(static_cast<Modio::RawHttpError>(ResponseCode);
							// return;
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
												"Response received for {}; status code was: {}",
												Request->Parameters().GetFormattedResourcePath(), ResponseCode);

					Modio::Detail::Buffer ResponseBuffer(ResultBuffer.size());
					Modio::Detail::BufferCopy(ResponseBuffer, ResultBuffer);

					Modio::Detail::Logger().Log(Modio::LogLevel::Detailed, Modio::LogCategory::Http,
												"Response body was {}",
												std::string(ResponseBuffer.begin(), ResponseBuffer.end()));

					if (ec != make_error_code(Modio::GenericError::EndOfFile))
					{
						Self.complete(ec);
						return;
					}
					else
					{
						// @note: We will never cache a response that's not 200 as they are returned earlier than this
						Services::GetGlobalService<CacheService>().AddToCache(
							Request->Parameters().GetFormattedResourcePath(), ResultBuffer);

						Self.complete(Modio::ErrorCode {});
						return;
					}
				}
			}
		};

		/// @brief Begins an asynchronous operation that performs a HTTP request and buffers the entire response into
		/// memory
		/// @tparam CompletionTokenType Type of the completion handler for the operation
		/// @param Response Dynamic buffer to hold the response data
		/// @param RequestParameters Request Parameters
		/// @param AllowCachedResponse Should the cache be checked for this request or do we want to always hit the
		/// server
		/// @param Token Concrete completion handler instance
		template<typename CompletionTokenType>
		auto PerformRequestAndGetResponseAsync(Modio::Detail::DynamicBuffer Response,
											   Modio::Detail::HttpRequestParams RequestParameters,
											   Modio::Detail::CachedResponse AllowCachedResponse,
											   CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				PerformRequestAndGetResponseOp(
					Response, RequestParameters,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetAPIRequestTicket(),
					AllowCachedResponse),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
