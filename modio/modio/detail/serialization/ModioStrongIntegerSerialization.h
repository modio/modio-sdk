/*
 *  Copyright (C) 2021-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/JsonWrapper.h"
#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	/// @docnone
	template<typename UnderlyingIntegerType>
	inline void from_json(const nlohmann::json& Json, Modio::StrongInteger<UnderlyingIntegerType>& Integer)
	{
		using nlohmann::from_json;
		UnderlyingIntegerType RawValue;
		from_json(Json, RawValue);
		Integer = Modio::StrongInteger<UnderlyingIntegerType>(RawValue);
	}

	/// @docnone
	template<typename UnderlyingIntegerType>
	inline void to_json(nlohmann::json& Json, const Modio::StrongInteger<UnderlyingIntegerType>& Integer)
	{
		using nlohmann::to_json;
		UnderlyingIntegerType RawValue = Integer;
		to_json(Json, RawValue);
	}
} // namespace Modio
