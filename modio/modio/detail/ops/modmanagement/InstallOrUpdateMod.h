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
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/compression/ExtractAllToFolderOp.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/file/ModioFileService.h"
#include <nlohmann/json.hpp>

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
				// We dont care wether we couldnt start the download because something was blocking it or if its since
				// been cancelled, bail anyways. null pointer ie the busy case should never happen
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

					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
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

					{
						Modio::Detail::Buffer LinearBuffer(ModInfoBuffer.size());
						Modio::Detail::BufferCopy(LinearBuffer, ModInfoBuffer);

						ModInfoResponse =
							nlohmann::json::parse(LinearBuffer.begin(), LinearBuffer.end(), nullptr, false);
					}

					{
						std::string Filename;
						if (!Detail::ParseSubobjectSafe(ModInfoResponse, Filename, "modfile", "filename"))
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}

						Modio::Optional<Modio::filesystem::path> TempFilePath =
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeTempFilePath(
								Filename);

						if (!TempFilePath)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
														"couldn't create temp file {}", Filename);

							Self.complete(Modio::make_error_code(FilesystemError::UnableToCreateFile));
							return;
						}

						DownloadPath = std::move(*TempFilePath);
					}

					// this check may be redundant given the above check
					if (std::shared_ptr<Modio::ModProgressInfo> MPI = ModProgress.lock())
					{
						Detail::ParseSubobjectSafe(ModInfoResponse, MPI->TotalDownloadSize, "modfile", "filesize");
						if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
								 .CheckSpaceAvailable(DownloadPath, MPI->TotalDownloadSize))
						{
							Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
							return;
						}
						// check if a fully downloaded file exists to prevent unnecessary redownloads
						if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
								DownloadPath))
						{
							Modio::Detail::File DownloadedFile(DownloadPath);
							if (DownloadedFile.GetFileSize() == MPI->TotalDownloadSize)
							{
								bFileDownloadComplete = true;
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
							ModInfoResponse["modfile"]["filename"].get<std::string>(),
							ModInfoResponse["id"].get<Modio::ModID>(), ModInfoResponse["name"].get<std::string>());

						yield Modio::Detail::ComposedOps::DownloadFileAsync(
							Modio::Detail::HttpRequestParams::FileDownload(
								ModInfoResponse["modfile"]["download"]["binary_url"])
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
							Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
							// TODO: @Modio-core handle errors when trying to delete the installed mod folder
							Self.complete(ec);
							return;
						}
					}

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					yield Modio::Detail::ExtractAllFilesAsync(DownloadPath, InstallPath, ModProgress, std::move(Self));

					if (ec)
					{
						Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
						Self.complete(ec);
						return;
					}
					// TODO: @modio-core perhaps resolve the file path issue by storing the mod path on the entry here
					// rather than dynamically calculating it elsewhere?
					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->UpdateSizeOnDisk(
						ExtractedSize);
					Modio::Detail::SDKSessionData::GetSystemModCollection().Entries().at(Mod)->SetModState(
						ModState::Installed);
					// ToDO: @modio-core invoke GetFolderSizeAsync here to store the value into the modstate?

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
			nlohmann::json ModInfoResponse;
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
