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

#include "android/detail/ops/file/DeleteFileOp.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"

namespace
{
	template<typename DeleteCallback>
	auto RecursivelyDeleteFile(Modio::filesystem::path FilePath,
							   std::shared_ptr<Modio::Detail::FileSharedState> SharedState,
							   DeleteCallback&& OnDeleteDone)
	{
		return asio::async_compose<DeleteCallback, void(Modio::ErrorCode)>(
			DeleteFileOp(FilePath, SharedState), OnDeleteDone,
			Modio::Detail::Services::GetGlobalContext().get_executor());
	}
} // namespace

#include <asio/yield.hpp>
/// <summary>
/// This class recursively iterates over a specified folder and either calls itself on subfolders, or asynchronously
/// deletes files
/// </summary>
class DeleteFolderOp
{
public:
	DeleteFolderOp(Modio::filesystem::path FolderPath, std::shared_ptr<Modio::Detail::FileSharedState> SharedState)
		: FolderPath(FolderPath),
		  SharedState(SharedState) {};

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		std::shared_ptr<Modio::Detail::FileSharedState> PinnedState = SharedState.lock();
		if (PinnedState != nullptr && PinnedState->bCancelRequested)
		{
			Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
			return;
		}

		reenter(CoroutineState)
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Begin delete of {}",
										FolderPath.string());

			// Collect all files and directories first
			{
				DirectoryIterator = Modio::filesystem::recursive_directory_iterator(FolderPath, ec);
				if (ec)
				{
					Self.complete(ec);
					return;
				}

				while (DirectoryIterator != Modio::filesystem::recursive_directory_iterator())
				{
					if (DirectoryIterator->is_directory(ec))
					{
						Folders.emplace_back(DirectoryIterator->path(), DirectoryIterator.depth());
					}
					else
					{
						Files.push_back(DirectoryIterator->path());
					}

					DirectoryIterator.increment(ec);
					if (ec)
					{
						Self.complete(ec);
						return;
					}
				}
			}

			// Delete all files
			FileIndex = 0;
			while (FileIndex < Files.size())
			{
				yield DeleteFileOp(Files[FileIndex], PinnedState)(Self);
				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Delete File {} error: {}", Files[FileIndex].string(),
												ec.message());
					Self.complete(ec);
					return;
				}
				FileIndex++;
			}

			// Sort in decreasing order of depth
			std::sort(Folders.begin(), Folders.end(),
					  [](const std::pair<Modio::filesystem::path, std::size_t>& A,
						 const std::pair<Modio::filesystem::path, std::size_t>& B) { return A.second > B.second; });

			// Delete directories
			FolderIndex = 0;
			while (FolderIndex < Folders.size())
			{
				Modio::filesystem::remove(Folders[FolderIndex].first, ec);
				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Delete Directory {} error: {}", Folders[FolderIndex].first.string(),
												ec.message());
					Self.complete(ec);
					return;
				}
				FolderIndex++;
			}

			// Make sure to remove the original folder path
			Modio::filesystem::remove(FolderPath, ec);
			if (ec)
			{
				Self.complete(ec);
			}
			else
			{
				Self.complete({});
			}
		}
	}

private:
	asio::coroutine CoroutineState {};
	Modio::filesystem::path FolderPath {};
	std::vector<Modio::filesystem::path> Files {};
	std::vector<std::pair<Modio::filesystem::path, int>> Folders {};
	Modio::filesystem::recursive_directory_iterator DirectoryIterator {};
	std::weak_ptr<Modio::Detail::FileSharedState> SharedState {};
	size_t FileIndex = 0;
	size_t FolderIndex = 0;
};

#include <asio/unyield.hpp>
