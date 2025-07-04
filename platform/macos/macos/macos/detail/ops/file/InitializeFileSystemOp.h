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
#include "fmt/format.h"
#include "macos/FileSharedState.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	namespace Detail
	{
		class InitializeFileSystemOp
		{
		public:
			InitializeFileSystemOp(Modio::InitializeOptions InitParams,
								   std::shared_ptr<Modio::Detail::FileSharedState> SharedState,
								   Modio::filesystem::path& CommonDataPath, Modio::filesystem::path& UserDataPath,
								   Modio::filesystem::path& TempPath)
				: InitParams(InitParams),
				  SharedState(SharedState),
				  CommonDataPath(CommonDataPath),
				  UserDataPath(UserDataPath),
				  TempPath(TempPath)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if ((ec = SharedState->Initialize()))
				{
					Self.complete(ec);
					return;
				}
				if (!GetDefaultCommonDataPath(CommonDataPath))
				{
					Self.complete(Modio::make_error_code(Modio::FilesystemError::UnableToCreateFolder));
					return;
				}
				const char* HomeDir = std::getenv("HOME");
				if (HomeDir == nullptr)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
												"Could not get home directory environment variable!");
					Self.complete(Modio::make_error_code(Modio::FilesystemError::UnableToCreateFolder));
					return;
				}
				UserDataPath = Modio::filesystem::path(HomeDir) / "Library/Application Support/mod.io" /
							   fmt::format("{}/{}/", InitParams.GameID, InitParams.User);
				TempPath = Modio::filesystem::path("/tmp/");

				// EC should never be null at this point
				Self.complete(Modio::ErrorCode {});
			}

		private:
			Modio::InitializeOptions InitParams {};
			std::shared_ptr<Modio::Detail::FileSharedState> SharedState {};
			Modio::filesystem::path& CommonDataPath;
			Modio::filesystem::path& UserDataPath;
			Modio::filesystem::path& TempPath;
		};
	} // namespace Detail
} // namespace Modio
