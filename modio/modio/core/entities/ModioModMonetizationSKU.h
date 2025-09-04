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
#include "modio/core/ModioSplitCompilation.h"
#include "modio/detail/JsonWrapper.h"
#include <string>
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Represents a SKU for monetization 
	struct ModMonetizationSKU
	{
		/// @brief Unique identifier for the SKU
		uint8_t Id;

		/// @brief Name of the actual SKU pack
		std::string Sku;

		/// @brief Portal that this SKU is associated with
		std::string Portal;

		
		/// @docnone
		friend bool operator==(const Modio::ModMonetizationSKU& A, const Modio::ModMonetizationSKU& B)
		{
			return (A.Id == B.Id);
		}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModMonetizationSKU& SKU);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::ModMonetizationSKU& SKU);
	};

} // namespace Modio
