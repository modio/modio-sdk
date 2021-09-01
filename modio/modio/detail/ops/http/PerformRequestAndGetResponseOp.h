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
#include "modio/core/ModioLogService.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/http/ResponseError.h"
#include "modio/http/ModioHttpRequest.h"
#include "modio/http/ModioHttpService.h"
#include <memory>

#ifndef MODIO_TRACE_DUMP_RESPONSE
	#define MODIO_TRACE_DUMP_RESPONSE 0
#endif

namespace Modio
{
	namespace Detail
	{
		enum class CachedResponse : std::uint8_t
		{
			Allow,
			Disallow
		};
	} // namespace Detail
} // namespace Modio

#include <asio/yield.hpp>
class PerformRequestAndGetResponseOp : public Modio::Detail::BaseOperation<PerformRequestAndGetResponseOp>
{
	Modio::StableStorage<Modio::Detail::HttpRequest> Request;
	asio::coroutine Coroutine;
	Modio::Detail::DynamicBuffer ResultBuffer;
	Modio::Detail::CachedResponse AllowCachedResponse;

	struct
	{
		Modio::Detail::DynamicBuffer ResponseBodyBuffer;
	} State;

	struct PerformRequestImpl
	{
		Modio::Detail::OperationQueue::Ticket RequestTicket;

	public:
		PerformRequestImpl(Modio::Detail::OperationQueue::Ticket RequestTicket)
			: RequestTicket(std::move(RequestTicket)) {};
		PerformRequestImpl(const PerformRequestImpl& Other) = delete;
	};

	std::unique_ptr<PerformRequestImpl> Impl;

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
	};

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		using namespace Modio::Detail;
		// Temporary storage for the request so we don't invoke undefined behaviour via Move
		Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;

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

			yield Request->ReadResponseHeadersAsync(std::move(Self));

			if (ec)
			{
				Self.complete(ec);
				return;
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

				//Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single call
				// so make sure we steal all of them
				while ((CurrentBuffer = State.ResponseBodyBuffer.TakeInternalBuffer()))
				{
					if (CurrentBuffer.has_value())
					{
						ResultBuffer.AppendBuffer(std::move(CurrentBuffer.value()));
					}
				}
			}
			// After this State.ResponseBodyBuffer is considered dead

			std::uint32_t ResponseCode = Request->GetResponseCode();

// Additional if guarding as this logging is extra slow, so don't want to incur any overhead if someone
// don't include this trace data
#if MODIO_TRACE_DUMP_RESPONSE
			if (Services::GetGlobalService<Modio::Detail::LogService>().GetLogLevel() <= Modio::LogLevel::Trace)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
											"Received response code {} of size {}:", ResponseCode, ResultBuffer.size());
				for (const auto& Buffer : ResultBuffer)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "{}", Buffer.Data());
				}
			}
#endif
			Modio::Detail::SDKSessionData::ClearLastValidationError();
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
					Modio::ErrorCode ErrRef = Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef));
					if (Error->ExtendedErrorInformation.has_value())
					{
						Modio::Detail::SDKSessionData::SetLastValidationError(Error->ExtendedErrorInformation.value());
					}
					if (ErrRef != Modio::ErrorConditionTypes::ApiErrorRefSuccess)
					{
						// No need to log rate-limited response out
						if (ErrRef == Modio::ApiError::Ratelimited)
						{
							Modio::Detail::SDKSessionData::MarkAsRateLimited();
						}
						else
						{
							Modio::Detail::Buffer ResponseBuffer(ResultBuffer.size());
							Modio::Detail::BufferCopy(ResponseBuffer, ResultBuffer);

							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Non 200-204 response received: {}", ResponseBuffer.Data());
						}
					}
					// Return the error-ref regardless, defer upwards to Subscribe/Unsubscribe etc to handle as success
					Self.complete(Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef)));
					return;
				}
				else
				{
					//we have a raw HTTP response error but no error ref (ie probably we have a cloudflare error)
					//Self.complete(Modio::make_error_code(static_cast<Modio::RawHttpError>(ResponseCode);
					//return;
					Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
					return;
				}
			}

			if (ec != make_error_code(Modio::GenericError::EndOfFile))
			{
				Self.complete(ec);
				return;
			}
			else
			{
				// @note: We will never cache a response that's not 200 as they are returned earlier than this
				Services::GetGlobalService<CacheService>().AddToCache(Request->Parameters().GetFormattedResourcePath(),
																	  ResultBuffer);

				Self.complete(Modio::ErrorCode {});
				return;
			}
		}
	}
};
#include <asio/unyield.hpp>
