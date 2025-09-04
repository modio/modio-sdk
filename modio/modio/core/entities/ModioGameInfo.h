/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioGameStats.h"
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/entities/ModioIcon.h"
#include "modio/detail/entities/ModioLogo.h"
#include "modio/core/entities/ModioGameCommunityOptions.h"
#include "modio/core/entities/ModioGameMaturityOptions.h"
#include "modio/core/entities/ModioGameMonetization.h"
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Details about a platform that this game supports
	struct GamePlatform
	{
		/// @brief Platform that the game supports
		Modio::ModfilePlatform Platform {};

		/// @brief Whether or not this platform is Locked from having files submitted to it
		bool Locked = false;

		/// @brief Whether or not files submitted for this platform are required to go through moderation
		bool Moderated = false;
	};

	/// @docpublic
	/// @brief Contains media URLs to the preview header image for the game
	struct HeaderImage
	{
		std::string Filename {};
		std::string Original {};

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::HeaderImage& HeaderImage);
	};

	/// @docpublic
	/// @brief Creator defined URLs to share
	struct OtherUrl
	{
		std::string Label;
		std::string Url;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::OtherUrl& OtherUrl);
	};

	/// @docpublic
	/// @brief Theme color values for the game
	struct Theme
	{
		std::string Primary;
		std::string Dark;
		std::string Light;
		std::string Success;
		std::string Warning;
		std::string Danger;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Theme& Theme);
	};

	/// @docpublic
	/// @brief Details about a game
	struct GameInfo
	{
		/// @brief Unique Game ID
		Modio::GameID GameID = Modio::GameID::InvalidGameID();
		/// @brief Unix timestamp of date game was registered
		std::int64_t DateAdded = 0;
		/// @brief Unix timestamp of date game was updated
		std::int64_t DateUpdated = 0;
		/// @brief Unix timestamp of date game was set live
		std::int64_t DateLive = 0;
		/// @brief Word used to describe user-generated content (mods, items, addons etc)
		std::string UgcName;
		/// @brief Contains media URLs to the icon for the game
		Modio::Detail::Icon Icon {};
		/// @brief Contains media URLs to the logo for the game
		Modio::Detail::Logo Logo {};
		/// @brief Contains media URLs to the preview header image for the game
		Modio::HeaderImage HeaderImage {};
		/// @brief Name of the game
		std::string Name;
		/// @brief Summary of the game's mod support
		std::string Summary;
		/// @brief A guide about creating and uploading mods for this game to mod.io
		std::string Instructions;
		/// @brief Link to a mod.io guide, modding wiki, or a page where modders can learn how to make and submit mods
		/// to this game's profile
		std::string InstructionsUrl;
		/// @brief URL to the game
		std::string ProfileUrl;
		/// @brief Theme color values for the game
		Modio::Theme Theme {};
		/// @brief Numerous aggregate stats for the game
		Modio::GameStats Stats {};
		/// @brief Creator defined URLs to share
		std::vector<Modio::OtherUrl> OtherUrls {};
		/// @brief Platforms supported by this title
		std::vector<Modio::GamePlatform> PlatformSupport {};
		/// @brief Community options for the game
		Modio::GameCommunityOptionsFlags	CommunityOptions;
		/// @brief Monetization options for the game
		Modio::GameMonetization		GameMonetizationOptions;
		/// @brief Maturity options for the game
		Modio::GameMaturityOptionsFlags	MaturityOptions;
		/// @brief Name of the Virtual Tokens for this game
		std::string VirtualTokenName;
		/// @brief Tags for this game
		std::vector<Modio::ModTagInfo> TagOptions {};

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::GameInfo& GameInfo);
	};
} // namespace Modio
