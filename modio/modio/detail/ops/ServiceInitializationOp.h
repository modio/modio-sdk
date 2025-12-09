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
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/LoadGlobalConfigOverrideFileDataOp.h"
#include "modio/detail/ops/LoadModCollectionFromStorage.h"
#include "modio/detail/ops/ValidateAllInstalledModsOp.h"
#include "modio/file/ModioFile.h"
#include "modio/file/ModioFileService.h"
#include "modio/http/ModioHttpService.h"
#include "modio/timer/ModioTimerService.h"
#include "modio/userdata/ModioUserDataService.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

#ifdef MODIO_PROCESS_INTERNAL_INITPARAMS
	#include "modio/detail/ModioExtendedInitParamHandler.h"
#else
	#include "modio/detail/ModioExtInitParamStub.h"
#endif

#include "modio/core/ModioMetricsService.h"

#include <asio/yield.hpp>

class ServiceInitializationOp
{
	ModioAsio::coroutine CoroutineState {};
	Modio::InitializeOptions InitParams {};
	Modio::filesystem::path GlobalOverridePath {};
	Modio::Detail::DynamicBuffer GlobalConfigFileReadBuffer {};
	std::map<std::string, std::string> ConfigurationValues {};

public:
	ServiceInitializationOp(Modio::InitializeOptions InitParams) : InitParams(InitParams) {}

	static Modio::Optional<std::string> GetExtendedParameterValue(Modio::InitializeOptions& InitParams,
																  std::string ParamName)
	{
		auto ParamIterator = InitParams.ExtendedParameters.find(ParamName);
		if (ParamIterator == InitParams.ExtendedParameters.end())
		{
			return {};
		}
		else
		{
			return ParamIterator->second;
		}
	}

	template<typename CoroType>
	void operator()(CoroType& Self, std::error_code ec = {})
	{
		MODIO_PROFILE_SCOPE(ServiceInitialization);

		Modio::Optional<std::string> PlatformOverride = GetExtendedParameterValue(InitParams, "PlatformOverride");
		Modio::Optional<std::string> PendingOnlyResults = GetExtendedParameterValue(InitParams, "PendingOnlyResults");
		Modio::Optional<std::string> InstallationDirectory =
			GetExtendedParameterValue(InitParams, "IgnoreModInstallationDirectoryOverride");
		Modio::Optional<std::string> PlatformEnvironment = GetExtendedParameterValue(InitParams, "PlatformEnvironment");
		Modio::Optional<std::string> MetricsSecretKey = GetExtendedParameterValue(InitParams, "MetricsSecretKey");
		Modio::Optional<std::string> ModStorageQuotaMB = GetExtendedParameterValue(InitParams, "ModStorageQuotaMB");
		Modio::Optional<std::string> CacheStorageQuotaMB = GetExtendedParameterValue(InitParams, "CacheStorageQuotaMB");

		reenter(CoroutineState)
		{
			if (!Modio::Detail::SDKSessionData::Initialize(InitParams))
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Core,
											"mod.io SDK was already initialized!");
				Self.complete(Modio::make_error_code(Modio::GenericError::SDKAlreadyInitialized));
				return;
			}

			if (PlatformOverride.has_value())
			{
				Modio::Detail::SDKSessionData::SetPlatformOverride(*PlatformOverride);
			}

			if (PlatformEnvironment.has_value())
			{
				Modio::Detail::SDKSessionData::SetPlatformEnvironment(*PlatformEnvironment);
			}

			if (MetricsSecretKey.has_value())
			{
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().InitMetricsSession(
					*MetricsSecretKey);
			}

			if (PendingOnlyResults.has_value()) 
			{
				Modio::Detail::SDKSessionData::SetPlatformStatusFilter(*PendingOnlyResults);
			}

			Modio::Detail::ExtendedInitParamHandler::PostSessionDataInit(InitParams);

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Core,
										"Initializing mod.io services");
			yield ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));

			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::TimerService>().InitializeAsync(
				std::move(Self));

			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().InitializeAsync(
				InitParams, std::move(Self));
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				Self.complete(ec);
				return;
			}

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::File, "Initialized File Service");

			{
				// 25MB minimum storage
				// Note that the image cache checks for at least 8MB of free space before downloading an image, so the
				// effective minimum image cache storage is 17MB
				Modio::FileSize MinimumStorageQuota = Modio::FileSize(25 * 1024 * 1024);
				if (ModStorageQuotaMB.has_value())
				{
					// ensure numeric input
					for (char c : ModStorageQuotaMB.value())
					{
						if (!std::isdigit(c))
						{
							Modio::Detail::SDKSessionData::Deinitialize();
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Error, Modio::LogCategory::File,
								"Extended parameter ModStorageQuotaMB must be a positive integer");
							Self.complete(Modio::make_error_code(Modio::GenericError::BadParameter));
							return;
						}
					}
					// convert to Modio::FileSize in bytes
					Modio::FileSize ModStorageQuota = Modio::FileSize(
						static_cast<std::uintmax_t>(std::stoull(ModStorageQuotaMB.value())) * 1024 * 1024);
					if (ModStorageQuota < MinimumStorageQuota)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::File,
													"Cannot set mod storage quota lower than {} bytes",
													MinimumStorageQuota);
						ModStorageQuota = MinimumStorageQuota;
					}
					Modio::Detail::SDKSessionData::SetModStorageQuota(ModStorageQuota);
				}
				if (CacheStorageQuotaMB.has_value())
				{
					// ensure numeric input
					for (char c : CacheStorageQuotaMB.value())
					{
						if (!std::isdigit(c))
						{
							Modio::Detail::SDKSessionData::Deinitialize();
							Modio::Detail::Logger().Log(
								Modio::LogLevel::Error, Modio::LogCategory::File,
								"Extended parameter CacheStorageQuotaMB must be a positive integer");
							Self.complete(Modio::make_error_code(Modio::GenericError::BadParameter));
							return;
						}
					}
					// convert to Modio::FileSize in bytes
					Modio::FileSize CacheStorageQuota = Modio::FileSize(
						static_cast<std::uintmax_t>(std::stoull(CacheStorageQuotaMB.value())) * 1024 * 1024);
					if (CacheStorageQuota < MinimumStorageQuota)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::File,
													"Cannot set cache storage quota lower than {} bytes",
													MinimumStorageQuota);
						CacheStorageQuota = MinimumStorageQuota;
					}
					ec = Modio::Detail::SDKSessionData::SetCacheStorageQuota(CacheStorageQuota);
					if (ec)
					{
						Modio::Detail::SDKSessionData::Deinitialize();
						Self.complete(ec);
						return;
					}
				}
			}

			// Override root storage location unless extended param IgnoreModInstallationDirectoryOverride is set
			if (!InstallationDirectory.has_value())
			{
				yield Modio::Detail::LoadGlobalConfigOverrideFileDataAsync(GlobalConfigFileReadBuffer, std::move(Self));

				if (ec)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::File,
												"Failed to load global configuration overrides: " + ec.message());
					ConfigurationValues = {};
				}
				else
				{
					using nlohmann::from_json;
					from_json(Modio::Detail::ToJson(GlobalConfigFileReadBuffer), ConfigurationValues);
					ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
							 .ApplyGlobalConfigOverrides(ConfigurationValues);
					if (ec)
					{
						Modio::Detail::SDKSessionData::Deinitialize();
						Self.complete(ec);
						return;
					}
				}
			}
			else
			{
				Modio::Detail::Logger().Log(
					Modio::LogLevel::Info, Modio::LogCategory::File,
					"Ignoring RootLocalStoragePath override. Mods will be installed to the default directory.");
			}

			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().InitializeAsync(
				std::move(Self));
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				Self.complete(ec);
				return;
			}
			ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().ApplyGlobalConfigOverrides(
				InitParams.ExtendedParameters);
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				Self.complete(ec);
				return;
			}
			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http, "Initialized Http Service");

			// Init user data service
			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().InitializeAsync(
				std::move(Self));
			if (ec && ec != Modio::UserAuthError::StatusAuthTokenMissing)
			{
				Modio::Detail::SDKSessionData::Deinitialize();

				Self.complete(ec);
				return;
			}
			ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().ApplyGlobalConfigOverrides(
				ConfigurationValues);
			if (ec)
			{
				Modio::Detail::SDKSessionData::Deinitialize();
				Self.complete(ec);
				return;
			}

			Modio::Detail::ExtendedInitParamHandler::PostUserDataServiceInit(InitParams);

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::User,
										"Initialized User Data service");
			if (Modio::Detail::SDKSessionData::GetUserModDirectoryOverride().has_value())
			{
				ConfigurationValues[Modio::Detail::Constants::JSONKeys::RootLocalStoragePath] =
					Modio::ToModioString(Modio::Detail::SDKSessionData::GetUserModDirectoryOverride().value().generic_u8string());
				ec = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().ApplyGlobalConfigOverrides(
					ConfigurationValues);
				if (ec)
				{
					Modio::Detail::SDKSessionData::Deinitialize();
					Self.complete(ec);
					return;
				}
			}

			// This should be safe to do here *after* we've loaded the user data folder and potentially overriden the
			// mod collection based on a value in the user data file, because nothing else should be trying to
			// re-hydrate/reconstitute the mods in the user's collection until after we're done with the SDK being
			// initialized. We do have to make sure that QueryUserSubscriptions etc fail if we're still initializing,
			// just to be on the safe side
			yield Modio::Detail::LoadModCollectionFromStorageAsync(std::move(Self));

			// Validates all user's mods marked as ModState::Installed.  Any mods that fail validation will have
			// ModState set to InstallationPending.  Will NOT return an error code even on validation failure to prevent
			// killing initialization.
			yield Modio::Detail::ValidateAllInstalledModsAsync(std::move(Self));

			yield Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().DeleteFolderAsync(
				Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
					.GetTempModRootInstallationPath(),
				std::move(Self));

			yield ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Self));

			Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Core,
										"mod.io service initialization complete");

			Modio::Detail::SDKSessionData::ConfirmInitialize();

			Self.complete({});
		}
	}
};

template<typename InitDoneCallback>
auto InitializeServiceAsync(Modio::InitializeOptions InitOptions, InitDoneCallback&& OnInitComplete)
{
	return ModioAsio::async_compose<InitDoneCallback, void(Modio::ErrorCode)>(
		ServiceInitializationOp(InitOptions), OnInitComplete,
		Modio::Detail::Services::GetGlobalContext().get_executor());
};
