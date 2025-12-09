/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/core/entities/ModioModInfoList.h"

namespace Modio
{
	/// @docpublic
	/// @brief Contains download stats and ratings for a mod collection
	struct ModCollectionStats
	{
		/// @brief Number of mod collection downloads today.
		std::int64_t DownloadsToday = 0;
		/// @brief Number of total mod collection downloads.
		std::int64_t DownloadsTotal = 0;
		/// @brief Number of unique mod collection downloads.
		std::int64_t DownloadsUnique = 0;
		/// @brief Number of times this mod collection has been rated.
		std::int64_t RatingTotal30Days = 0;
		/// @brief Number of positive ratings over 30 days.
		std::int64_t RatingPositive30Days = 0;
		/// @brief Number of negative ratings over 30 days.
		std::int64_t RatingNegative30Days = 0;
		/// @brief Number of times this mod collection has been rated.
		std::int64_t RatingTotal = 0;
		/// @brief Number of positive ratings.
		std::int64_t RatingPositive = 0;
		/// @brief Number of negative ratings.
		std::int64_t RatingNegative = 0;
		/// @brief Number of comments.
		std::int64_t NumberOfComments = 0;
		/// @brief Number of followers.
		std::int64_t NumberOfFollowers = 0;
		/// @brief Number of mods in the collection.
		std::int64_t NumberOfMods = 0;
	};

	/// @brief Information about a given mod collection
	struct ModCollectionInfo
	{
		/// @brief The unique ID of the collection.
		Modio::ModCollectionID Id {};
		/// @brief The unique ID of the game the collection belongs to.
		Modio::GameID GameId {};
		/// @brief the game name
		std::string GameName {};
		/// @brief The current ModStatus on the server: Accepted, NotAccepted, or Deleted.
		Modio::ModServerSideStatus CollectionStatus = Modio::ModServerSideStatus::NotAccepted;
		/// @brief The mod collection's visibility status. Defaults to Public (1)
		Modio::ObjectVisibility Visibility = Modio::ObjectVisibility::Public;
		/// @brief Information on the user who submitted the collection
		Modio::User ProfileSubmittedBy {};
		/// @brief Chosen category of the collection
		std::string Category {};
		/// @brief Unix timestamp of the date the mod was registered
		std::int64_t ProfileDateAdded = 0;
		/// @brief Unix timestamp of the date the mod was updated
		std::int64_t ProfileDateUpdated = 0;
		/// @brief Unix timestamp of the date the mod was marked live
		std::int64_t ProfileDateLive = 0;
		/// @brief boolean to indicate whether the collection contains hidden mods
		bool Incomplete = false;
		/// @brief Object representing a mod.io user profile
		///	* Maturity options flagged by the mod developer, this is only relevant if the parent game allows collections
		///to
		///	* be labeled as mature. The value of this field will default to None unless the parent game allows
		/// * to flag mature content.
		Modio::ProfileMaturity ProfileMaturityOption {};
		/// @brief Size of collection in bytes 
		std::uint64_t FileSize = 0;
		/// @brief Size of the uncompressed collection in bytes
		std::uint64_t FileSizeUncompressed = 0;
		/// @brief Supported platform for this collection
		std::vector<Modio::ModfilePlatform> Platforms {};
		/// @brief Tags this mod collection has set
		std::vector<std::string> Tags {};
		/// @brief Stats and rating information for the mod collection.
		Modio::ModCollectionStats Stats {};
		/// @brief Media data related to the mod collection logo
		Modio::Detail::Logo Logo {};
		/// @brief Name of the mod collection
		std::string ProfileName {};
		/// @brief Name of the mod collection
		std::string ProfileNameId {};
		/// @brief Summary of the mod collection
		std::string ProfileSummary {};
		/// @brief Detailed description in HTML format
		std::string ProfileDescription {};
		/// @brief Detailed description in plaintext format
		std::string ProfileDescriptionPlaintext {};

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModCollectionInfo& ModCollection);
	};

	/// @docpublic
	/// @brief Class representing a list of mods that may be a page from a larger set of results
	class ModCollectionInfoList : public Modio::PagedResult, public List<std::vector, Modio::ModCollectionInfo>
	{
	public:
		/// @docpublic
		/// @brief Insert ModCollectionInfoList to the end of this list
		void Append(const ModCollectionInfoList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModCollectionInfo to the end of this list
		void Append(const ModCollectionInfo& ModInfoData)
		{
			InternalList.push_back(ModInfoData);
		}

		friend inline void from_json(const nlohmann::json& Json, Modio::ModCollectionInfoList& OutModInfoList);
	};


	/// @docpublic
	/// @brief Class specifying the parameters to submit when creating a mod collection
	struct CreateModCollectionParams
	{
		/// @docpublic
		/// @brief Path to the logo for the mod collection . Must point to a valid file on disk or the mod will not be
		/// created on the server.
		std::string PathToLogoFile {};

		/// @docpublic
		/// @brief The name of the mod collection
		std::string Name {};

		/// @docpublic
		/// @brief A brief summary of what the mod collection is
		std::string Summary {};

		/// @docpublic
		/// @brief A category for the mod collection
		std::string Category = {};

		/// @docpublic
		/// @brief Optional override for the mod collection's visibility status. Defaults to Public (1)
		Modio::ObjectVisibility Visibility = Modio::ObjectVisibility::Public;

		/// @docpublic
		/// @brief Optional override for the list of contained mods
		Modio::Optional<std::vector<Modio::ModID>> Mods {};

		/// @docpublic
		/// @brief Optional override for the name 'slug' in the mod collection 's URL
		Modio::Optional<std::string> NamePath {};

		/// @docpublic
		/// @brief Optional long description of the mod collection
		Modio::Optional<std::string> Description {};

		/// @docpublic
		/// @brief Optional list of mod collection tags. All tags must be supported by the parent game to be applied
		Modio::Optional<std::vector<std::string>> Tags {};
	};

	/// @docpublic
	/// @brief Class specifying fields to update for a mod collection
	struct EditModCollectionParams
	{
		/// @docpublic
		/// @brief Optional path to a new logo image
		Modio::Optional<std::string> LogoPath {};

		/// @docpublic
		/// @brief The name of the mod collection
		Modio::Optional<std::string> Name {};

		/// @docpublic
		/// @brief A brief summary of what the mod collection is
		Modio::Optional<std::string> Summary {};

		/// @docpublic
		/// @brief A category for the mod collection
		Modio::Optional<std::string> Category {};

		/// @docpublic
		/// @brief Optional override for the mod collection's visibility status. Defaults to Public (1)
		Modio::Optional<Modio::ObjectVisibility> Visibility {};

		/// @docpublic
		/// @brief Optional override for the list of contained mods
		Modio::Optional<std::vector<Modio::ModID>> Mods {};

		/// @docpublic
		/// @brief Optional override for the name 'slug' in the mod collection's URL
		Modio::Optional<std::string> NamePath {};

		/// @docpublic
		/// @brief Optional long description of the mod collection
		Modio::Optional<std::string> Description {};

		/// @docpublic
		/// @brief Optional tags vector for this mod collection
		Modio::Optional<std::vector<std::string>> Tags {};
	};

} // namespace Modio
