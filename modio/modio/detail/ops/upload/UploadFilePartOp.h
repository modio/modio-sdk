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
#include "modio/detail/entities/ModioUploadSession.h"
#include "modio/detail/http/PerformRequestImpl.h"
#include "modio/detail/http/ResponseError.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include "modio/http/ModioHttpService.h"
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class UploadFilePartOp
		{
			Modio::StableStorage<Modio::Detail::HttpRequest> Request;
			asio::coroutine Coroutine;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::Detail::CachedResponse AllowCachedResponse;
			std::shared_ptr<Modio::Detail::UploadSession> Session;
			std::unique_ptr<Modio::Detail::File> ArchiveFile;
			std::unique_ptr<PerformRequestImpl> Impl;
			// Location in the file respective to its origin
			std::uintmax_t FileOffset = 0;
			// How large the file is
			std::uintmax_t FileSize = 0;
			// The number of bytes that have already been sent over the wire
			std::uintmax_t BytesProcessed = 0;
			// The number of bytes that will be sent over the wire
			std::uintmax_t BytesToSend = 0;

		public:
			UploadFilePartOp(Modio::Detail::DynamicBuffer Response, Modio::Detail::HttpRequestParams BasicParams,
							 Modio::filesystem::path FilePath, int FilePart,
							 std::shared_ptr<Modio::Detail::UploadSession> UploadSession,
							 Modio::Detail::OperationQueue::Ticket RequestTicket,
							 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: Session(UploadSession)
			{
				if (UploadSession->UploadID.has_value() == false)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"Multipart upload requires a valud upload_id");
					return;
				}

				Impl = std::make_unique<PerformRequestImpl>(std::move(RequestTicket));
				Impl->ProgressInfo = ProgressInfo;
				ResponseBuffer = Response;
				this->AllowCachedResponse = Modio::Detail::CachedResponse::Disallow;
				ArchiveFile = std::make_unique<Modio::Detail::File>(FilePath, Modio::Detail::FileMode::ReadOnly, false);
				std::uintmax_t MaxFilePart = Modio::Detail::Constants::Configuration::MultipartMaxFilePartSize;
				FileOffset = FilePart * MaxFilePart;
				FileSize = ArchiveFile->GetFileSize();
				// The number of bytes that have already been sent over the wire, start at zero always
				BytesProcessed = 0;
				// We need to know how many bytes this operation will process. If it exceeds the number
				// of bytes remaining in the file, it would only send that portion, otherwise MaxFilePart
				BytesToSend = FileOffset + MaxFilePart > FileSize ? FileSize - FileOffset : MaxFilePart;

				// This needs to append the amount of bytes to send over the wire
				Modio::Detail::HttpRequestParams RequestParams =
					BasicParams
						.AppendPayloadFile("uploadMultipart-binarydata", ArchiveFile->GetPath(),
										   Modio::FileOffset(FileOffset), Modio::FileSize(BytesToSend))
						// For all request, append the UploadID
						.AppendPayloadValue("upload_id", UploadSession->UploadID.value());

				// It has "BytesToRead - 1" because the server counts the "0" byte
				RequestParams.SetContentRange(Modio::FileOffset(FileOffset),
											  Modio::FileOffset(FileOffset + BytesToSend - 1),
											  Modio::FileOffset(FileSize));

				Request = std::make_shared<Modio::Detail::HttpRequest>(RequestParams);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {},
							Modio::Optional<Modio::Detail::Buffer> FileChunk = {})
			{
				// Only read as many as "ChunkOfBytes" from the file, then send to the request.
				constexpr std::size_t ChunkOfBytes = 64 * 1024;

				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();

				if (Impl->ProgressInfo.lock() == nullptr)
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::UploadCancelled));
					return;
				}

				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::ModManagementDisabled));
					return;
				}

				reenter(Coroutine)
				{
					// Confirm (again) the file exists in the system
					if (FileService.FileExists(ArchiveFile->GetPath()) == false)
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::FileNotFound));
						return;
					}

					if (FileOffset > FileSize)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Multipart part upload with offset: {} exceeds file size: {}",
													FileOffset, FileSize);
						Self.complete(make_error_code(Modio::GenericError::EndOfFile));
						return;
					}

					yield Request->SendAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					while (BytesProcessed < BytesToSend)
					{
						// Read up-to "MultipartMaxFilePartSize" bytes from the File with the offset moving the
						// section of file to retrieve
						yield ArchiveFile->ReadSomeAtAsync(FileOffset, ChunkOfBytes, std::move(Self));

						if (ec && ec != Modio::GenericError::EndOfFile)
						{
							Self.complete(ec);
							return;
						}

						if (FileChunk.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
							return;
						}

						yield Request->WriteSomeAsync(std::move(FileChunk.value()), std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}

						BytesProcessed += ChunkOfBytes;
						FileOffset += ChunkOfBytes;

						std::shared_ptr<Modio::ModProgressInfo> Progress = Impl->ProgressInfo.lock();
						if (Progress)
						{
							Progress->CurrentlyDownloadedBytes = Modio::FileSize(BytesProcessed);
							SetCurrentProgress(*Progress.get(), Modio::FileSize(BytesProcessed));
						}
					}

					yield Request->ReadResponseHeadersAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// According to the swagger definition:
					// Code 400: An uploaded part with the specified start - finish byte range has already been
					// uploaded to that session Code 403: The authenticated user does not have permission to upload
					// modfiles for the specified mod With that in context, a 400 might be able to continue with the
					// next part, whereas a 403 should stop trying to upload any more parts of the file.

					if (Request->GetResponseCode() == 403)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Multipart upload failed related to permission to upload");
						Self.complete(Modio::make_error_code(Modio::HttpError::InsufficientPermissions));
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"Multipart upload_id {} bytes sent: {}", Session->UploadID.value(),
												FileOffset);

					// We only need to read the response when debugging with the server
					yield Request->ReadSomeFromResponseBodyAsync(ResponseBuffer, std::move(Self));

					if (ec && ec != make_error_code(Modio::GenericError::EndOfFile))
					{
						Self.complete(ec);
						return;
					}

					Modio::Detail::Logger().Log(
						Modio::LogLevel::Trace, Modio::LogCategory::Http,
						"UploadFilePartOp sent a part with upload_id: {}, response code {} of size {}:",
						Session->UploadID.value(), Request->GetResponseCode(), ResponseBuffer.size());

#if MODIO_TRACE_DUMP_RESPONSE
					for (const auto& Buffer : ResponseBuffer)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "{}",
													std::string(Buffer.begin(), Buffer.end()));
					}
#endif // MODIO_TRACE_DUMP_RESPONSE

					// All parts of the file were uploaded as expected
					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto UploadFilePartAsync(Modio::Detail::DynamicBuffer Response,
								 Modio::Detail::HttpRequestParams RequestParameters, Modio::filesystem::path FilePath,
								 int FilePart, std::shared_ptr<Modio::Detail::UploadSession> Session,
								 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo, CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				UploadFilePartOp(
					Response, RequestParameters, FilePath, FilePart, Session,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
					ProgressInfo),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
