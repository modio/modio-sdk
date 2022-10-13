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
	#include "modio/core/entities/ModioProfileMaturity.h"
#endif

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::ProfileMaturity& ProfileMaturity)
	{
		std::uint8_t maturity = 0;
		if (Json.is_number_integer())
		{
			using nlohmann::from_json;
			from_json(Json, maturity);
		}
		else
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::Core,
				"ProfileMaturity from_json requires an integer.  Using default ProfileMaturity 0");
		}
		ProfileMaturity.Value = maturity;
	}
	void to_json(nlohmann::json& Json, const Modio::ProfileMaturity& ProfileMaturity)
	{
		// In case the Value inside ProfileMaturity is Optional::None, then it matches MaturityOption::None
		std::uint8_t DefaultMaturity = static_cast<uint8_t>(MaturityOption::None);
		Json = ProfileMaturity.Value.value_or(DefaultMaturity);
	}
} // namespace Modio