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

#include "modio/core/entities/ModioGameMonetization.h"

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::GameMonetization& GameMonetization)
	{
		Modio::GameMonetization::StorageType monetization = 0;
		if (Json.is_number_integer())
		{
			using nlohmann::from_json;
			from_json(Json, monetization);
		}
		else
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::Core,
				"GameMonetization from_json requires an integer.  Using default GameMonetizationOptions 0");
		}
		GameMonetization = monetization;
	}

	inline void to_json(nlohmann::json& Json, const Modio::GameMonetization& GameMonetization)
	{
		Json = GameMonetization.RawValue();
	}
} // namespace Modio
