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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ops/compression/CompressFolderOp.h"
#include "modio/detail/ops/upload/UploadFileOp.h"
#include "modio/detail/ops/upload/UploadMultipartFileOp.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitNewModFileOp
		{
		public:
			SubmitNewModFileOp(Modio::ModID ModID, Modio::CreateModFileParams Params)
				: CurrentModID(ModID),
				  CurrentModParams(Params)
			{
				// The assignment of SubmitParams is delayed until the size of modfile_{}.zip can be known
				// to either make a "simple" or "multipart" upload.

				ModRootDirectory = Params.RootDirectory;
				ArchivePath = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
								  .MakeTempFilePath(fmt::format("modfile_{}.zip", ModID))
								  .value_or("");
				ProgressInfo = Modio::Detail::SDKSessionData::StartModDownloadOrUpdate(CurrentModID);
				FileHash = std::make_shared<uint64_t>(0);
			}

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
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
							Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound)});
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound));
						return;
					}
					else
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
							Modio::ModManagementEvent {CurrentModID,
													   Modio::ModManagementEvent::EventType::BeginUpload,
													   {}});
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
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
							Modio::ErrorCodeMatches(ec, Modio::GenericError::OperationCanceled)
								? Modio::make_error_code(Modio::ModManagementError::UploadCancelled)
								: ec});
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(ec);
						return;
					}

					{
						// Determine the file size of the compressed modfile
						ZipFileSize = Modio::filesystem::file_size(ArchivePath, ec);

						if (ec)
						{
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
								Modio::ModManagementEvent {CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
														   ec});
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
								Modio::ModManagementEvent {CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
														   ec});
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

					if (ec)
					{
						// Don't need to marshal generic cancellation here because UploadFileAsync returns
						// upload-specific cancellation code already
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded, ec});
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(ec);
						return;
					}
					else
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(
							Modio::ModManagementEvent {CurrentModID,
													   Modio::ModManagementEvent::EventType::Uploaded,
													   {}});
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete({});
						return;
					}
				}
			}

			Modio::Detail::HttpRequestParams CreateRequestParams(HttpRequestParams AddRequest, Modio::ModID ModID,
																 Modio::CreateModFileParams Params)
			{
				Modio::Detail::HttpRequestParams RequestParams =
					AddRequest.SetModID(ModID)
						.AppendPayloadValue("version", Params.Version)
						.AppendPayloadValue("changelog", Params.Changelog)
						.AppendPayloadValue("metadata_blob", Params.MetadataBlob);

				if (Params.bSetAsActive)
				{
					RequestParams = RequestParams.AppendPayloadValue("active", *Params.bSetAsActive ? "true" : "false");
				}
				if (Params.Platforms)
				{
					// sort Platforms vector and remove duplicates
					std::sort(Params.Platforms.value().begin(), Params.Platforms.value().end());
					auto Last = std::unique(Params.Platforms.value().begin(), Params.Platforms.value().end());
					Params.Platforms.value().erase(Last, Params.Platforms.value().end());

					// loop through vector to append appropriate value to request
					std::size_t i = 0;
					for (const Modio::ModfilePlatform Platform : *Params.Platforms)
					{
						switch (Platform)
						{
							case (Modio::ModfilePlatform::Windows):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Windows);
								break;
							case (Modio::ModfilePlatform::Mac):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Mac);
								break;
							case (Modio::ModfilePlatform::Linux):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Linux);
								break;
							case (Modio::ModfilePlatform::Android):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Android);
								break;
							case (Modio::ModfilePlatform::iOS):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::iOS);
								break;
							case (Modio::ModfilePlatform::XboxOne):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::XboxOne);
								break;
							case (Modio::ModfilePlatform::XboxSeriesX):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i),
									Modio::Detail::Constants::PlatformNames::XboxSeriesX);
								break;
							case (Modio::ModfilePlatform::PS4):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::PS4);
								break;
							case (Modio::ModfilePlatform::PS5):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::PS5);
								break;
							case (Modio::ModfilePlatform::Switch):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Switch);
								break;
							case (Modio::ModfilePlatform::Oculus):
								RequestParams = RequestParams.AppendPayloadValue(
									fmt::format("platforms[{}]", i), Modio::Detail::Constants::PlatformNames::Oculus);
								break;
							default:
								Modio::Detail::Logger().Log(
									LogLevel::Warning, LogCategory::File,
									"Platform {} does not match any Modio::ModfilePlatform values. "
									"Unable to append to SubmitNewModFileForMod request.",
									Platform);
						}
						i++;
					}
				}

				return RequestParams;
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams SubmitParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			std::shared_ptr<Modio::Detail::UploadSession> Session = {};
			Modio::filesystem::path ArchivePath;
			Modio::filesystem::path ModRootDirectory;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
			std::shared_ptr<uint64_t> FileHash;
			Modio::ModID CurrentModID;
			Modio::CreateModFileParams CurrentModParams;
			std::uintmax_t ZipFileSize = 0;
		};
#include <asio/unyield.hpp>
		template<typename SubmitDoneCallback>
		auto SubmitNewModFileAsync(Modio::ModID ModID, Modio::CreateModFileParams Params, SubmitDoneCallback&& Callback)
		{
			return asio::async_compose<SubmitDoneCallback, void(Modio::ErrorCode)>(
				Modio::Detail::SubmitNewModFileOp(ModID, Params), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
