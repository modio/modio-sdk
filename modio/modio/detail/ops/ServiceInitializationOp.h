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
#include "modio/cache/ModioCacheService.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioServices.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/LoadGlobalConfigOverrideFileDataOp.h"
#include "modio/detail/ops/LoadModCollectionFromStorage.h"
#include "modio/detail/ops/ValidateAllInstalledModsOp.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include "modio/http/ModioHttpService.h"
#include "modio/userdata/ModioUserDataService.h"
#include <algorithm>
#include <memory>

#include <asio/yield.hpp>

class ServiceInitializationOp
{
	asio::coroutine CoroutineState;
	Modio::InitializeOptions InitParams;
	Modio::filesystem::path GlobalOverridePath;
	Modio::Detail::DynamicBuffer GlobalConfigFileReadBuffer;
	std::map<std::string, std::string> ConfigurationValues;

public:
	ServiceInitializationOp(Modio::InitializeOptions InitParams) : InitParams(InitParams) {}

	template<typename CoroType>
	void operator()(CoroType& self, std::error_code ec = {})
	{
		reenter(CoroutineState)
		{
			if (!Modio::Detail::SDKSessionData::Initialize(InitParams))
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Core,
											"mod.io SDK was already initialized!");
				self.complete(Modio::make_error_code(Modio::GenericError::SDKAlreadyInitialized));
				return;
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Core,
										"Initializing mod.io services");
			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().InitializeAsync(
				InitParams, std::move(self));
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				self.complete(ec);
				return;
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::File, "Initialized File Service");

			yield Modio::Detail::LoadGlobalConfigOverrideFileDataAsync(GlobalConfigFileReadBuffer, std::move(self));
			if (ec)
			{
				// do a warning here about not being able to load the global config ?
				ConfigurationValues = {};
			}
			else
			{
				using nlohmann::from_json;
				from_json(Modio::Detail::ToJson(GlobalConfigFileReadBuffer), ConfigurationValues);
				ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().ApplyGlobalConfigOverrides(
					ConfigurationValues);
				if (ec)
				{
					Modio::Detail::SDKSessionData::Deinitialize();
					self.complete(ec);
					return;
				}
			}

			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().InitializeAsync(
				std::move(self));
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				self.complete(ec);
				return;
			}
			ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().ApplyGlobalConfigOverrides(
				ConfigurationValues);
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				self.complete(ec);
				return;
			}
			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http, "Initialized Http Service");

			// Init user data service
			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().InitializeAsync(
				std::move(self));
			if (ec && ec != Modio::UserAuthError::StatusAuthTokenMissing)
			{
				Modio::Detail::SDKSessionData::Deinitialize();

				self.complete(ec);
				return;
			}
			ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().ApplyGlobalConfigOverrides(
				ConfigurationValues);
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				self.complete(ec);
				return;
			}
			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::User,
										"Initialized User Data service");
			if (Modio::Detail::SDKSessionData::GetUserModDirectoryOverride().has_value())
			{
				ConfigurationValues[Modio::Detail::Constants::JSONKeys::RootLocalStoragePath] =
					Modio::Detail::SDKSessionData::GetUserModDirectoryOverride().value().generic_u8string();
				ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().ApplyGlobalConfigOverrides(
					ConfigurationValues);
				if (ec)
				{
					Modio::Detail::SDKSessionData::Deinitialize();
					self.complete(ec);
					return;
				}
			}

			// This should be safe to do here *after* we've loaded the user data folder and potentially overriden the
			// mod collection based on a value in the user data file, because nothing else should be trying to
			// re-hydrate/reconstitute the mods in the user's collection until after we're done with the SDK being
			// initialized. We do have to make sure that QueryUserSubscriptions etc fail if we're still initializing,
			// just to be on the safe side
			yield Modio::Detail::LoadModCollectionFromStorageAsync(std::move(self));

			// Validates all user's mods marked as ModState::Installed.  Any mods that fail validation will have
			// ModState set to InstallationPending.  Will NOT return an error code even on validation failure to prevent
			// killing initialization.
			yield Modio::Detail::ValidateAllInstalledModsAsync(std::move(self));

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Core,
										"mod.io service initialization complete");

			Modio::Detail::SDKSessionData::ConfirmInitialize();

			self.complete({});
		}
	}
};
