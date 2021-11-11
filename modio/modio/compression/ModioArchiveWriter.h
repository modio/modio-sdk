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

#include "modio/compression/ModioCompressionService.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include <memory>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		class ArchiveWriter : public asio::basic_io_object<CompressionService>
		{
			Modio::filesystem::path FilePath;

		public:
			explicit ArchiveWriter(Modio::filesystem::path FilePath)
				: asio::basic_io_object<CompressionService>(Modio::Detail::Services::GetGlobalContext()),
				  FilePath(FilePath)
			{
				get_implementation()->FilePath = FilePath;
			};
			ArchiveWriter(ArchiveWriter&& Other)
				: asio::basic_io_object<CompressionService>(std::move(Other)),
				  FilePath(std::move(Other.FilePath))
			{}

			// TODO: @modio-core take a reference to a progress info object?

			/// @brief Compresses the specified file and writes its data into the archive
			/// @tparam CompletionHandlerType Type of the Callable being used as the handler
			/// @param SourceFilePath Path to the file to compress
			/// @param PathInsideArchive Path to use for the entry inside the archive - will be used as the destination
			/// during extraction. Must be a relative path
			/// @param Handler Callable invoked with the results of the operation
			template<typename CompletionHandlerType>
			auto AddFileEntryToArchiveAsync(Modio::filesystem::path SourceFilePath,
											Modio::filesystem::path PathInsideArchive,
											std::weak_ptr<class Modio::ModProgressInfo> ProgressInfo,
											CompletionHandlerType&& Handler)
			{
				get_service().AddFileEntryAsync(get_implementation(), SourceFilePath, PathInsideArchive, ProgressInfo,
												std::forward<CompletionHandlerType>(Handler));
			}

			/// @brief Adds an empty/virtual directory entry to the archive.
			/// @tparam CompletionHandlerType Type of the Callable being used as the handler
			/// @param DirectoryPath Path to use for the entry inside the archive - must be a relative path.
			/// @param Handler Callable invoked with the result of the operation
			template<typename CompletionHandlerType>
			auto AddDirectoryEntryToArchiveAsync(Modio::filesystem::path DirectoryPath, CompletionHandlerType&& Handler)
			{
				get_service().AddDirectoryEntryAsync(get_implementation(), DirectoryPath,
													 std::forward<CompletionHandlerType>(Handler));
			}

			/// @brief Finalizes the archive by writing any required header or footer data to the file
			/// @tparam CompletionHandlerType Type of the Callable being used as the handler
			/// @param Handler Callable invoked with the result of the operation
			template<typename CompletionHandlerType>
			auto FinalizeArchiveAsync(CompletionHandlerType&& Handler)
			{
				get_service().FinalizeArchiveAsync(get_implementation(), std::forward<CompletionHandlerType>(Handler));
			}
		};
	} // namespace Detail
} // namespace Modio
