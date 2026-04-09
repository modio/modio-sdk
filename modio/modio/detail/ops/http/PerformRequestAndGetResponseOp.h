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
#include "modio/detail/http/PerformRequestImpl.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/http/ModioHttpRequest.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class PerformRequestAndGetResponseOp : public Modio::Detail::BaseOperation<PerformRequestAndGetResponseOp>
		{
			Modio::StableStorage<Modio::Detail::HttpRequest> Request {};
			ModioAsio::coroutine Coroutine {};
			Modio::Detail::DynamicBuffer ResultBuffer {};
			Modio::Detail::CachedResponse AllowCachedResponse {};
			std::unique_ptr<PerformRequestImpl> Impl {};

			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			} State;

			bool CanUseCachedResponse(Modio::Detail::CachedResponse AllowCachedResponse);
			void FormatPayloadHeader();
			void InitPayloadFile();
			std::size_t CalculateNumBytesToRead() const;
			void WriteFinalPayloadBoundary();
			void AppendRemainingResults();
			Modio::ErrorCode MarshallResponse();

		public:
			PerformRequestAndGetResponseOp(Modio::Detail::DynamicBuffer Response,
										   Modio::Detail::HttpRequestParams RequestParams,
										   Modio::Detail::OperationQueue::Ticket RequestTicket,
										   Modio::Detail::CachedResponse AllowCachedResponse);

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(PerformRequest);
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
					if (CanUseCachedResponse(AllowCachedResponse))
					{
						Self.complete({});
						return;
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
								FormatPayloadHeader();

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
								InitPayloadFile();

								while (Impl->CurrentPayloadFileBytesRead < Impl->CurrentPayloadFile->GetFileSize())
								{
									MaxBytesToRead = CalculateNumBytesToRead();

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

						// Write the final boundary onto the wire
						WriteFinalPayloadBoundary();

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
						AppendRemainingResults();
					}

					/// \todo	is this right???? [RB]
					ec = MarshallResponse();

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
			return ModioAsio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				PerformRequestAndGetResponseOp(
					Response, RequestParameters,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetAPIRequestTicket(),
					AllowCachedResponse),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>

#ifndef MODIO_SEPARATE_COMPILATION
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.ipp"
#endif