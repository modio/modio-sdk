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
			InstallOrUpdateModOp(Modio::ModID Mod, bool IsTempMod) : Mod(Mod), IsTempMod(IsTempMod)
			{
				ModProgress = Modio::Detail::SDKSessionData::StartModDownloadOrUpdate(Mod);
			}
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, Modio::FileSize ExtractedSize = {})
			{
				MODIO_PROFILE_SCOPE(InstallOrUpdateMod);
				// We dont care whether we couldn't start the download because something was blocking it or if it's
				// since been cancelled, bail anyways. null pointer ie the busy case should never happen
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
					return;
				}
				if (ModProgress.lock() == nullptr)
				{
					// Avoid rollback of state when ModProgress is null, should only happen when unsubing to an
					// installing mod
					Transaction.Commit();
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
					return;
				}

				reenter(CoroutineState)
				{
					CollectionEntry = IsTempMod
										  ? Modio::Detail::SDKSessionData::GetTempModCollection().Entries().at(Mod)
										  : Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod);
					Transaction = BeginTransaction(CollectionEntry);

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ModInfoBuffer,
						Modio::Detail::GetModRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.SetModID(Mod)
							.AddPlatformStatusFilter()
							.AddStatusFilter(),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}

					CollectionEntry->SetModState(ModState::Downloading);

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
													ModInfoData.ProfileName);
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::GenericError::NoDataAvailable));
						return;
					}

					// Check the local mod storage quota.  This quota applies to both regular and temp mods.
					if (Modio::StorageInfo::GetQuota(Modio::StorageLocation::Local).has_value())
					{
						Modio::StorageInfo Storage = Modio::QueryStorageInfo();
						if (ModInfoData.FileInfo->FilesizeUncompressed >
							Storage.GetSpace(Modio::StorageLocation::Local, Modio::StorageUsage::Available))
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
														"Installing mod {} would exceed the local mod storage quota",
														ModInfoData.ModId);
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
							return;
						}
					}

					if (IsTempMod)
					{
						// Temp mod quota is not yet implemented
						if (Modio::Optional<Modio::FileSize> TempQuota =
								Modio::Detail::SDKSessionData::GetTempModStorageQuota())
						{
							const auto TempModSet = Modio::QueryTempModSet();
							Modio::FileSize AvailableSpace = TempQuota.value();
							for (auto& TempMod : TempModSet)
							{
								if (TempMod.second.GetSizeOnDisk().has_value())
								{
									if (AvailableSpace > TempMod.second.GetSizeOnDisk().value())
									{
										AvailableSpace -= TempMod.second.GetSizeOnDisk().value();
									}
									else
									{
										AvailableSpace = Modio::FileSize(0);
										break;
									}
								}
							}
							if (ModInfoData.FileInfo->FilesizeUncompressed > AvailableSpace)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
															"Installing mod {} would exceed the temp mod storage quota",
															ModInfoData.ModId);
								Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
								Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
								return;
							}
						}
					}

					// Is there room in the installation path for the extracted files?
					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CheckSpaceAvailable(
							CollectionEntry->GetPath(), Modio::FileSize(ModInfoData.FileInfo->FilesizeUncompressed)))
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
						return;
					}

					// Do we already have the file on disk?
					if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
							DownloadPath))
					{
						Modio::Detail::File DownloadedFile(DownloadPath, Modio::Detail::FileMode::ReadOnly);
						// Is the downloaded file the correct size?
						if (DownloadedFile.GetFileSize() == ModInfoData.FileInfo->Filesize)
						{
							bFileDownloadComplete = true;
						}
					}
					// If we either don't have the file on disk or its size is wrong, check if we have enough space to
					// download the file
					if (!bFileDownloadComplete &&
						!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CheckSpaceAvailable(
							DownloadPath, Modio::FileSize(ModInfoData.FileInfo->Filesize)))
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
						return;
					}

					// this check may be redundant given the above check
					if (std::shared_ptr<Modio::ModProgressInfo> MPI = ModProgress.lock())
					{
						SetState(*MPI.get(), Modio::ModProgressInfo::EModProgressState::Downloading);
						SetTotalProgress(*MPI.get(), Modio::ModProgressInfo::EModProgressState::Downloading,
										 Modio::FileSize(ModInfoData.FileInfo->Filesize));
						if (bFileDownloadComplete)
						{
							CompleteProgressState(*MPI.get(), Modio::ModProgressInfo::EModProgressState::Downloading);
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
							LogLevel::Detailed, LogCategory::Http,
							"Starting download of modfile `{}` total size `{}`, for mod {} \"{}\"",
							ModInfoData.FileInfo->Filename, ModInfoData.FileInfo.value().Filesize, ModInfoData.ModId,
							ModInfoData.ProfileName);

						yield Modio::Detail::DownloadFileAsync(
							Modio::Detail::HttpRequestParams::FileDownload(ModInfoData.FileInfo->DownloadBinaryURL)
								.value(),
							DownloadPath, ModProgress, ModInfoData.FileInfo.value().Filesize, std::move(Self));
						if (ec)
						{
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							Self.complete(ec);
							return;
						}
					}

					CollectionEntry->SetModState(ModState::Extracting);

					if (Modio::filesystem::exists(CollectionEntry->GetPath(), ec) && !ec)
					{
						yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFolderAsync(
							CollectionEntry->GetPath(), std::move(Self));
						if (ec)
						{
							Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
														"DeleteFolderAsync during InstallOrUpdateModOp was not "
														"successful, path: {} and error message: ",
														CollectionEntry->GetPath(), ec.message());

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

					yield Modio::Detail::ExtractAllFilesAsync(DownloadPath, CollectionEntry->GetPath(), ModProgress,
															  std::move(Self));
					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}

					// TODO: @modio-core perhaps resolve the file path issue by storing the mod path on the entry
					// here rather than dynamically calculating it elsewhere?
					CollectionEntry->UpdateSizeOnDisk(ExtractedSize);
					CollectionEntry->SetModState(ModState::Installed);
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
			Modio::ModID Mod {};
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ModInfoBuffer {};
			Modio::filesystem::path DownloadPath {};
			bool IsTempMod {};
			std::shared_ptr<Modio::ModCollectionEntry> CollectionEntry {};
			Modio::ModInfo ModInfoData {};
			Modio::Detail::Transaction<Modio::ModCollectionEntry> Transaction {};
			std::weak_ptr<Modio::ModProgressInfo> ModProgress {};
			bool bFileDownloadComplete = false;
		};

		template<typename InstallDoneCallback>
		auto InstallOrUpdateModAsync(Modio::ModID Mod, bool IsTempMod, InstallDoneCallback&& OnInstallComplete)
		{
			return ModioAsio::async_compose<InstallDoneCallback, void(Modio::ErrorCode)>(
				InstallOrUpdateModOp(Mod, IsTempMod), OnInstallComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>

MODIO_DIAGNOSTIC_POP