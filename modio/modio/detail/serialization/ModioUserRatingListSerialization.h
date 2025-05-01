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

#include "modio/core/entities/ModioUserRatingList.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::UserRating& OutUserRating)
	{
		Detail::ParseSafe(Json, OutUserRating.GameId, "game_id");
		Detail::ParseSafe(Json, OutUserRating.ModId, "mod_id");
		Detail::ParseSafe(Json, OutUserRating.ModRating, "rating");
	}

	inline void from_json(const nlohmann::json& Json, Modio::UserRatingList& OutUserRatingList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutUserRatingList));

		Detail::ParseSafe(Json, OutUserRatingList.InternalList, "data");
	}
} // namespace Modio