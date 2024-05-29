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
#include "modio/core/ModioServices.h"
#include "modio/file/ModioFileService.h"

#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class UninstallModOp
		{
		public:
			UninstallModOp(Modio::ModID ModId, bool bForce, bool bIsTempMod)
				: ModId(ModId), bForce(bForce), bIsTempMod(bIsTempMod) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}
				reenter(CoroutineState)
				{
					InstallPath =
						bIsTempMod
							? Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeTempModPath(ModId)
							: Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().MakeModPath(ModId);

					yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFolderAsync(
						InstallPath, std::move(Self));

					// If the directory never existed, then don't call it a failure if we couldn't delete the files
					if (ec && (ec != std::errc::no_such_file_or_directory && ec != Modio::FilesystemError::DirectoryNotFound))
					{
						Modio::Detail::Logger().Log(
							LogLevel::Error, LogCategory::File,
							"DeleteFolderAsync during UninstallModOp was not successful, path: {} and error message: {}",
							InstallPath.string(), ec.message());

						// Mod will still be considered as installed as something went wrong when deleting the file
						Self.complete(ec);
						return;
					}

					Modio::ModCollection& Collection = 
						bIsTempMod 
							? Modio::Detail::SDKSessionData::GetTempModCollection() 
							: Modio::Detail::SDKSessionData::GetSystemModCollection();

					Collection.RemoveMod(ModId, bForce);

					Self.complete({});
				}
			}

		private:
			Modio::filesystem::path InstallPath;
			Modio::ModID ModId;
			asio::coroutine CoroutineState;
			bool bForce;
			bool bIsTempMod;
		};

		template<typename UninstallDoneCallback>
		auto UninstallModAsync(Modio::ModID Mod, UninstallDoneCallback&& OnUninstallComplete, bool bForce = false,
							   bool bIsTempMod = false)
		{
			return asio::async_compose<UninstallDoneCallback, void(Modio::ErrorCode)>(
				UninstallModOp(Mod, bForce, bIsTempMod), OnUninstallComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>