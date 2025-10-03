/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioEntitlement.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::Entitlement& OutEntitlement)
	{
		Modio::Detail::ParseSafe(Json, OutEntitlement.SkuId, "sku_id");
		Modio::Detail::ParseSafe(Json, OutEntitlement.Type, "entitlement_type");
	}
	inline void from_json(const nlohmann::json& Json, Modio::EntitlementList& OutEntitlementList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutEntitlementList));
		Modio::Detail::ParseSafe(Json, OutEntitlementList.InternalList, "data");
	}
} // namespace Modio