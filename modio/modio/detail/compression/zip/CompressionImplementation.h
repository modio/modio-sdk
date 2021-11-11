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

#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/ops/compression/zip/AddDirectoryEntryOp.h"
#include "modio/detail/ops/compression/zip/AddFileEntryOp.h"
#include "modio/detail/ops/compression/zip/FinalizeArchiveOp.h"

namespace Modio
{
	namespace Detail
	{
		class CompressionImplementation
		{
		public:
			using IOObjectImplementationType = std::shared_ptr<ArchiveFileImplementation>;

			void InitializeIOObjectImplementation(IOObjectImplementationType& Implementation)
			{
				Implementation.reset(new ArchiveFileImplementation());
			}

			void MoveIOObjectImplementation(IOObjectImplementationType& Implementation,
											IOObjectImplementationType& OtherImplementation)
			{
				Implementation = std::move(OtherImplementation);
			}

			template<typename CompletionHandlerType>
			auto AddFileEntryAsync(IOObjectImplementationType& PlatformIOObject, Modio::filesystem::path SourceFilePath,
								   Modio::filesystem::path PathInsideArchive, std::weak_ptr<class Modio::ModProgressInfo> ProgressInfo,  CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					Modio::Detail::AddFileEntryOp(PlatformIOObject, SourceFilePath, PathInsideArchive, ProgressInfo),
					Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionHandlerType>
			auto AddDirectoryEntryAsync(IOObjectImplementationType& PlatformIOObject,
										Modio::filesystem::path DirectoryPath, CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					Modio::Detail::AddDirectoryEntryOp(PlatformIOObject, DirectoryPath),
					Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionHandlerType>
			auto FinalizeArchiveAsync(IOObjectImplementationType& PlatformIOObject, CompletionHandlerType&& Handler)
			{
				return asio::async_compose<CompletionHandlerType, void(Modio::ErrorCode)>(
					Modio::Detail::FinalizeArchiveOp(PlatformIOObject), Handler,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		};
	} // namespace Detail
} // namespace Modio
