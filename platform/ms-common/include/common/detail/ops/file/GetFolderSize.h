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

#include "common/detail/ops/file/DeleteFile.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		/// <summary>
		/// This class recursively iterates over a specified folder and either calls itself on subfolders, or
		/// asynchronously deletes files
		/// </summary>
		class GetFolderSizeOp
		{
		public:
			GetFolderSizeOp(Modio::filesystem::path FolderPath) : FolderPath(FolderPath), CurrentSize(0) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {}, std::size_t SizeToAdd = 0)
			{
				reenter(CoroutineState)
				{
					{
						Modio::ErrorCode ec;
						DirectoryIterator = Modio::filesystem::directory_iterator(FolderPath, ec);
						if (ec)
						{
							Self.complete(ec, 0);
							return;
						}
					}
					while (DirectoryIterator != end(DirectoryIterator))
					{
						if (DirectoryIterator->is_directory(ec))
						{
							yield GetFolderSizeAsync(DirectoryIterator->path(), std::move(Self));
							if (!ec)
							{
								CurrentSize += SizeToAdd;
							}
						}
						else
						{
							Modio::ErrorCode FileSizeError;
							CurrentSize += Modio::filesystem::file_size(DirectoryIterator->path(), FileSizeError);
							if (FileSizeError)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File, FileSizeError.message());
								Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError), 0);
							}
						}
						DirectoryIterator++;
					}
					Self.complete(ec, CurrentSize);
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::filesystem::path FolderPath;
			std::vector<std::pair<Modio::filesystem::path, int>> Folders;
			Modio::filesystem::directory_iterator DirectoryIterator;
			std::size_t CurrentSize;
		};

#include <asio/unyield.hpp>

		template<typename DeleteCallback>
		auto GetFolderSizeAsync(Modio::filesystem::path FilePath, DeleteCallback&& OnDeleteDone)
		{
			return asio::async_compose<DeleteCallback, void(Modio::ErrorCode, std::size_t)>(
				GetFolderSizeOp(FilePath), OnDeleteDone, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio