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

#include "modio/core/ModioFilterParams.h"
#include "modio/core/entities/ModioPlacement.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioStringHash.h"
#include "modio/detail/serialization/ModioFilterParamsSerialization.h"
#include <string_view>

namespace Modio
{
	NLOHMANN_JSON_SERIALIZE_ENUM(Modio::Placement::PlacementSize, {{Modio::Placement::PlacementSize::Large, "large"},
																   {Modio::Placement::PlacementSize::Medium, "medium"},
																   {Modio::Placement::PlacementSize::Small, "small"}});

	NLOHMANN_JSON_SERIALIZE_ENUM(Modio::Placement::PlacementType,
								 {{Modio::Placement::PlacementType::Filters, "filters"}});

	// FilterParams are never returned directly from the REST API so we are deserializing as part of the one place we
	// need to materialize one with the contents of a REST API response
	inline void from_json(const nlohmann::json& Json, Modio::FilterParams& Filters)
	{
		if (Modio::FilterParams::Timespan OutTimespan = FilterParams::Timespan::AllTime;
			Detail::ParseSafe(Json, OutTimespan, "timespan"))
		{
			Filters = Filters.WithinTimespan(OutTimespan);
		}

		if (std::string SortString; Detail::ParseSafe(Json, SortString, "_sort"))
		{
			const bool bDescending = !SortString.empty() && SortString.front() == '-';
			const char* SortFieldStringPtr = bDescending ? SortString.c_str() + 1 : SortString.c_str();
			Modio::FilterParams::SortFieldType SortField = FilterParams::SortFieldType::Alphabetical;
			using namespace Modio::Detail;
			switch (Modio::Detail::hash_64_fnv1a_const(SortFieldStringPtr))
			{
				case "date_live"_hash:
					SortField = Modio::FilterParams::SortFieldType::DateMarkedLive;
					break;
				case "date_updated"_hash:
					SortField = Modio::FilterParams::SortFieldType::DateUpdated;
					break;
				case "downloads_today"_hash:
					SortField = Modio::FilterParams::SortFieldType::DownloadsToday;
					break;
				case "ratings_weighted_aggregate"_hash:
					SortField = Modio::FilterParams::SortFieldType::Rating;
					break;
				case "subscribers_total"_hash:
					SortField = Modio::FilterParams::SortFieldType::SubscriberCount;
					break;
				case "id"_hash:
					SortField = Modio::FilterParams::SortFieldType::ID;
					break;
				case "downloads_total"_hash:
					SortField = Modio::FilterParams::SortFieldType::DownloadsTotal;
					break;
				case "name"_hash:
					SortField = Modio::FilterParams::SortFieldType::Alphabetical;
					break;

				default:
					break;
			}
			Filters = Filters.SortBy(SortField, bDescending ? Modio::FilterParams::SortDirection::Descending
															: Modio::FilterParams::SortDirection::Ascending);
		}

		if (Modio::FilterParams::RevenueFilterType RevType = FilterParams::RevenueFilterType::FreeAndPaid;
			Modio::Detail::ParseSafe(Json, RevType, "revenue_type"))
		{
			Filters = Filters.RevenueType(RevType);
		}
	}

	inline void from_json(const nlohmann::json& Json, Modio::Placement& Out)
	{
		Detail::ParseSafe(Json, Out.ID, "id");
		Detail::ParseSafe(Json, Out.Type, "type");
		Detail::ParseSafe(Json, Out.Name, "name");
		Detail::ParseSafe(Json, Out.NameLocalized, "name_localized");
		Detail::ParseSafe(Json, Out.DisplayPosition, "display_position");
		Detail::ParseSafe(Json, Out.Total, "total");
		Detail::ParseSafe(Json, Out.Size, "size");
		Detail::ParseSafe(Json, Out.bEnabled, "enabled");
		Detail::ParseSafe(Json, Out.Filters, "filters");
	}
} // namespace Modio
