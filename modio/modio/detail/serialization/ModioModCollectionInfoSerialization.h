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

#include "modio/detail/serialization/ModioGameInfoSerialization.h"
#include "modio/detail/serialization/ModioModInfoSerialization.h"
#include "modio/core/entities/ModioModCollection.h"

namespace Modio
{
	/// @docnone
	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModCollectionStats& ModStats) 
	{
		Detail::ParseSafe(Json, ModStats.DownloadsToday, "downloads_today");
		Detail::ParseSafe(Json, ModStats.DownloadsTotal, "downloads_total");
		Detail::ParseSafe(Json, ModStats.DownloadsUnique, "downloads_unique");
		Detail::ParseSafe(Json, ModStats.RatingTotal30Days, "ratings_total_30days");
		Detail::ParseSafe(Json, ModStats.RatingPositive30Days, "ratings_positive_30days");
		Detail::ParseSafe(Json, ModStats.RatingNegative30Days, "ratings_negative_30days");
		Detail::ParseSafe(Json, ModStats.RatingTotal, "ratings_total");
		Detail::ParseSafe(Json, ModStats.RatingPositive, "ratings_positive");
		Detail::ParseSafe(Json, ModStats.RatingNegative, "ratings_negative");
		Detail::ParseSafe(Json, ModStats.NumberOfComments, "comments_total");
		Detail::ParseSafe(Json, ModStats.NumberOfFollowers, "followers_total");
		Detail::ParseSafe(Json, ModStats.NumberOfMods, "mods_total");

	}

	/// @docnone
	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::ModCollectionInfo& ModCollection) 
	{
		Detail::ParseSafe(Json, ModCollection.Id, "id");
		Detail::ParseSafe(Json, ModCollection.GameId, "game_id");
		Detail::ParseSafe(Json, ModCollection.GameName, "game_name");
		Detail::ParseSafe(Json, ModCollection.CollectionStatus, "status");
		Detail::ParseSafe(Json, ModCollection.Visibility, "visible");
		Detail::ParseSafe(Json, ModCollection.ProfileSubmittedBy, "submitted_by");
		Detail::ParseSafe(Json, ModCollection.Category, "category");
		Detail::ParseSafe(Json, ModCollection.ProfileDateAdded, "date_added");
		Detail::ParseSafe(Json, ModCollection.ProfileDateUpdated, "date_updated");
		Detail::ParseSafe(Json, ModCollection.ProfileDateLive, "date_live");
		Detail::ParseSafe(Json, ModCollection.Incomplete, "incomplete");
		Detail::ParseSafe(Json, ModCollection.ProfileMaturityOption, "maturity_option");
		Detail::ParseSafe(Json, ModCollection.FileSize, "filesize");
		Detail::ParseSafe(Json, ModCollection.FileSizeUncompressed, "filesize_uncompressed");
		Detail::ParseSafe(Json, ModCollection.Platforms, "platforms");
		Detail::ParseSafe(Json, ModCollection.Tags, "tags");
		Detail::ParseSafe(Json, ModCollection.Stats, "stats");
		Detail::ParseSafe(Json, ModCollection.Logo, "logo");
		Detail::ParseSafe(Json, ModCollection.ProfileName, "name");
		Detail::ParseSafe(Json, ModCollection.ProfileNameId, "name_id");
		Detail::ParseSafe(Json, ModCollection.ProfileSummary, "summary");
		Detail::ParseSafe(Json, ModCollection.ProfileDescription, "description");
	}
} // namespace Modio