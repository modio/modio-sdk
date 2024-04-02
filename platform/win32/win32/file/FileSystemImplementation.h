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
#include "common/FileSharedState.h"
#include "common/file/FileSystemImplementation.h"

#include "common/detail/ops/file/InitializeFileSystemOp.h"
#include "common/detail/ops/file/StreamWriteOp.h"
#include "common/detail/ops/file/WriteSomeToFileOp.h"
#include "common/file/StaticDirectoriesImplementation.h"

namespace Modio
{
	namespace Detail
	{
		class FileSystemImplementation : public Modio::Detail::FileSystemImplementationBase<FileSystemImplementation>
		{
		public:
			using FileSystemImplementationBase::FileSystemImplementationBase;

			auto MakeInitializeStorageOp(Modio::InitializeOptions InitParams, Modio::filesystem::path& CommonPath,
										 Modio::filesystem::path& UserPath, Modio::filesystem::path& TempPath)
			{
				return Modio::Detail::InitializeFileSystemOp(InitParams, CommonPath, UserPath, TempPath);
			}

			auto MakeStreamWriteOp(IOObjectImplementationType PlatformIOObjectInstance, Modio::Detail::Buffer Buffer)
			{
				return StreamWriteOp(PlatformIOObjectInstance, SharedState, std::move(Buffer));
			}

			auto MakeWriteSomeToFileOp(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset, Modio::Detail::Buffer Buffer)
			{
				return WriteSomeToFileOp(PlatformIOObjectInstance, SharedState, Offset, std::move(Buffer));
			}

			auto MakeSharedState()
			{
				return std::make_shared<FileSharedState>();
			}

			static Modio::filesystem::path GetDefaultCommonDataPath(Modio::filesystem::path& CommonDataPath)
			{
				Modio::Detail::GetDefaultCommonDataPath(CommonDataPath);
				return CommonDataPath;
			}
		};
	} // namespace Detail
} // namespace Modio
