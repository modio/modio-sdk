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

#include "modio/core/entities/ModioEntitlementConsumptionStatusList.h"
#include "modio/detail/serialization/ModioPagedResultSerialization.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::EntitlementWalletBalance& WalletBalance)
	{
		Detail::ParseSafe(Json, WalletBalance.Balance, "balance");
	}

	inline void from_json(const nlohmann::json& Json,
				   Modio::EntitlementConsumptionStatusList& OutEntitlementConsumptionStatusList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutEntitlementConsumptionStatusList));

		Detail::ParseSafe(Json, OutEntitlementConsumptionStatusList.InternalList, "data");

		if (OutEntitlementConsumptionStatusList.InternalList.empty())
		{
			return;
		}

		Modio::EntitlementWalletBalance WalletBalance;
		if (Detail::ParseSafe(Json, WalletBalance, "wallet"))
		{
			OutEntitlementConsumptionStatusList.WalletBalance = WalletBalance;	
		}
	}
} // namespace Modio