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
#include "common/detail/ops/file/InitializeFileSystemOp.h"
#include "common/file/FileSystemImplementation.h"

namespace Modio
{
	namespace Detail
	{
		class FileSystemImplementation
			: public Modio::Detail::FileSystemImplementationBase<FileSystemImplementation>
		{
		public:
			using FileSystemImplementationBase::FileSystemImplementationBase;
			
			auto MakeInitializeStorageOp(Modio::InitializeOptions InitParams,
										 Modio::filesystem::path& CommonPath, Modio::filesystem::path& UserPath, Modio::filesystem::path& TempPath)
			{
				return Modio::Detail::InitializeFileSystemOp(InitParams, CommonPath, UserPath, TempPath);
			}
		};
	} // namespace Detail
} // namespace Modio
