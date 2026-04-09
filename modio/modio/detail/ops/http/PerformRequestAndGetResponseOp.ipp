/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#endif

#include "modio/detail/http/ResponseError.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioResponseErrorSerialization.h"

namespace Modio
{
	namespace Detail
	{
		PerformRequestAndGetResponseOp::PerformRequestAndGetResponseOp(
			Modio::Detail::DynamicBuffer Response,
										   Modio::Detail::HttpRequestParams RequestParams,
										   Modio::Detail::OperationQueue::Ticket RequestTicket,
										   Modio::Detail::CachedResponse AllowCachedResponseValue)
		{
			Request = std::make_shared<Modio::Detail::HttpRequest>(RequestParams);
			Impl = std::make_unique<PerformRequestImpl>(std::move(RequestTicket));
			ResultBuffer = Response;
			// Don't allow cache hits in the case of a endpoint of type != GET
			this->AllowCachedResponse = Request->Parameters().GetTypedVerb() == Modio::Detail::Verb::GET
											? AllowCachedResponseValue
											: Modio::Detail::CachedResponse::Disallow;
		}

		bool PerformRequestAndGetResponseOp::CanUseCachedResponse(
			Modio::Detail::CachedResponse AllowCachedResponseValue)
		{
			if (AllowCachedResponseValue == Modio::Detail::CachedResponse::Allow)
			{
				MODIO_PROFILE_SCOPE(RequestCache);
				Modio::Optional<Modio::Detail::DynamicBuffer> CachedResponse =
					Services::GetGlobalService<CacheService>().FetchFromCache(
						Request->Parameters().GetFormattedResourcePath());
				if (CachedResponse)
				{
					ResultBuffer.CopyBufferConfiguration(CachedResponse.value());
					BufferCopy(ResultBuffer, CachedResponse.value());
					return true;
				}
			}
			return false;
		}

		void PerformRequestAndGetResponseOp::FormatPayloadHeader()
		{
			std::string PayloadContentFilename;
			if (Impl->PayloadElement->second.PType == PayloadContent::PayloadType::File)
			{
				PayloadContentFilename = fmt::format(
					"; filename=\"{}\"",
					Modio::ToModioString(Impl->PayloadElement->second.PathToFile->filename().u8string()));
			}
			std::string PayloadContentHeader =
				fmt::format("\r\n--{}\r\nContent-Disposition: form-data; name=\"{}\"{}\r\n\r\n",
							Modio::Detail::HttpRequestParams::GetBoundaryHash(), Impl->PayloadElement->first,
							PayloadContentFilename);
			Impl->HeaderBuf = std::make_unique<Modio::Detail::Buffer>(PayloadContentHeader.size());
			std::copy(PayloadContentHeader.begin(), PayloadContentHeader.end(), Impl->HeaderBuf->begin());
		}

		void PerformRequestAndGetResponseOp::InitPayloadFile()
		{
			Impl->CurrentPayloadFileBytesRead = Modio::FileSize(0);

			Impl->CurrentPayloadFile = std::make_unique<Modio::Detail::File>(
				Impl->PayloadElement->second.PathToFile.value(), Modio::Detail::FileMode::ReadOnly);
		}

		std::size_t PerformRequestAndGetResponseOp::CalculateNumBytesToRead() const
		{
			constexpr std::size_t ChunkOfBytes = 64 * 1024;
			std::size_t MaxBytesToRead = 0;
			// In case a file is less than ChunkOfBytes, it will read only the necessary number
			// of bytes.
			MaxBytesToRead =
				(ChunkOfBytes + Impl->CurrentPayloadFileBytesRead < Impl->CurrentPayloadFile->GetFileSize())
					? ChunkOfBytes
					: Impl->CurrentPayloadFile->GetFileSize() - Impl->CurrentPayloadFileBytesRead;

			return MaxBytesToRead;
		}

		void PerformRequestAndGetResponseOp::WriteFinalPayloadBoundary()
		{
			/// Write the final boundary onto the wire
			std::string FinalPayloadBoundary =
				fmt::format("\r\n--{}\r\n", Modio::Detail::HttpRequestParams::GetBoundaryHash());
			Impl->HeaderBuf = std::make_unique<Modio::Detail::Buffer>(FinalPayloadBoundary.size());
			std::copy(FinalPayloadBoundary.begin(), FinalPayloadBoundary.end(), Impl->HeaderBuf->begin());
		}

		void PerformRequestAndGetResponseOp::AppendRemainingResults()
		{
			// Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single
			// call so make sure we steal all of them
			Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;
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

		Modio::ErrorCode PerformRequestAndGetResponseOp::MarshallResponse()
		{
			// After this State.ResponseBodyBuffer is considered dead
			std::uint32_t ResponseCode = Request->GetResponseCode();

			// Additional if guarding as this logging is extra slow, so don't want to incur any overhead if
			// someone don't include this trace data
#if defined(MODIO_TRACE_DUMP_RESPONSE) && MODIO_TRACE_DUMP_RESPONSE
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
					return Modio::make_error_code(Modio::HttpError::ServersOverloaded);
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
						return Modio::make_error_code(Modio::HttpError::InvalidResponse);
					}

					Modio::ErrorCode ErrRef = Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef));
					if (Error->ExtendedErrorInformation.has_value())
					{
						Modio::Detail::SDKSessionData::SetLastValidationError(
							Error->ExtendedErrorInformation.value());
					}
					if (ErrRef == Modio::ApiError::ExpiredOrRevokedAccessToken)
					{
						return Modio::make_error_code(Modio::UserAuthError::StatusAuthTokenInvalid);
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
					return ErrRef;
				}
				else
				{
					// we have a raw HTTP response error but no error ref (ie probably we have a cloudflare
					// error)
					// Self.complete(Modio::make_error_code(static_cast<Modio::RawHttpError>(ResponseCode);
					// return;
					return Modio::make_error_code(Modio::HttpError::InvalidResponse);
				}
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
										"Response received for {}; status code was: {}",
										Request->Parameters().GetFormattedResourcePath(), ResponseCode);

			Modio::Detail::Buffer ResponseBuffer(ResultBuffer.size());
			Modio::Detail::BufferCopy(ResponseBuffer, ResultBuffer);

			Modio::Detail::Logger().Log(Modio::LogLevel::Detailed, Modio::LogCategory::Http, "Response body was {}",
										std::string(ResponseBuffer.begin(), ResponseBuffer.end()));

			return {};
		}

	} // namespace Detail
} // namespace Modio