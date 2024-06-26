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

#include "modio/core/entities/ModioModDetails.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::ModDetails& ModDetails)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(ModDetails));

		Modio::Detail::ParseSafe(Json, ModDetails.InternalList, "data");
	}
} // namespace Modio