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
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioEntitlementConsumptionStatus.h"
#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include "modio/detail/ModioDefines.h"
#include <vector>

namespace Modio
{
	/// @docpublic
	///	@brief Updated wallet balance from the sync entitlements call
	struct EntitlementWalletBalance
	{
		uint64_t Balance = 0;
	};

	/// @docpublic
	/// @brief Class representing a list of mods that may be a page from a larger set of results
	class EntitlementConsumptionStatusList : public PagedResult,
											 public List<std::vector, Modio::EntitlementConsumptionStatus>
	{
	public:
		/// @docpublic
		/// @brief Insert ModioEntitlementConsumptionStatusList to the end of this list
		void Append(const EntitlementConsumptionStatusList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModioEntitlementConsumptionStatus to the end of this list
		void Append(const EntitlementConsumptionStatus& EntitlementConsumptionStatusData)
		{
			InternalList.push_back(EntitlementConsumptionStatusData);
		}

		/// @docpublic
		/// @brief Updated wallet balance from syncing entitlements
		Modio::Optional<Modio::EntitlementWalletBalance> WalletBalance;

		/// @docpublic
		/// @brief Filter elements that require a second request to confirm the entitlement
		/// @return Optional list with the elements that need a retry, otherwise an empty object.
		MODIO_IMPL Modio::Optional<EntitlementConsumptionStatusList> EntitlementsThatRequireRetry() const;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json,
										 EntitlementConsumptionStatusList& OutEntitlementConsumptionStatusList);

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, EntitlementWalletBalance& WalletBalance);
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioEntitlementConsumptionStatusList.ipp"
#endif
