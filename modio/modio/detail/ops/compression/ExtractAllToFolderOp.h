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

#include "modio/compression/ModioArchiveReader.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/file/ModioFileService.h"

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class ExtractAllToFolderOp
		{
			struct ExtractAllImpl
			{
				asio::coroutine CoroutineState;
				Modio::filesystem::path ArchivePath;
				Modio::filesystem::path RootOutputPath;
				Modio::Detail::ArchiveReader ArchiveView;
				Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo;
				std::vector<Modio::Detail::ArchiveFileImplementation::ArchiveEntry>::iterator CurrentEntryIterator;
				ExtractAllImpl(asio::coroutine CoroutineState, Modio::filesystem::path ArchivePath,
							   Modio::filesystem::path RootOutputPath, Modio::Detail::ArchiveReader ArchiveView,
							   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
					: CoroutineState(CoroutineState),
					  ArchivePath(ArchivePath),
					  RootOutputPath(RootOutputPath),
					  ArchiveView(std::move(ArchiveView)),
					  ProgressInfo(ProgressInfo) {}
			};
			Modio::StableStorage<ExtractAllImpl> Impl;

		public:
			ExtractAllToFolderOp(ExtractAllToFolderOp&& Other)
			{
				this->Impl = Other.Impl;
			}
			ExtractAllToFolderOp(Modio::filesystem::path ArchivePath, Modio::filesystem::path RootOutputPath,
								 Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
			{
				Impl = std::make_shared<ExtractAllImpl>(ExtractAllImpl ({},
																		ArchivePath,
																		RootOutputPath,
																		Modio::Detail::ArchiveReader(ArchivePath),
																		ProgressInfo));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(ExtractAllToFolder);
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled), Modio::FileSize(0));
					return;
				}

				reenter(Impl->CoroutineState)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Compression,
												"Extracting archive {}", Impl->ArchivePath.u8string());

					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
							Impl->ArchivePath))
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::FileNotFound), Modio::FileSize(0));
						return;
					}

					// Parsing archive contents is an async operation because it requires reading/seeking around in
					// the archive file which is async
					yield Impl->ArchiveView.ParseArchiveContentsAsync(std::move(Self));

					if (ec)
					{
						Self.complete(ec, Modio::FileSize(0));
						return;
					}

					if (!Impl->RootOutputPath.is_absolute())
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::DirectoryNotFound),
									  Modio::FileSize(0));
						return;
					}

					if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CheckSpaceAvailable(
							Impl->RootOutputPath, Impl->ArchiveView.GetTotalExtractedSize()))
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace),
									  Modio::FileSize(0));
						return;
					}

					// Update the total size of the archive
					if (Impl->ProgressInfo.has_value())
					{
						if (!Impl->ProgressInfo->expired())
						{
							auto Info = Impl->ProgressInfo->lock();
							SetState(*Info.get(), Modio::ModProgressInfo::EModProgressState::Extracting);
							SetTotalProgress(*Info.get(), ModProgressInfo::EModProgressState::Extracting,
											 Impl->ArchiveView.GetTotalExtractedSize());
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled),
										  Modio::FileSize(0));
							return;
						}
					}

					// Now we know what entries are in the zip file, we need to loop through them and extract them
					// to disk
					Impl->CurrentEntryIterator = Impl->ArchiveView.begin();

					while (Impl->CurrentEntryIterator != Impl->ArchiveView.end())
					{
						if (Impl->CurrentEntryIterator->FilePath.has_parent_path())
						{
							// Security checks for file paths that attempt to escape from their intended destination
							// Blocks relative paths, double dots, and absolute paths
							if ((Impl->CurrentEntryIterator->FilePath.string().find(".\\") != std::string::npos) ||
								(Impl->CurrentEntryIterator->FilePath.string().find("./") != std::string::npos) ||
								(Impl->CurrentEntryIterator->FilePath.string().find("..") != std::string::npos) ||
								(Impl->CurrentEntryIterator->FilePath.string().find("\\") == 0) ||
								(Impl->CurrentEntryIterator->FilePath.string().find("/") == 0) ||
								(Impl->CurrentEntryIterator->FilePath.is_absolute()))
							{
								Modio::Detail::Logger().Log(
									Modio::LogLevel::Error, Modio::LogCategory::Http,
									"The path of the file to extract {} contains a forbidden sequence of characters",
									Impl->CurrentEntryIterator->FilePath.string());

								ec = Modio::make_error_code(Modio::FilesystemError::NoPermission);
								Self.complete(ec, Modio::FileSize(0));
								return;
							}
						}
						// If the current entry has no filename (ie it is just a directory), create that directory
						if (!Impl->CurrentEntryIterator->FilePath.has_filename() ||
							Impl->CurrentEntryIterator->bIsDirectory)
						{
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CreateFolder(
								Impl->RootOutputPath / Impl->CurrentEntryIterator->FilePath);
						}
						else
						{
							// Clear Stack
							yield asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),std::move(Self));
							// Otherwise, begin an async operation to extract that entry to a file with the path
							// based on the root path
							yield Impl->ArchiveView.ExtractEntryAsync(*Impl->CurrentEntryIterator, Impl->RootOutputPath,
																	  Impl->ProgressInfo, std::move(Self));
							if (ec)
							{
								Self.complete(ec, Modio::FileSize(0));
								return;
							}
						}
						Impl->CurrentEntryIterator++;
					}
					// Once all files are extracted, notify the caller
					Self.complete(Modio::ErrorCode {}, Impl->ArchiveView.GetTotalExtractedSize());
				}
			}
		};
#include <asio/unyield.hpp>

		template<typename CompletionTokenType>
		auto ExtractAllFilesAsync(Modio::filesystem::path ArchiveFile, Modio::filesystem::path AbsoluteDestinationPath,
								  Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo,
								  CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode, Modio::FileSize)>(
				ExtractAllToFolderOp(ArchiveFile, AbsoluteDestinationPath, ProgressInfo), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

MODIO_DIAGNOSTIC_POP