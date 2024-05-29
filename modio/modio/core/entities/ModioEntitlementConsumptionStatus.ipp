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
	#include "modio/core/entities/ModioEntitlementConsumptionStatus.h"
#endif

#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, EntitlementConsumptionVirtualCurrencyDetails& Details)
	{
		Detail::ParseSafe(Json, Details.TokensAllocated, "tokens_allocated");
	}

	void to_json(nlohmann::json& Json, const EntitlementConsumptionVirtualCurrencyDetails& Details)
	{
		Json = nlohmann::json {{"tokens_allocated", Details.TokensAllocated}};
	}

	/// @docnone
	void from_json(const nlohmann::json& Json, Modio::EntitlementConsumptionStatus& EntitlementTransaction)
	{
		Detail::ParseSafe(Json, EntitlementTransaction.TransactionId, "transaction_id");
		Detail::ParseSafe(Json, EntitlementTransaction.TransactionState, "transaction_state");
		Detail::ParseSafe(Json, EntitlementTransaction.SkuId, "sku_id");
		Detail::ParseSafe(Json, EntitlementTransaction.EntitlementConsumed, "entitlement_consumed");
		Detail::ParseSafe(Json, EntitlementTransaction.EntitlementType, "entitlement_type");

		Detail::ParseSafe(Json, EntitlementTransaction.VirtualCurrencyDetails, "details");
	}

	/// @docnone
	void to_json(nlohmann::json& Json, const Modio::EntitlementConsumptionStatus& EntitlementTransaction)
	{
		Json = nlohmann::json {
			{"transaction_id", EntitlementTransaction.TransactionId},
			{"transaction_state", EntitlementTransaction.TransactionState},
			{"sku_id", EntitlementTransaction.SkuId},
			{"entitlement_consumed", EntitlementTransaction.EntitlementConsumed},
		};
	}
} // namespace Modio