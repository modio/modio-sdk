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
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include <ShlObj.h>
#include <memory>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @brief Default implementation of the file system initialization on MS platforms. Can be re-used on any
		/// platform where the user identifier passed into InitializeAsync is a std::string.
		class InitializeFileSystemOp
		{
		public:
			InitializeFileSystemOp(Modio::InitializeOptions InitParams, Modio::filesystem::path& CommonDataPath,
								   Modio::filesystem::path& UserDataPath, Modio::filesystem::path& TempPath)
				: InitParams(InitParams),
				  CommonDataPath(CommonDataPath),
				  UserDataPath(UserDataPath),
				  TempPath(TempPath)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				/*if (!Modio::Detail::SDKSessionData::IsInitialized())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}*/
				reenter(CoroutineState)
				{
					// Get root data path in program data
					{
						PWSTR path = NULL;

#ifdef XDK_HAXX
						HRESULT hr;
#else
						HRESULT hr = SHGetKnownFolderPath(FOLDERID_Public, 0, NULL, &path);
#endif

						if (SUCCEEDED(hr))
						{
							CommonDataPath = Modio::filesystem::path(std::wstring(path));
							CommonDataPath /= fmt::format("mod.io/");
							CoTaskMemFree(path);
						}
						else
						{
							CoTaskMemFree(path);
							Self.complete(Modio::make_error_code(Modio::FilesystemError::UnableToCreateFolder));
							return;
						}
					}

					// Get user data directory in AppData
					{
						Modio::filesystem::path UDF;
						PWSTR path = NULL;

#ifdef XDK_HAXX
						HRESULT hr;
#else
						HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
#endif

						if (SUCCEEDED(hr))
						{
							UDF = Modio::filesystem::path(std::wstring(path));
							UDF /= Modio::filesystem::path(
								fmt::format("mod.io/{}/{}/", InitParams.GameID, InitParams.User));
							CoTaskMemFree(path);
							UserDataPath = UDF;
						}
						else
						{
							CoTaskMemFree(path);
							Self.complete(Modio::make_error_code(Modio::FilesystemError::UnableToCreateFolder));
							return;
						}
					}

					TempPath = Modio::filesystem::temp_directory_path(ec);
					Self.complete(Modio::ErrorCode {});
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::InitializeOptions InitParams;
			Modio::filesystem::path& CommonDataPath;
			Modio::filesystem::path& UserDataPath;
			Modio::filesystem::path& TempPath;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
