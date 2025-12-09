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
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioOperationQueue.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/serialization/ModioUploadSessionSerialization.h"
#include "modio/detail/http/PerformRequestImpl.h"
#include "modio/detail/http/ResponseError.h"
#include "modio/detail/ops/upload/Multipart/MultipartGetSessionOp.h"
#include "modio/detail/ops/upload/Multipart/MultipartGetUploadedOp.h"
#include "modio/detail/ops/upload/UploadFilePartOp.h"
#include "modio/file/ModioFile.h"
#include "modio/http/ModioHttpRequest.h"
#include "modio/http/ModioHttpService.h"
#include <memory>

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS

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
			Modio::Detail::HttpRequestParams OpenSessionRequest {};
			Modio::StableStorage<Modio::Detail::HttpRequest> CloseSessionRequest {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};
			Modio::Detail::CachedResponse AllowCachedResponse {};
			std::shared_ptr<Modio::Detail::UploadSession> Session {};
			std::shared_ptr<Modio::Detail::UploadSessionPartList> SessionParts {};
			Modio::ModID ModID {};
			Modio::filesystem::path ArchivePath {};
			Modio::Detail::OperationQueue::Ticket RequestTicket;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo {};
			ModioAsio::coroutine Coroutine {};

		public:
			UploadMultipartFileOp(std::shared_ptr<Modio::Detail::UploadSession> ResponseSession,
								  Modio::ModID CurrentModID, Modio::filesystem::path ArchivePath, std::string FileHash,
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

				// Request to open an Upload Session
				OpenSessionRequest = Modio::Detail::CreateMultipartUploadSessionRequest
										 .SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
										 .SetModID(CurrentModID)
										 .AppendPayloadValue("filename", FileName)
										 .AppendPayloadValue("nonce", FileHash);

				// Request to Close an Upload Session
				Modio::Detail::HttpRequestParams ParamsRequest =
					Modio::Detail::CompleteMultipartUploadSessionRequest
						.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
						.SetModID(CurrentModID);
				CloseSessionRequest = std::make_shared<Modio::Detail::HttpRequest>(ParamsRequest);
				SessionParts = std::make_shared<Modio::Detail::UploadSessionPartList>();
			}

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
						// If the error reference value is "29002", let the user know that a file with different
						// contents must be uploaded.
						if (ec.value() == 29002)
						{
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Error, Modio::LogCategory::Http,
								"Try to upload an updated file with different contents in the zip file");
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Error retrieving an UploadID");
						}
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

					// Identify if the session has uploaded parts previously
					{
						yield Modio::Detail::MultipartGetUploadedAsync(SessionParts, ModID, *Session, std::move(Self));

						if (ec)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
														"Error executing MultipartGetUploadedAsync: {}", ec.message());
							Self.complete(ec);
							return;
						}
					}

					// With a successful UploadID, then calculate the number of parts to upload based on
					// the current file size
					{
						std::uint64_t FileSize =
							Modio::Detail::File(ArchivePath, Modio::Detail::FileMode::ReadOnly, false).GetFileSize();
						// Make sure that NumParts nears to the next integer
						NumParts = 1 + static_cast<int>(FileSize / Constants::Configuration::MultipartMaxFilePartSize);
                        
                        // Report to the ProgressInfo that the TotalDownloadSize would be the FileSize
                        std::shared_ptr<Modio::ModProgressInfo> Progress = ProgressInfo.lock();
                        if (Progress)
                        {
							SetState(*Progress.get(), Modio::ModProgressInfo::EModProgressState::Uploading);
							SetTotalProgress(*Progress.get(), Modio::ModProgressInfo::EModProgressState::Uploading,
											 Modio::FileSize(FileSize));
						}
					}

					// #2: Upload every part of the file
					while (FilePart < NumParts)
					{
						// We need to check for FilePart + 1 because the server counts from 1, whereas FilePart counts
						// from 0
						if (ContainsPart(*SessionParts, FilePart + 1) == true)
						{
							// Advance the progress by the amount of bytes the part already has in the server
							std::shared_ptr<Modio::ModProgressInfo> Progress = ProgressInfo.lock();
							if (Progress)
							{
								std::uintmax_t MaxFilePart =
									Modio::Detail::Constants::Configuration::MultipartMaxFilePartSize;
								Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
															"Part {} already uploaded, advancing the ModProgressInfo",
															FilePart);
								SetCurrentProgress(*Progress.get(), Modio::FileSize(MaxFilePart * (FilePart + 1)));
							}

							// Increase the part to process next
							FilePart += 1;
							continue;
						}

						// This section means that this part needs to upload
						yield Modio::Detail::UploadFilePartAsync(
							ResponseBuffer,
							Modio::Detail::AddMultipartUploadPartRequest
								.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
								.SetModID(ModID)
								// A "AddMultipartUploadPartRequest" needs the upload_id as part of the
								// URL parameters
								.AddQueryParamRaw("upload_id", Session->UploadID.value()),
							ArchivePath, FilePart, Session, ProgressInfo, std::move(Self));

						if (ec)
						{
							// Make sure any connection error would clear the upload_id to any caller of this operation
							Session->UploadID = {};
							Self.complete(ec);
							return;
						}

						// Increase the part to process next
						FilePart += 1;
					}

					// #3: Finalize the "Upload Session"
					{
						CloseSessionRequest->Parameters().AddQueryParamRaw("upload_id", Session->UploadID.value());
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
    
						if (Modio::Optional<std::uint32_t> RetryAfter = CloseSessionRequest->GetRetryAfter())
						{
							Modio::Detail::SDKSessionData::MarkAsRateLimited(RetryAfter.value());
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

					}

					// #4: Send a normal UploadFileOp with the "upload_id" obtained by this operation.
					Self.complete({});
				}
			}

		private:
			// Iterate over "UploadSessionPartList" to check if any equals the "Part" variable
			static bool ContainsPart(Modio::Detail::UploadSessionPartList Parts, std::uint32_t Part)
			{
				if (Parts.Size() <= 0)
				{
					return false;
				}

				auto PartEqual = [&Part](Modio::Detail::UploadSessionPart UpPart) {
					return UpPart.PartNumber == Part;
				};

				auto PartFound = std::find_if(Parts.begin(), Parts.end(), PartEqual);

				// If the "Part" was found, it should not be equal to the end of the list
				return PartFound != Parts.end();
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto UploadMultipartFileAsync(std::shared_ptr<Modio::Detail::UploadSession> Response, Modio::ModID CurrentModID,
									  Modio::filesystem::path ArchivePath, std::string FileHash,
									  std::weak_ptr<Modio::ModProgressInfo> ProgressInfo, CompletionTokenType&& Token)
		{
			return ModioAsio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				UploadMultipartFileOp(
					std::move(Response), CurrentModID, ArchivePath, FileHash,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().GetFileDownloadTicket(),
					ProgressInfo),
				Token, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

MODIO_DIAGNOSTIC_POP
