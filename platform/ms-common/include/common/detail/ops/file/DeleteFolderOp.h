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

#include "common/detail/ops/file/DeleteFileOp.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"

namespace
{
	template<typename DeleteCallback>
	auto RecursivelyDeleteFile(Modio::filesystem::path FilePath, DeleteCallback&& OnDeleteDone)
	{
		return asio::async_compose<DeleteCallback, void(Modio::ErrorCode)>(
			DeleteFileOp(FilePath), OnDeleteDone, Modio::Detail::Services::GetGlobalContext().get_executor());
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
	DeleteFolderOp(Modio::filesystem::path FolderPath) : FolderPath(FolderPath) {};
	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		reenter(CoroutineState)
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File, "Begin delete of {}",
										FolderPath.string());
			{
				DirectoryIterator = Modio::filesystem::recursive_directory_iterator(FolderPath, ec);
				if (ec)
				{
					Self.complete(ec);
					return;
				}
			}
			while (DirectoryIterator != end(DirectoryIterator))
			{
				// If entry is a folder, mark it for later deletion
				if (DirectoryIterator->is_directory(ec))
				{
					Folders.push_back(std::make_pair(DirectoryIterator->path(), DirectoryIterator.depth()));
				}
				else
				{
					yield RecursivelyDeleteFile(DirectoryIterator->path(), std::move(Self));
				}
				DirectoryIterator.increment(ec);
				if (ec)
				{
					Self.complete(ec);
					return;
				}
			}
			// Sort in decreasing order of depth
			std::sort(Folders.begin(), Folders.end(),
					  [](const std::pair<Modio::filesystem::path, std::size_t>& A,
						 const std::pair<Modio::filesystem::path, std::size_t>& B) { return A.second > B.second; });

			for (auto Folder : Folders)
			{
				Modio::filesystem::remove(Folder.first, ec);
				if (ec)
				{
					Self.complete(ec);
					return;
				}
			}

			Self.complete(ec);
		}
	}

private:
	asio::coroutine CoroutineState;
	Modio::filesystem::path FolderPath;
	std::vector<std::pair<Modio::filesystem::path, int>> Folders;
	Modio::filesystem::recursive_directory_iterator DirectoryIterator;
};

#include <asio/unyield.hpp>
