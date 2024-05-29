/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioModStats.h"
#include "modio/detail/JsonWrapper.h"
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief State of an entitlement consumption transaction
	enum class EntitlementConsumptionState
	{
		Failed = 0,
		Pending = 1, 
		Fulfilled = 2,
		ConsumeLimitExceeded = 3
	};

	/// @docpublic
	///	@brief Type of entitlement that was consumed
	enum class EntitlementType
	{
		VirtualCurrency = 0
	};

	/// @docpublic
	///	@brief Further details about a Virtual Currency entitlement that was consumed
	struct EntitlementConsumptionVirtualCurrencyDetails
	{
		/// @brief Amount of tokens that were issued for this specific entitlement consumption
		int TokensAllocated = 0;
	};

	/// @docpublic
	/// @brief The result of an entitlement's consumption
	struct EntitlementConsumptionStatus
	{
		/// @brief ID of the transaction to redeem this entitlement
		std::string TransactionId = "";

		/// @brief State of the transaction
		Modio::EntitlementConsumptionState TransactionState = Modio::EntitlementConsumptionState::Failed;

		/// @brief ID of the SKU that we attempted to consume
		std::string SkuId = "";

		/// @brief Whether this entitlement was consumed or not
		bool EntitlementConsumed;

		/// @brief Type of Entitlement that was consumed
		Modio::EntitlementType EntitlementType = Modio::EntitlementType::VirtualCurrency;

		/// @brief Details about 
		Modio::EntitlementConsumptionVirtualCurrencyDetails VirtualCurrencyDetails;
	};

	/// @docnone
	MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::EntitlementConsumptionStatus& EntitlementTransaction);

	/// @docnone
	MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::EntitlementConsumptionStatus& EntitlementTransaction);

	/// @docnone
	MODIO_IMPL void from_json(const nlohmann::json& Json, EntitlementConsumptionVirtualCurrencyDetails& Details);

	/// @docnone
	MODIO_IMPL void to_json(nlohmann::json& Json, const EntitlementConsumptionVirtualCurrencyDetails& Details);
}

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioEntitlementConsumptionStatus.ipp"
#endif
