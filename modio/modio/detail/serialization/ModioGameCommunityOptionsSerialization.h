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

#include "modio/core/entities/ModioGameCommunityOptions.h"

#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::GameCommunityOptionsFlags& GameCommunity)
	{
		// We marshal to a uint64_t initially to protect against an overflow
		std::uint64_t gameCommunity = 0;
		if (Json.is_number_integer())
		{
			using nlohmann::from_json;
			from_json(Json, gameCommunity);
		}
		else
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::Core,
				"GameCommunity from_json requires an integer.  Using default GameCommunityOptions 0");
		}
		GameCommunity.Value = static_cast<Modio::GameCommunityOptionsFlags::StorageType>(gameCommunity);
	}

	inline void to_json(nlohmann::json& Json, const Modio::GameCommunityOptionsFlags& GameCommunity)
	{
		// In case the Value inside GameCommunity is Optional::None, then it matches GameCommunity::None
		std::uint32_t DefaultGameCommunity = static_cast<uint32_t>(GameCommunityOptions::None);
		Json = GameCommunity.Value.value_or(DefaultGameCommunity);
	}
} // namespace Modio
