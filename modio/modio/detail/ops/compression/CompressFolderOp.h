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

#include "modio/compression/ModioArchiveWriter.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class CompressFolderOp
		{
		public:
			CompressFolderOp(Modio::filesystem::path SourceDirectoryRootPath,
							 Modio::filesystem::path DestinationArchivePath,
							 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: SourceDirectoryRootPath(SourceDirectoryRootPath),
				  ProgressInfo(ProgressInfo)
			{
				// need to delete the destination archive file if it exists
				if (Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().FileExists(
						DestinationArchivePath))
				{
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFile(
						DestinationArchivePath);
				}

				DestinationArchive = std::make_unique<Modio::Detail::ArchiveWriter>(DestinationArchivePath);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				std::shared_ptr<Modio::ModProgressInfo> PinnedProgressInfo = ProgressInfo.lock();
				if (PinnedProgressInfo == nullptr)
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				reenter(CoroutineState)
				{
					if (SourceDirectoryRootPath.has_filename())
					{
						SourceDirectoryRootPath = SourceDirectoryRootPath.parent_path();
					}

					EntriesInFolder = Modio::filesystem::recursive_directory_iterator(SourceDirectoryRootPath, ec);
					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
													"Unable to iterate over directory {}",
													SourceDirectoryRootPath.string());
						Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
						return;
					}
					// calculate the size of all the files in the folder here:
					CurrentEntry = begin(EntriesInFolder);
					while (CurrentEntry != end(EntriesInFolder))
					{
						CurrentRelativePath =
							Modio::filesystem::relative(CurrentEntry->path(), SourceDirectoryRootPath, ec);
						if (ec)
						{
							Self.complete(ec);
							return;
						}

						if (Modio::filesystem::is_regular_file(CurrentEntry->path(), ec))
						{
							PinnedProgressInfo->TotalExtractedSizeOnDisk +=
								Modio::FileSize(Modio::filesystem::file_size(CurrentEntry->path(), ec));
							if (ec)
							{
								Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
								return;
							}
						}
						CurrentEntry.increment(ec);
					}

					//Reset the iterator
					EntriesInFolder = Modio::filesystem::recursive_directory_iterator(SourceDirectoryRootPath, ec);
					CurrentEntry = begin(EntriesInFolder);
					while (CurrentEntry != end(EntriesInFolder))
					{
						CurrentRelativePath =
							Modio::filesystem::relative(CurrentEntry->path(), SourceDirectoryRootPath, ec);
						if (ec)
						{
							Self.complete(ec);
							return;
						}

						if (Modio::filesystem::is_regular_file(CurrentEntry->path(), ec))
						{
							yield DestinationArchive->AddFileEntryToArchiveAsync(CurrentEntry->path(),
																				 CurrentRelativePath, ProgressInfo,  std::move(Self));
							if (ec)
							{
								Self.complete(ec);
								return;
							}
						}
						else if (!ec &&
								 Modio::filesystem::is_directory(
									 CurrentEntry->path(), ec)) // Only fall back to checking if it's a directory, if we
																// didnt get an error checking the file type above
						{
							yield DestinationArchive->AddDirectoryEntryToArchiveAsync(CurrentRelativePath / "",
																					  std::move(Self));
							if (ec)
							{
								Self.complete(ec);
								return;
							}
						}
						else // error checking file_type
						{
							Self.complete(ec);
							return;
						}
						CurrentEntry.increment(ec);
					}

					yield DestinationArchive->FinalizeArchiveAsync(std::move(Self));

					Self.complete(ec);
					return;
				}
			}

		private:
			std::unique_ptr<Modio::Detail::ArchiveWriter> DestinationArchive;
			asio::coroutine CoroutineState;
			Modio::filesystem::path SourceDirectoryRootPath;
			Modio::filesystem::recursive_directory_iterator EntriesInFolder;
			Modio::filesystem::recursive_directory_iterator CurrentEntry;
			Modio::filesystem::path CurrentRelativePath;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
		};
#include <asio/unyield.hpp>

		template<typename CompletionHandlerType>
		auto CompressFolderAsync(Modio::filesystem::path FolderToCompress, Modio::filesystem::path PathToOutputArchive,
								 std::weak_ptr<Modio::ModProgressInfo> ProgressInfo, CompletionHandlerType&& Handler)
		{
			return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
				CompressFolderOp(FolderToCompress, PathToOutputArchive, ProgressInfo), Handler,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio