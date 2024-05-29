/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioEntitlementConsumptionStatusList.h"
#endif

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::EntitlementWalletBalance& WalletBalance)
	{
		Detail::ParseSafe(Json, WalletBalance.Balance, "balance");	
	}

	void from_json(const nlohmann::json& Json,
				   Modio::EntitlementConsumptionStatusList& OutEntitlementConsumptionStatusList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutEntitlementConsumptionStatusList));

		Detail::ParseSafe(Json, OutEntitlementConsumptionStatusList.InternalList, "data");

		Modio::EntitlementWalletBalance WalletBalance;
		if (Detail::ParseSafe(Json, WalletBalance, "wallet"))
		{
			OutEntitlementConsumptionStatusList.WalletBalance = WalletBalance;
		}
	}
} // namespace Modio