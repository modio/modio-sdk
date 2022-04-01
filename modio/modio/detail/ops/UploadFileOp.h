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
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/http/ResponseError.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include "modio/http/ModioHttpService.h"
#include <memory>

#ifndef MODIO_TRACE_DUMP_RESPONSE
	#define MODIO_TRACE_DUMP_RESPONSE 0
#endif
#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class UploadFileOp
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
				std::unique_ptr<Modio::Detail::File> CurrentPayloadFile;
				Modio::Detail::DynamicBuffer PayloadFileBuffer;
				Modio::FileSize CurrentPayloadFileBytesRead;
				Modio::Optional<std::pair<std::string, Modio::Detail::PayloadContent>> PayloadElement;
				std::unique_ptr<Modio::Detail::Buffer> HeaderBuf;
				std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;

			public:
				PerformRequestImpl(Modio::Detail::OperationQueue::Ticket RequestTicket)
					: RequestTicket(std::move(RequestTicket)) {};
				PerformRequestImpl(const PerformRequestImpl& Other) = delete;
			};

			std::unique_ptr<PerformRequestImpl> Impl;

		public:
			UploadFileOp(Modio::Detail::DynamicBuffer Response, Modio::Detail::HttpRequestParams RequestParams,
						 Modio::Detail::OperationQueue::Ticket RequestTicket,
						 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
			{
				Request = std::make_shared<Modio::Detail::HttpRequest>(RequestParams);
				Impl = std::make_unique<PerformRequestImpl>(std::move(RequestTicket));
				Impl->ProgressInfo = ProgressInfo;
				ResultBuffer = Response;
				this->AllowCachedResponse = Modio::Detail::CachedResponse::Disallow;
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				using namespace Modio::Detail;
				// Temporary storage for the request so we don't invoke undefined behaviour via Move
				Modio::Optional<Modio::Detail::Buffer> CurrentBuffer;
				size_t BytesToRead = 0;

				if (Impl->ProgressInfo.lock() == nullptr)
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::UploadCancelled));
					return;
				}
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
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

					yield Request->SendAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					if (Request->Parameters().ContainsFormData())
					{
						while ((Impl->PayloadElement = Request->Parameters().TakeNextPayloadElement()))
						{
							// Write the header for the field in the form data
							{
								{
									std::string PayloadContentFilename = "";
									if (Impl->PayloadElement->second.bIsFile)
									{
										PayloadContentFilename =
											fmt::format("; filename=\"{}\"",
														Impl->PayloadElement->second.PathToFile->filename().u8string());
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
							if (Impl->PayloadElement->second.bIsFile)
							{
								Impl->CurrentPayloadFileBytesRead = Modio::FileSize(0);

								Impl->CurrentPayloadFile = std::make_unique<Modio::Detail::File>(
									Impl->PayloadElement->second.PathToFile.value());
								{
									// Currently no API endpoints ask for more than one file, so we only need to do this
									// the once
									std::shared_ptr<Modio::ModProgressInfo> Progress = Impl->ProgressInfo.lock();
									if (Progress)
									{
										Progress->TotalDownloadSize =
											Modio::FileSize(Impl->CurrentPayloadFile->GetFileSize());
									}
								}
								while (Impl->CurrentPayloadFileBytesRead < Impl->CurrentPayloadFile->GetFileSize())
								{
									{
										// Limitting the number of bytes to what is necessary for the file size
										// is required in macOS to correctly send data over the WriteStream
										constexpr size_t MaxBytesBuffer = 64 * 1024;
										size_t FileSize = Impl->CurrentPayloadFile->GetFileSize();
										size_t RemainingBytes = FileSize - Impl->CurrentPayloadFileBytesRead;
										BytesToRead = std::min(MaxBytesBuffer, RemainingBytes);
									}

									yield Impl->CurrentPayloadFile->ReadAsync(BytesToRead, Impl->PayloadFileBuffer,
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
									{
										std::shared_ptr<Modio::ModProgressInfo> Progress = Impl->ProgressInfo.lock();
										if (Progress)
										{
											Progress->CurrentlyDownloadedBytes = Modio::FileSize(Impl->CurrentPayloadFileBytesRead);
										}
									}
								}
							}
							else
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

						// Some implementations of ReadSomeFromResponseBodyAsync may store multiple buffers in a single
						// call so make sure we steal all of them
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
													"Received response code {} of size {}:", ResponseCode,
													ResultBuffer.size());
						for (const auto& Buffer : ResultBuffer)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "{}",
														Buffer.Data());
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
							Modio::ErrorCode ErrRef =
								Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef));
							if (Error->ExtendedErrorInformation.has_value())
							{
								Modio::Detail::SDKSessionData::SetLastValidationError(
									Error->ExtendedErrorInformation.value());
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
																"Non 200-204 response received: {}",
																ResponseBuffer.Data());
								}
							}
							// Return the error-ref regardless, defer upwards to Subscribe/Unsubscribe etc to handle as
							// success
							Self.complete(Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef)));
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

					if (ec != make_error_code(Modio::GenericError::EndOfFile))
					{
						Self.complete(ec);
						return;
					}
					else
					{
						Self.complete(Modio::ErrorCode {});
						return;
					}
				}
			}
		};
#include <asio/unyield.hpp>
		template<typename CompletionTokenType>
		auto UploadFileAsync(Modio::Detail::DynamicBuffer Response, Modio::Detail::HttpRequestParams RequestParameters,
							 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo, CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				UploadFileOp(
					Response, RequestParameters,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
					ProgressInfo),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
