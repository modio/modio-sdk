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
#include "ModioGeneratedVariables.h"
#include "modio/detail/ModioDefines.h"

#include "modio/core/entities/ModioFileMetadata.h"
#include "modio/core/entities/ModioModStats.h"
#include "modio/core/entities/ModioURLList.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/entities/ModioGalleryList.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Modio
{
	enum class MaturityOption : std::uint8_t
	{
		None = 0,
		Alcohol = 1,
		Drugs = 2,
		Violence = 4,
		Explicit = 8
	};

	// @todo: If ModTag and Metadata is used somewhere else than in ModInfo, then migrate them to their own headers
	struct ModTag
	{
		std::string Tag;
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModTag& ModTag);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::ModTag& Tag);


	// Migrate: std::vector<Modio::ModTag> Tags to a custom class that encapsulates std::map helpers to fetch Values as
	// something like this: Metadata.Get<int32>( "OptimizedFor" );
	struct Metadata
	{
		std::string Key;
		std::string Value;
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::Metadata& Metadata);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::Metadata& Metadata);

	/// @docpublic
	/// @brief Full mod profile including current release information, media links, and stats
	struct ModInfo
	{
		/// @brief Unique Mod ID
		Modio::ModID ModId;

		/// @brief Name of the mod
		std::string ProfileName;
		/// @brief Summary of the mod
		std::string ProfileSummary;
		/// @brief Detailed description in HTML format
		std::string ProfileDescription;
		/// @brief Detailed description in plaintext
		std::string ProfileDescriptionPlaintext;
		/// @brief URL to the mod profile
		std::string ProfileURL;
		/// @brief Information on the user who submitted the mod
		Modio::User ProfileSubmittedBy;
		/// @brief Unix timestamp of the date the mod was registered
		std::int64_t ProfileDateAdded;
		/// @brief Unix timestamp of the date the mod was updated
		std::int64_t ProfileDateUpdated;
		/// @brief Unix timestamp of the date the mod was marked live
		std::int64_t ProfileDateLive;
		/// @brief Flags for maturity options
		///	* Maturity options flagged by the mod developer, this is only relevant if the parent game allows mods to
		///	* be labeled as mature.
		///	*
		///	* 0 = None set (default)
		///	* 1 = Alcohol
		///	* 2 = Drugs
		///	* 4 = Violence
		///	* 8 = Explicit
		std::uint8_t ProfileMaturityOption;

		std::string MetadataBlob;
		/// @brief Information about the mod's most recent public release
		Modio::FileMetadata FileInfo;
		/// @brief Arbitrary key-value metadata set for this mod
		std::vector<Modio::Metadata> MetadataKvp;
		/// @brief Tags this mod has set
		std::vector<Modio::ModTag> Tags;

		/// @brief Number of images in the mod's media gallery
		std::size_t NumGalleryImages;

		/// @brief List of youtube links provided by the creator of the mod
		Modio::YoutubeURLList YoutubeURLs;
		/// @brief List of sketchfab links provided by the creator of the mod
		Modio::SketchfabURLList SketchfabURLs;
		/// @brief Stats and rating information for the mod
		Modio::ModStats Stats;
		
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModInfo& ModInfo);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::ModInfo& Info);
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModInfo.ipp"
#endif
