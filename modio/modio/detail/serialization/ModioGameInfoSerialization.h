/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioGameInfo.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

#include "modio/detail/serialization/ModioGameCommunityOptionsSerialization.h"
#include "modio/detail/serialization/ModioGameMaturityOptionsSerialization.h"
#include "modio/detail/serialization/ModioGameMonetizationSerialization.h"

namespace Modio
{
	NLOHMANN_JSON_SERIALIZE_ENUM(
		Modio::ModfilePlatform,
		{{Modio::ModfilePlatform::Windows, Modio::Detail::Constants::PlatformNames::Windows},
		 {Modio::ModfilePlatform::Mac, Modio::Detail::Constants::PlatformNames::Mac},
		 {Modio::ModfilePlatform::Linux, Modio::Detail::Constants::PlatformNames::Linux},
		 {Modio::ModfilePlatform::Android, Modio::Detail::Constants::PlatformNames::Android},
		 {Modio::ModfilePlatform::iOS, Modio::Detail::Constants::PlatformNames::iOS},
		 {Modio::ModfilePlatform::XboxOne, Modio::Detail::Constants::PlatformNames::XboxOne},
		 {Modio::ModfilePlatform::XboxSeriesX, Modio::Detail::Constants::PlatformNames::XboxSeriesX},
		 {Modio::ModfilePlatform::PS4, Modio::Detail::Constants::PlatformNames::PS4},
		 {Modio::ModfilePlatform::PS5, Modio::Detail::Constants::PlatformNames::PS5},
		 {Modio::ModfilePlatform::Switch, Modio::Detail::Constants::PlatformNames::Switch},
		 {Modio::ModfilePlatform::Oculus, Modio::Detail::Constants::PlatformNames::Oculus},
		 {Modio::ModfilePlatform::Source, Modio::Detail::Constants::PlatformNames::Source},
		 {Modio::ModfilePlatform::WindowsServer, Modio::Detail::Constants::PlatformNames::WindowsServer},
		 {Modio::ModfilePlatform::LinuxServer, Modio::Detail::Constants::PlatformNames::LinuxServer}});

	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::HeaderImage& HeaderImage)
	{
		Detail::ParseSafe(Json, HeaderImage.Filename, "filename");
		Detail::ParseSafe(Json, HeaderImage.Original, "original");
	}

	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::OtherUrl& OtherUrl)
	{
		Detail::ParseSafe(Json, OtherUrl.Label, "label");
		Detail::ParseSafe(Json, OtherUrl.Url, "url");
	}

	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::Theme& Theme)
	{
		Detail::ParseSafe(Json, Theme.Primary, "primary");
		Detail::ParseSafe(Json, Theme.Dark, "dark");
		Detail::ParseSafe(Json, Theme.Light, "light");
		Detail::ParseSafe(Json, Theme.Success, "success");
		Detail::ParseSafe(Json, Theme.Warning, "warning");
		Detail::ParseSafe(Json, Theme.Danger, "danger");
	}

	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::GameInfo& GameInfo)
	{
		Detail::ParseSafe(Json, GameInfo.GameID, "id");
		Detail::ParseSafe(Json, GameInfo.DateAdded, "date_added");
		Detail::ParseSafe(Json, GameInfo.DateUpdated, "date_updated");
		Detail::ParseSafe(Json, GameInfo.DateLive, "date_live");
		Detail::ParseSafe(Json, GameInfo.UgcName, "ugc_name");
		Detail::ParseSafe(Json, GameInfo.Name, "name");
		Detail::ParseSafe(Json, GameInfo.Summary, "summary");
		Detail::ParseSafe(Json, GameInfo.Instructions, "instructions");
		Detail::ParseSafe(Json, GameInfo.InstructionsUrl, "instructions_url");
		Detail::ParseSafe(Json, GameInfo.ProfileUrl, "profile_url");
		Detail::ParseSafe(Json, GameInfo.Logo, "logo");
		Detail::ParseSafe(Json, GameInfo.Icon, "icon");
		Detail::ParseSafe(Json, GameInfo.HeaderImage, "header");
		Detail::ParseSafe(Json, GameInfo.Stats, "stats");
		Detail::ParseSafe(Json, GameInfo.Theme, "theme");
		Detail::ParseSafe(Json, GameInfo.OtherUrls, "other_urls");
		Detail::ParseSafe(Json, GameInfo.CommunityOptions, "community_options");
		Detail::ParseSafe(Json, GameInfo.GameMonetizationOptions, "monetization_options");
		Detail::ParseSafe(Json, GameInfo.MaturityOptions, "maturity_options");
		Detail::ParseSafe(Json, GameInfo.VirtualTokenName, "token_name");
		Detail::ParseSafe(Json, GameInfo.TagOptions, "tag_options");
		Detail::ParseSafe(Json, GameInfo.CloudCookingStatus, "cloud_cooking_status");

		nlohmann::json PlatformsJson;
		if (Detail::GetSubobjectSafe(Json, "platforms", PlatformsJson))
		{
			for (nlohmann::json& Entry : PlatformsJson)
			{
				GamePlatform GamePlatformDetails = {};
				Detail::ParseSafe(Entry, GamePlatformDetails.Locked, "locked");
				Detail::ParseSafe(Entry, GamePlatformDetails.Moderated, "moderated");
				Detail::ParseSafe(Entry, GamePlatformDetails.Platform, "platform");

				GameInfo.PlatformSupport.push_back(GamePlatformDetails);
			}
		}
	}
} // namespace Modio