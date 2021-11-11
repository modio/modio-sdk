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
#include "modio/detail/ops/compression/ExtractEntryDeflateOp.h"
#include "modio/detail/ops/compression/ExtractEntryStoreOp.h"
#include "modio/detail/ops/compression/ParseArchiveContentsOp.h"
#include <memory>
#include <vector>

namespace Modio
{
	namespace Detail
	{
		class ArchiveReader : public asio::basic_io_object<CompressionService>
		{
			Modio::filesystem::path FilePath;

		public:
			explicit ArchiveReader(Modio::filesystem::path FilePath)
				: asio::basic_io_object<CompressionService>(Modio::Detail::Services::GetGlobalContext()),
				  FilePath(FilePath)
			{
				get_implementation()->FilePath = FilePath;
			};
			ArchiveReader(ArchiveReader&& Other)
				: asio::basic_io_object<CompressionService>(std::move(Other)),
				  FilePath(std::move(Other.FilePath))
			{}

			std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator begin()
			{
				return get_implementation()->begin();
			}

			std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator end()
			{
				return get_implementation()->end();
			}

			//TODO: @modio-core should be routed through the service instead
			template<typename CompletionHandlerType>
			auto ParseArchiveContentsAsync(CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					ParseArchiveContentsOp(get_implementation()), Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			//TODO: @modio-core should be routed through the service instead
			template<typename CompletionHandlerType>
			auto ExtractEntryAsync(ArchiveFileImplementation::ArchiveEntry Entry,
								   Modio::filesystem::path RootPathToExtractTo,
								   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo,
								   CompletionHandlerType&& Handler)
			{
			//This should live in the implementation perhaps?
				if (Entry.Compression == ArchiveFileImplementation::CompressionMethod::Deflate)
				{
					return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
						ExtractEntryDeflateOp(get_implementation(), Entry, RootPathToExtractTo, ProgressInfo), Handler,
						Modio::Detail::Services::GetGlobalContext().get_executor());
				}
				else if (Entry.Compression == ArchiveFileImplementation::CompressionMethod::Store)
				{
					return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
						ExtractEntryStoreOp(get_implementation(), Entry, RootPathToExtractTo, ProgressInfo), Handler,
						Modio::Detail::Services::GetGlobalContext().get_executor());
				}
				else
				{
					asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
							   [Handler = std::forward<CompletionHandlerType>(Handler)]() mutable {
								   Handler(Modio::make_error_code(Modio::ArchiveError::UnsupportedCompression));
							   });
				}
			}
			Modio::FileSize GetTotalExtractedSize()
			{
				return get_implementation()->TotalExtractedSize;
			}
		};
	} // namespace Detail
} // namespace Modio
