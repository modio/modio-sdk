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

#include "modio/core/entities/ModioModCommunityOptions.h"

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::ModCommunityOptionsFlags& ModCommunity)
	{
		// We marshal to a uint64_t initially to protect against an overflow
		std::uint64_t modCommunity = 0;
		if (Json.is_number_integer())
		{
			using nlohmann::from_json;
			from_json(Json, modCommunity);
		}
		else
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::Core,
				"ModCommunity from_json requires an integer. Using default ModCommunityOptions 0");
		}
		ModCommunity = Modio::ModCommunityOptionsFlags::StorageType(modCommunity);
	}

	inline void to_json(nlohmann::json& Json, const Modio::ModCommunityOptionsFlags& ModCommunity)
	{
		// In case the Value inside ModCommunity is Optional::None, then it matches ModCommunityOptions::None
		Json = ModCommunity.RawValue();
	}
} // namespace Modio
