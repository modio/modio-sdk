/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioModMonetizationSKU.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	/// @docnone
	inline void from_json(const nlohmann::json& Json, Modio::ModMonetizationSKU& SKU)
	{
		Modio::Detail::ParseSafe(Json, SKU.Id, "id");
		Modio::Detail::ParseSafe(Json, SKU.Sku, "sku");
		Modio::Detail::ParseSafe(Json, SKU.Portal, "portal");
	}

	/// @docnone
	inline void to_json(nlohmann::json& Json, const Modio::ModMonetizationSKU& InSKU)
	{
		Json = nlohmann::json {{"id", InSKU.Id},
								   {"sku", InSKU.Sku},
								   {"portal", InSKU.Portal}};
	}
} // namespace Modio
