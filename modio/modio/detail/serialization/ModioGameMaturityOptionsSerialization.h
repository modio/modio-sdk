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

#include "modio/core/entities/ModioGameMaturityOptions.h"

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::GameMaturityOptionsFlags& GameMaturity)
	{
		// We marshal to a uint64_t initially to protect against an overflow
		std::uint64_t gameMaturity = 0;
		if (Json.is_number_integer())
		{
			using nlohmann::from_json;
			from_json(Json, gameMaturity);
		}
		else
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::Core,
				"GameMaturity from_json requires an integer.  Using default GameMaturityOptions 0");
		}
		GameMaturity = static_cast<Modio::GameMaturityOptionsFlags::StorageType>(gameMaturity);
	}

	inline void to_json(nlohmann::json& Json, const Modio::GameMaturityOptionsFlags& GameMaturity)
	{
		Json = GameMaturity.RawValue();
	}
} // namespace Modio
