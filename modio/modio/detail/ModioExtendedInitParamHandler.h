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

#include "modio/core/ModioInitializeOptions.h"
#include "modio/detail/ModioSDKSessionData.h"
#include <chrono>

namespace Modio
{
	namespace Detail
	{
		struct InternalConstants
		{
			static constexpr const char* ServerURL = "ServerURL";
			static constexpr const char* OAuthToken = "OAuthToken";
			static constexpr const char* PlatformOverride = "PlatformOverride";
		};

		/// @brief Struct for handling internal-only initialization options
		struct ExtendedInitParamHandler
		{
			// We probably want to move this to a centralized location and use a single implementation everywhere
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

			/// @brief Invoked after SDKSessionData is initialized but prior to service initialization
			/// @param InitParams Mutable set of initialization options to process
			static void PostSessionDataInit(Modio::InitializeOptions& InitParams)
			{
				Modio::Optional<std::string> ServerURLOverride =
					GetExtendedParameterValue(InitParams, Modio::Detail::InternalConstants::ServerURL);
				if (ServerURLOverride.has_value())
				{
					Modio::Detail::SDKSessionData::SetEnvironmentOverrideUrl(*ServerURLOverride);
				}

				Modio::Optional<std::string> PlatformOverride =
					GetExtendedParameterValue(InitParams, Modio::Detail::InternalConstants::PlatformOverride);

				if (PlatformOverride.has_value())
				{
					Modio::Detail::SDKSessionData::SetPlatformOverride(*PlatformOverride);
				}
			};

			/// @brief Invoked after UserDataService initialization
			/// @param InitParams Mutable set of initialization options to process
			static void PostUserDataServiceInit(Modio::InitializeOptions& InitParams)
			{
				Modio::Optional<std::string> OAuthTokenOverride =
					GetExtendedParameterValue(InitParams, Modio::Detail::InternalConstants::OAuthToken);
				if (OAuthTokenOverride.has_value())
				{
					Modio::Detail::SDKSessionData::InitializeForUser(Modio::User{}, Modio::Detail::OAuthToken(*OAuthTokenOverride, (std::chrono::system_clock::now().time_since_epoch() + std::chrono::hours(8760)).count()));
				}
			}
		};
	} // namespace Detail
} // namespace Modio