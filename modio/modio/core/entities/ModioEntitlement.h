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

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <string>

namespace Modio
{
	struct Entitlement
	{
		std::string SkuId;
		Modio::EntitlementType Type;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Entitlement& OutEntitlement);
	};

	class EntitlementList : public Modio::PagedResult, public Modio::List<std::vector, Modio::Entitlement>
	{
	public:
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, EntitlementList& OutEntitlementList);
	};
} // namespace Modio