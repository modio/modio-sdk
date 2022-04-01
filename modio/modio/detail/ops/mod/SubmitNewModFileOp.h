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
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ops/UploadFileOp.h"
#include "modio/detail/ops/compression/CompressFolderOp.h"
#include "modio/file/ModioFileService.h"
#include "modio/core/ModioLogger.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitNewModFileOp
		{
		public:
			SubmitNewModFileOp(Modio::ModID ModID, Modio::CreateModFileParams Params) : CurrentModID(ModID)
			{
				SubmitParams =
					Modio::Detail::AddModfileRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
						.SetModID(ModID)
						.AppendPayloadValue("version", Params.Version)
						.AppendPayloadValue("changelog", Params.Changelog)
						.AppendPayloadValue("metadata_blob", Params.MetadataBlob);
				if (Params.bSetAsActive)
				{
					SubmitParams.AppendPayloadValue("active", Params.bSetAsActive ? "true" : "false");
				}
				ModRootDirectory = Params.RootDirectory;
				ArchivePath = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeTempFilePath(
					fmt::format("modfile_{}.zip", ModID)).value_or("");
				ProgressInfo = Modio::Detail::SDKSessionData::StartModDownloadOrUpdate(CurrentModID);
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
					Modio::Detail::Logger().Log(
						Modio::LogLevel::Info, Modio::LogCategory::ModManagement,
						"Beginning submission of mod {}", CurrentModID);
					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DirectoryExists(
							ModRootDirectory) || ArchivePath.empty())
					{
						Modio::Detail::SDKSessionData::GetModManagementEventLog().AddEntry(Modio::ModManagementEvent {
							CurrentModID, Modio::ModManagementEvent::EventType::Uploaded,
							Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound)});
						Modio::Detail::SDKSessionData::CancelModDownloadOrUpdate(CurrentModID);
						Self.complete(Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound));
						return;
					}

					yield Modio::Detail::CompressFolderAsync(ModRootDirectory, ArchivePath, ProgressInfo,
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

					yield Modio::Detail::UploadFileAsync(ResponseBuffer,
														 SubmitParams.AppendPayloadFile("filedata", ArchivePath),
														 ProgressInfo, std::move(Self));

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

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams SubmitParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::filesystem::path ArchivePath;
			Modio::filesystem::path ModRootDirectory;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
			Modio::ModID CurrentModID;
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