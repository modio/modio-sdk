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
#include "modio/core/entities/ModioProfileMaturity.h"
#include "modio/core/entities/ModioURLList.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/entities/ModioGalleryList.h"
#include "modio/detail/entities/ModioLogo.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Modio
{
	// @todo: If ModTag and Metadata is used somewhere else than in ModInfo, then migrate them to their own headers
	struct ModTag
	{
		std::string Tag;

		friend bool operator==(const Modio::ModTag& A, const Modio::ModTag& B)
		{
			return (A.Tag == B.Tag);
		}
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModTag& ModTag);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::ModTag& Tag);

	// Migrate: std::vector<Modio::ModTag> Tags to a custom class that encapsulates std::map helpers to fetch Values as
	// something like this: Metadata.Get<int32>( "OptimizedFor" );
	struct Metadata
	{
		std::string Key;
		std::string Value;

		friend bool operator==(const Modio::Metadata& A, const Modio::Metadata& B)
		{
			return (A.Key == B.Key && A.Value == B.Value);
		}
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::Metadata& Metadata);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::Metadata& Metadata);

	enum class ModServerSideStatus : std::uint8_t
	{
		NotAccepted = 0,
		Accepted = 1,
		Deleted = 3
	};

	/// @docpublic
	/// @brief Full mod profile including current release information, media links, and stats
	struct ModInfo
	{
		/// @brief Unique Mod ID
		Modio::ModID ModId = Modio::ModID(0);
		/// @brief Name of the mod
		std::string ProfileName = "";
		/// @brief Summary of the mod
		std::string ProfileSummary = "";
		/// @brief Detailed description in HTML format
		std::string ProfileDescription = "";
		/// @brief Detailed description in plaintext
		std::string ProfileDescriptionPlaintext = "";
		/// @brief URL to the mod profile
		std::string ProfileURL = "";
		/// @brief Information on the user who submitted the mod
		Modio::User ProfileSubmittedBy;
		/// @brief Unix timestamp of the date the mod was registered
		std::int64_t ProfileDateAdded = 0;
		/// @brief Unix timestamp of the date the mod was updated
		std::int64_t ProfileDateUpdated = 0;
		/// @brief Unix timestamp of the date the mod was marked live
		std::int64_t ProfileDateLive = 0;
		/// @brief Object representing a mod.io user profile
		///	* Maturity options flagged by the mod developer, this is only relevant if the parent game allows mods to
		///	* be labeled as mature. The value of this field will default to None unless the parent game allows
		/// * to flag mature content.
		Modio::ProfileMaturity ProfileMaturityOption;
		/// @brief Metadata stored by the game developer.
		std::string MetadataBlob = "";
		/// @brief Information about the mod's most recent public release
		Modio::Optional<Modio::FileMetadata> FileInfo = {};
		/// @brief Arbitrary key-value metadata set for this mod
		std::vector<Modio::Metadata> MetadataKvp;
		/// @brief Tags this mod has set
		std::vector<Modio::ModTag> Tags;
		/// @brief Number of images in the mod's media gallery
		std::size_t NumGalleryImages = 0;
		/// @brief List of images in the mod's media gallery
		Modio::GalleryList GalleryImages = {};
		/// @brief List of youtube links provided by the creator of the mod
		Modio::YoutubeURLList YoutubeURLs;
		/// @brief List of sketchfab links provided by the creator of the mod
		Modio::SketchfabURLList SketchfabURLs;
		/// @brief Stats and rating information for the mod
		Modio::ModStats Stats;
		/// @brief Media data related to the mod logo
		Modio::Detail::Logo ModLogo;
		/// @brief The current ModInfo version. This property is updated when changes to the class happen.
		std::string Version = "1.0";
		/// @brief The current ModStatus on the server: Accepted, NotAccepted, or Deleted.
		Modio::ModServerSideStatus ModStatus = Modio::ModServerSideStatus::NotAccepted;

		friend bool operator==(const Modio::ModInfo& A, const Modio::ModInfo& B)
		{
			if ((A.NumGalleryImages == B.NumGalleryImages) && (A.SketchfabURLs == B.SketchfabURLs) &&
				(A.ProfileMaturityOption == B.ProfileMaturityOption) && (A.GalleryImages == B.GalleryImages) &&
				(A.ModId == B.ModId) && (A.ProfileName == B.ProfileName) && (A.ProfileSummary == B.ProfileSummary) &&
				(A.ProfileDescription == B.ProfileDescription) &&
				(A.ProfileDescriptionPlaintext == B.ProfileDescriptionPlaintext) && (A.ProfileURL == B.ProfileURL) &&
				(A.ProfileSubmittedBy == B.ProfileSubmittedBy) && (A.ProfileDateAdded == B.ProfileDateAdded) &&
				(A.ProfileDateUpdated == B.ProfileDateUpdated) && (A.ProfileDateLive == B.ProfileDateLive) &&
				(A.MetadataBlob == B.MetadataBlob) && (A.MetadataKvp == B.MetadataKvp) && (A.Tags == B.Tags) &&
				(A.YoutubeURLs == B.YoutubeURLs) && (A.Stats == B.Stats) && (A.ModLogo == B.ModLogo) &&
				(A.ModStatus == B.ModStatus))
			{
				if (A.FileInfo.has_value() && B.FileInfo.has_value())
				{
					if ((A.FileInfo.value() == B.FileInfo.value()))
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else if ((A.FileInfo.has_value() && !B.FileInfo.has_value()) ||
						 (!A.FileInfo.has_value() && B.FileInfo.has_value()))
				{
					return false;
				}
				else
				{
					return true;
				}
			}
			else
			{
				return false;
			}
		}
	};

	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModInfo& ModInfo);
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::ModInfo& Info);
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModInfo.ipp"
#endif
