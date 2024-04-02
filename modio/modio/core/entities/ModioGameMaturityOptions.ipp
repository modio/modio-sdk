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
	#include "modio/core/entities/ModioGameMaturityOptions.h"
#endif

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::GameMaturityOptionsFlags& GameMaturity)
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
		GameMaturity.Value = static_cast<Modio::GameMaturityOptionsFlags::StorageType>(gameMaturity);
	}

	void to_json(nlohmann::json& Json, const Modio::GameMaturityOptionsFlags& GameMaturity)
	{
		// In case the Value inside GameMaturity is Optional::None, then it matches GameMaturity::None
		std::uint32_t DefaultGameMaturity = static_cast<uint32_t>(GameMaturityOptions::None);
		Json = GameMaturity.Value.value_or(DefaultGameMaturity);
	}
} // namespace Modio