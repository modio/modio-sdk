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
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/DownloadFileOp.h"
#include "modio/detail/ops/compression/ExtractAllToFolderOp.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/file/ModioFileService.h"


MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS


#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class InstallOrUpdateModOp : public Modio::Detail::BaseOperation<InstallOrUpdateModOp>
		{
		public:
			InstallOrUpdateModOp(Modio::ModID Mod) : Mod(Mod)
			{
				ModProgress = Modio::Detail::SDKSessionData::StartModDownloadOrUpdate(Mod);
			};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, Modio::FileSize ExtractedSize = {})
			{
				MODIO_PROFILE_SCOPE(InstallOrUpdateMod);
				// We dont care whether we couldn't start the download because something was blocking it or if it's
				// since been cancelled, bail anyways. null pointer ie the busy case should never happen
				if (ModProgress.lock() == nullptr)
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
					return;
				}
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
					return;
				}
				reenter(CoroutineState)
				{
					Transaction =
						BeginTransaction(Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod));

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ModInfoBuffer,
						Modio::Detail::GetModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.SetModID(Mod),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}

					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->SetModState(
						ModState::Downloading);

					// TryMarshalResponse to get ModInfo object
					if (Modio::Optional<Modio::ModInfo> ModInfoResponse =
							TryMarshalResponse<Modio::ModInfo>(ModInfoBuffer))
					{
						ModInfoData = std::move(*ModInfoResponse);
					}
					else
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
						return;
					}

					// Check if we have valid FileInfo, and create download path if valid
					if (ModInfoData.FileInfo.has_value())
					{
						if (Modio::Optional<Modio::filesystem::path> TempFilePath =
								Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
									.MakeTempFilePath(ModInfoData.FileInfo->Filename))
						{
							DownloadPath = std::move(*TempFilePath);
						}
						else
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
														"Couldn't create temp file {}", ModInfoData.FileInfo->Filename);
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							Self.complete(Modio::make_error_code(Modio::FilesystemError::UnableToCreateFile));
							return;
						}
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
													"Data received for mod {} contains no modfile information",
													ModInfoData.FileInfo->Filename);
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::GenericError::NoDataAvailable));
						return;
					}

					// this check may be redundant given the above check
					if (std::shared_ptr<Modio::ModProgressInfo> MPI = ModProgress.lock())
					{
						SetState(*MPI.get(), Modio::ModProgressInfo::EModProgressState::Downloading);
						SetTotalProgress(*MPI.get(), Modio::ModProgressInfo::EModProgressState::Downloading, Modio::FileSize(ModInfoData.FileInfo->Filesize));

						if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
								 .CheckSpaceAvailable(DownloadPath, MPI->GetTotalProgress(Modio::ModProgressInfo::EModProgressState::Downloading)))
						{
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
							return;
						}
						// check if a fully downloaded file exists to prevent unnecessary redownloads
						if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
								DownloadPath))
						{
							Modio::Detail::File DownloadedFile(DownloadPath, Modio::Detail::FileMode::ReadWrite);
							if (DownloadedFile.GetFileSize() ==
								MPI->GetTotalProgress(Modio::ModProgressInfo::EModProgressState::Downloading))
							{
								bFileDownloadComplete = true;
								CompleteProgressState(*MPI.get(),
													  Modio::ModProgressInfo::EModProgressState::Downloading);
							}
						}
					}
					else
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
						return;
					}

					if (!bFileDownloadComplete)
					{
						Modio::Detail::Logger().Log(
							LogLevel::Info, LogCategory::Http, "Starting download of modfile {} for mod {} \"{}\"",
							ModInfoData.FileInfo->Filename, ModInfoData.ModId, ModInfoData.ProfileName);

						yield Modio::Detail::DownloadFileAsync(
							Modio::Detail::HttpRequestParams::FileDownload(ModInfoData.FileInfo->DownloadBinaryURL)
								.value(),
							DownloadPath, ModProgress, std::move(Self));
						if (ec)
						{
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							Self.complete(ec);
							return;
						}
					}

					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->SetModState(
						ModState::Extracting);
					InstallPath =
						Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeModPath(Mod);

					if (Modio::filesystem::exists(InstallPath, ec) && !ec)
					{
						yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFolderAsync(
							InstallPath, std::move(Self));
						if (ec)
						{
							Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
														"DeleteFolderAsync during InstallOrUpdateModOp was not "
														"successful, path: {} and error message: ",
														InstallPath.string(), ec.message());

							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							// TODO: @Modio-core handle errors when trying to delete the installed mod folder
							Self.complete(ec);
							return;
						}
					}

					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}


					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CheckSpaceAvailable(
						InstallPath, Modio::FileSize(ModInfoData.FileInfo->FilesizeUncompressed)))
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
						return;
					}


					yield Modio::Detail::ExtractAllFilesAsync(DownloadPath, InstallPath, ModProgress, std::move(Self));

					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}

					// TODO: @modio-core perhaps resolve the file path issue by storing the mod path on the entry
					// here rather than dynamically calculating it elsewhere?
					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->UpdateSizeOnDisk(
						ExtractedSize);
					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->SetModState(
						ModState::Installed);
					// ToDO: @modio-core invoke GetFolderSizeAsync here to store the value into the modstate?

					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
							DownloadPath) == false)
					{
						Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::File,
													"Downloaded file {} was not removed", DownloadPath.string());
					}

					Transaction.Commit();

					// TODO: @modio-core update profile here
					Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
					Self.complete({});
					return;
				}
			}

		private:
			Modio::ModID Mod;
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ModInfoBuffer;
			Modio::filesystem::path DownloadPath;
			Modio::filesystem::path InstallPath;
			Modio::ModInfo ModInfoData;
			Modio::Detail::Transaction<Modio::ModCollectionEntry> Transaction;
			std::weak_ptr<Modio::ModProgressInfo> ModProgress;
			bool bFileDownloadComplete = false;
		};

		template<typename InstallDoneCallback>
		auto InstallOrUpdateModAsync(Modio::ModID Mod, InstallDoneCallback&& OnInstallComplete)
		{
			return asio::async_compose<InstallDoneCallback, void(Modio::ErrorCode)>(
				InstallOrUpdateModOp(Mod), OnInstallComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>


MODIO_DIAGNOSTIC_POP