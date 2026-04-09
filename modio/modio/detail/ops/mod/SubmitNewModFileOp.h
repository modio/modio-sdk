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

#include "modio/core/ModioCreateModFileParams.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ops/compression/CompressFolderOp.h"
#include "modio/detail/ops/upload/UploadFileOp.h"
#include "modio/detail/ops/upload/UploadMultipartFileOp.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitNewModFileOp
		{
		public:

			SubmitNewModFileOp(Modio::ModID ModID, Modio::CreateModFileParams Params);

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				reenter(CoroutineState)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
						"Beginning submission of mod {}", CurrentModID);
					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DirectoryExists(
						ModRootDirectory) ||
						ArchivePath.empty())
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent{
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
							Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound) });
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound));
						return;
					}
					else
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
							Modio::ModManagementEvent{ CurrentModID,
													   Modio::ModManagementEvent::EventType::BeginUpload,
													   {} });
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
						"Compressing directory {}", ModRootDirectory.string());
					yield Modio::Detail::CompressFolderAsync(ModRootDirectory, ArchivePath, FileHash, ProgressInfo,
						std::move(Self));

					if (ec)
					{
						// Marshal generic cancellation as mod management specific cancellation, because the archive
						// implementation is potentially used outside of uploads it can return
						// Modio::GenericError::OperationCanceled
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent{
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
							Modio::ErrorCodeMatches(ec, Modio::GenericError::OperationCanceled)
								? Modio::make_error_code(Modio::ModManagementError::UploadCancelled)
								: ec });
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);

						Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(ArchivePath);
						Self.complete(ec);
						return;
					}

					{
						// Determine the file size of the compressed modfile
						ZipFileSize = Modio::filesystem::file_size(ArchivePath, ec);

						if (ec)
						{
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
								ArchivePath);

							Self.complete(ec);
							return;
						}
					}

					SubmitParams = CreateRequestParams(
						Modio::Detail::AddModfileRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID()),
						CurrentModID, CurrentModParams);

					// The API requires that 500MiB+ files should upload using the Multipart
					if (ZipFileSize > Modio::Detail::Constants::Configuration::MultipartMaxFileSize)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
							"ModID {} Upload Multipart Archive file at {}", CurrentModID,
							ArchivePath.string());
						Session = std::make_shared<Modio::Detail::UploadSession>();
						// This operation would split request in 50MB chunks
						yield Modio::Detail::UploadMultipartFileAsync(Session, CurrentModID, ArchivePath,
							std::to_string(*FileHash), ProgressInfo,
							std::move(Self));

						FileHash.reset();
						if (ec)
						{
							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent{ CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
														   ec });

							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
								ArchivePath);
							Self.complete(ec);
							return;
						}

						if (Session->UploadID.has_value() == false)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
								"ModID {} Upload Multipart did not obtain an UploadID",
								CurrentModID, ArchivePath.string());
							ec = Modio::make_error_code(Modio::HttpError::RequestError);
							Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
								Modio::ModManagementEvent{ CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
														   ec });

							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
								ArchivePath);
							Self.complete(ec);
							return;
						}

						Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
							"Upload Archive file {} with UploadID {}", ArchivePath.string(),
							Session->UploadID.value());
						yield Modio::Detail::UploadFileAsync(
							ResponseBuffer, SubmitParams.AppendPayloadValue("upload_id", Session->UploadID.value()),
							ProgressInfo, std::move(Self));
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
							"Upload Archive file at {}", ArchivePath.string());
						yield Modio::Detail::UploadFileAsync(ResponseBuffer,
							SubmitParams.AppendPayloadFile("filedata", ArchivePath),
							ProgressInfo, std::move(Self));
					}

					// Delete zip file when the upload is done
					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
						ArchivePath))
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::ModManagement,
							"Failed to delete temp archive {}", ArchivePath.string());
					}

					if (ec)
					{
						// Don't need to marshal generic cancellation here because UploadFileAsync returns
						// upload-specific cancellation code already
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent{
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded, ec });
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(ec);
						return;
					}
					else
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
							Modio::ModManagementEvent{ CurrentModID,
													   Modio::ModManagementEvent::EventType::Uploaded,
													   {} });
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete({});
						return;
					}
				}
			}

			Modio::Detail::HttpRequestParams CreateRequestParams(HttpRequestParams AddRequest, Modio::ModID ModID,
																 Modio::CreateModFileParams Params);
		private:
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::HttpRequestParams SubmitParams {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};
			std::shared_ptr<Modio::Detail::UploadSession> Session {};
			Modio::filesystem::path ArchivePath {};
			Modio::filesystem::path ModRootDirectory {};
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo {};
			std::shared_ptr<uint64_t> FileHash {};
			Modio::ModID CurrentModID {};
			Modio::CreateModFileParams CurrentModParams {};
			std::uintmax_t ZipFileSize = 0;
		};
#include <asio/unyield.hpp>
		template<typename SubmitDoneCallback>
		auto SubmitNewModFileAsync(Modio::ModID ModID, Modio::CreateModFileParams Params, SubmitDoneCallback&& Callback)
		{
			return ModioAsio::async_compose<SubmitDoneCallback, void(Modio::ErrorCode)>(
				Modio::Detail::SubmitNewModFileOp(ModID, Params), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "SubmitNewModFileOp.ipp"
#endif