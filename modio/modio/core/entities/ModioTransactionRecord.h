/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioModInfo.h"

namespace Modio
{
	/// @docpublic
	/// @brief Contains information about a successful transaction for a mod
	struct TransactionRecord
	{
		/// @brief The mod the purchase was for
		Modio::ModID AssociatedModID {};
		/// @brief The price the mod was purchased for
		std::uint64_t Price = 0;
		/// @brief The updated balance in the user's wallet after the transaction
		std::uint64_t UpdatedUserWalletBalance = 0;
		/// @brief ModInfo of the mod that was purchase and subscribed to
		Modio::ModInfo Mod {};

	};
}
