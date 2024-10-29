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

	Modio::Optional<Modio::EntitlementConsumptionStatusList> EntitlementConsumptionStatusList::
		EntitlementsThatRequireRetry() const
	{
		if (InternalList.empty())
		{
			return {};
		}

		EntitlementConsumptionStatusList RetryElements;
		for (EntitlementConsumptionStatus Entitlement : InternalList)
		{
			if (Entitlement.EntitlementRequiresRetry() == true)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::Http,
											"Entitlement transaction needs retry with SKU: {} & TransactionID: {}",
											Entitlement.SkuId, Entitlement.TransactionId);
				RetryElements.Append(Entitlement);
			}
		}

		return RetryElements;
	}

} // namespace Modio