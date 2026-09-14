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

#include "modio/core/entities/ModioPlacementList.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"
#include "modio/detail/serialization/ModioPlacementSerialization.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::PlacementList& OutPlacementList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutPlacementList));

		Detail::ParseSafe(Json, OutPlacementList.GetRawList(), "data");
	}
} // namespace Modio