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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include <algorithm>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class LoadGlobalConfigOverrideFileDataOp
		{
		public:
			LoadGlobalConfigOverrideFileDataOp(Modio::Detail::DynamicBuffer FileBuffer) : FileBuffer(FileBuffer)
			{
				ConfigJson = std::make_unique<nlohmann::json>();
			};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();

				reenter(CoroutineState)
				{
					FileBuffer.Clear();
					// Special-casing the path to the global settings directory as it should be in the user data folder
					// but not namespaced to game or game-specific user string
					if (FileService.FileExists(FileService.UserDataFolder() / "../../globalsettings.json"))
					{
						ConfigFile = std::make_unique<Modio::Detail::File>(
							FileService.UserDataFolder() / "../../globalsettings.json", false);
						yield ConfigFile->ReadAsync(ConfigFile->GetFileSize(), FileBuffer, std::move(Self));
						if (ec)
						{
							// warn about not being able to read the config file for whatever reason
						}
						if (FileBuffer.size() > 0)
						{
							ConfigJson = std::make_unique<nlohmann::json>(Modio::Detail::ToJson(FileBuffer));
							if (*ConfigJson != nlohmann::json::value_t::discarded)
							{
								if (ConfigJson->contains(Modio::Detail::Constants::JSONKeys::RootLocalStoragePath))
								{
									Self.complete({});
									return;
								}
							}
						}
					}
					// We are either here because there was no file, a zero length file (so ConfigJson is null) or a non
					// zero length file which either fails to parse or has the rootlocalstoragepath missing(therefore
					// ConfigJson is non null but invalid) so just recreate it
					ConfigJson = std::make_unique<nlohmann::json>();
					(*ConfigJson)[Modio::Detail::Constants::JSONKeys::RootLocalStoragePath] =
						FileService.GetRootLocalStoragePath().generic_u8string();

					{
						std::string DefaultConfigString = ConfigJson->dump();
						DefaultConfigBuffer = std::make_unique<Modio::Detail::Buffer>(DefaultConfigString.size());
						if (!ConfigFile)
						{
							ConfigFile = std::make_unique<Modio::Detail::File>(
								FileService.UserDataFolder() / "../../globalsettings.json", false);
						}
						std::copy(DefaultConfigString.begin(), DefaultConfigString.end(), DefaultConfigBuffer->Data());
					}

					yield ConfigFile->WriteAsync(std::move(*DefaultConfigBuffer), std::move(Self));
					if (ec)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::File, "Error code after write async {}", ec.value());
					}
					FileBuffer.Clear();
					Self.complete(Modio::make_error_code(Modio::FilesystemError::FileNotFound));
					return;
				}
			}
			std::unique_ptr<Modio::Detail::Buffer> DefaultConfigBuffer;
			std::unique_ptr<nlohmann::json> ConfigJson;
			std::unique_ptr<Modio::Detail::File> ConfigFile;
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer FileBuffer;
		};

		template<typename CompletionTokenType>
		auto LoadGlobalConfigOverrideFileDataAsync(Modio::Detail::DynamicBuffer FileData, CompletionTokenType&& Token)
		{
			return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				LoadGlobalConfigOverrideFileDataOp(FileData), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>