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
	#include "modio/core/entities/ModioGameMonetization.h"
#endif

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::GameMonetization& GameMonetization)
	{
		std::uint8_t monetization = 0;
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
		GameMonetization.Value = monetization;
	}

	void to_json(nlohmann::json& Json, const Modio::GameMonetization& GameMonetization)
	{
		// In case the Value inside GameMonetization is Optional::None, then it matches GameMonetizationOptions::None
		std::uint8_t DefaultGameMonetization = static_cast<uint8_t>(GameMonetizationOptions::None);
		Json = GameMonetization.Value.value_or(DefaultGameMonetization);
	}
} // namespace Modio