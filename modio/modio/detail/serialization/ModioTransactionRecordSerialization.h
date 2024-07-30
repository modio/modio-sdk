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

#include "modio/core/entities/ModioTransactionRecord.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::TransactionRecord& Transaction)
	{
		Modio::Detail::ParseSafe(Json, Transaction.Price, Modio::Detail::Constants::JSONKeys::GrossAmount);
		Modio::Detail::ParseSafe(Json, Transaction.UpdatedUserWalletBalance,
								 Modio::Detail::Constants::JSONKeys::WalletBalance);

		Modio::Detail::ParseSafe(Json, Transaction.Mod, "mod");
	}

} // namespace Modio
