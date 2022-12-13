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
#include "modio/detail/ops/upload/UploadFilePartOp.h"
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
		class UploadMultipartFileOp
		{
			// Keeps track of the upload number
			int FilePart = 0;
			// A calculated number of parts related to the file size
			int NumParts = 0;
			Modio::Detail::HttpRequestParams OpenSessionRequest;
			Modio::StableStorage<Modio::Detail::HttpRequest> CloseSessionRequest;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::Detail::CachedResponse AllowCachedResponse;
			std::shared_ptr<Modio::Detail::UploadSession> Session;
			Modio::ModID ModID;
			Modio::filesystem::path ArchivePath;
			Modio::Detail::OperationQueue::Ticket RequestTicket;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
			asio::coroutine Coroutine;

		public:
			UploadMultipartFileOp(std::shared_ptr<Modio::Detail::UploadSession> ResponseSession,
								  Modio::ModID CurrentModID, Modio::filesystem::path ArchivePath,
								  Modio::Detail::OperationQueue::Ticket RequestTicket,
								  std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: ModID(CurrentModID),
				  ArchivePath(ArchivePath),
				  RequestTicket(std::move(RequestTicket)),
				  ProgressInfo(ProgressInfo)
			{
				Session = ResponseSession;
				this->AllowCachedResponse = Modio::Detail::CachedResponse::Disallow;
				std::string FileName = ArchivePath.filename().string();
				std::string FileNonce = std::to_string(Modio::Detail::hash_64_fnv1a_const(FileName.c_str()));

				// Request to open an Upload Session
				OpenSessionRequest = Modio::Detail::CreateMultipartUploadSessionRequest
										 .SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
										 .SetModID(CurrentModID)
										 .AppendPayloadValue("filename", FileName)
										 .AppendPayloadValue("nonce", FileNonce);
				;

				// Request to Close an Upload Session
				Modio::Detail::HttpRequestParams ParamsRequest =
					Modio::Detail::CompleteMultipartUploadSessionRequest
						.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
						.SetModID(CurrentModID);
				CloseSessionRequest = std::make_shared<Modio::Detail::HttpRequest>(ParamsRequest);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (ProgressInfo.lock() == nullptr)
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
					// Queue this operation for the next available slot
					yield RequestTicket.WaitForTurnAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Confirm the file exists in the system
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
							ArchivePath) == false)
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::FileNotFound));
						return;
					}

					// #1: Request an "Upload Session"
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer, OpenSessionRequest, Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
													"Error retrieving an UploadID");
						Self.complete(ec);
						return;
					}

					{
						// After the first request, we expect a "Upload Session", which is used
						// to append all other "chunks" of the large file.
						Modio::Optional<Modio::Detail::UploadSession> OptSession =
							TryMarshalResponse<Modio::Detail::UploadSession>(ResponseBuffer);
						if (OptSession.has_value() == false)
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
						else
						{
							// D_D LoL with the OptSession
							Session->UploadID = OptSession.value().UploadID;
							Session->UploadStatus = OptSession.value().UploadStatus;
						}

						// If we are aware of a completed/cancelled upload status, then
						// there is no need to progress in this op, but report a different
						// error depending on the status type
						if (Session->UploadStatus == UploadSession::Status::Cancelled)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Multipart upload session {} was cancelled",
														Session->UploadID.value());
							Self.complete(Modio::make_error_code(Modio::ModManagementError::UploadCancelled));
							return;
						}

						if (Session->UploadStatus == UploadSession::Status::Completed)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
														"Multipart upload session {} has been completed already",
														Session->UploadID.value());
							Self.complete({});
							return;
						}
					}

					// With a successful UploadID, then calculate the number of parts to upload based on
					// the current file size
					{
						std::uint64_t FileSize = Modio::Detail::File(ArchivePath, Modio::Detail::FileMode::ReadOnly, false).GetFileSize();
						// Make sure that NumParts nears to the next integer, for that a + 0.5 happens
						NumParts = static_cast<int>(
							std::ceil(FileSize / Constants::Configuration::MultipartMaxFilePartSize + 0.5f));
                        
                        // Report to the ProgressInfo that the TotalDownloadSize would be the FileSize
                        std::shared_ptr<Modio::ModProgressInfo> Progress = ProgressInfo.lock();
                        if (Progress)
                        {
                            Progress->TotalDownloadSize = Modio::FileSize(FileSize);
                        }
					}

					// #2: Upload every part of the file
					while (FilePart < NumParts)
					{
						yield Modio::Detail::UploadFilePartAsync(
							ResponseBuffer,
							Modio::Detail::AddMultipartUploadPartRequest
								.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
								.SetModID(ModID)
								// A "AddMultipartUploadPartRequest" needs the upload_id as part of the
								// URL parameters
								.SetFilterString("upload_id=" + Session->UploadID.value()),
							ArchivePath, FilePart, Session, ProgressInfo, std::move(Self));

						if (ec)
						{
							// Make sure any connection error would clear the upload_id to any caller of this operation
							Session->UploadID = {};
							Self.complete(ec);
							return;
						}

						// Per
						FilePart += 1;
					}

					// #3: Finalize the "Upload Session"
					{
						CloseSessionRequest->Parameters().SetFilterString("upload_id=" + Session->UploadID.value());
						yield CloseSessionRequest->SendAsync(std::move(Self));

						if (ec)
						{
							// Make sure any connection error would clear the upload_id to any caller of this operation
							Session->UploadID = {};
							Self.complete(ec);
							return;
						}

						yield CloseSessionRequest->ReadResponseHeadersAsync(std::move(Self));

						if (ec)
						{
							Self.complete(ec);
							return;
						}

						if (CloseSessionRequest->GetResponseCode() == 415)
						{
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Trace, Modio::LogCategory::Http,
								"Complete Request requires a Content-Type x-www-form-urlencoded");
							Self.complete(Modio::make_error_code(Modio::HttpError::RequestError));
							return;
						}

						yield CloseSessionRequest->ReadSomeFromResponseBodyAsync(ResponseBuffer, std::move(Self));

						if (ec && ec != make_error_code(Modio::GenericError::EndOfFile))
						{
							Self.complete(ec);
							return;
						}

						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"UploadMultipartFileOp CloseSessionRequest with upload_id: {}, "
													"response code {} of size {}:",
													Session->UploadID.value(), CloseSessionRequest->GetResponseCode(),
													ResponseBuffer.size());

#if MODIO_TRACE_DUMP_RESPONSE
						for (const auto& Buffer : ResponseBuffer)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http, "{}",
														std::string(Buffer.begin(), Buffer.end()));
						}
#endif // MODIO_TRACE_DUMP_RESPONSE
					}

					// #4: Send a normal UploadFileOp with the "upload_id" obtained by this operation.
					Self.complete({});
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto UploadMultipartFileAsync(std::shared_ptr<Modio::Detail::UploadSession> Response, Modio::ModID CurrentModID,
									  Modio::filesystem::path ArchivePath,
									  std::weak_ptr<Modio::ModProgressInfo> ProgressInfo, CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				UploadMultipartFileOp(
					std::move(Response), CurrentModID, ArchivePath,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
					ProgressInfo),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
