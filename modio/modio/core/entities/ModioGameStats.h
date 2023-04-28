/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioSplitCompilation.h"
#include "modio/detail/JsonWrapper.h"

namespace Modio
{
	/// @docpublic
	/// @brief Contains download stats for a game
	struct GameStats
	{
		/// @brief Unique game id
		Modio::GameID GameID = Modio::GameID::InvalidGameID();
		/// @brief Available mod count for the game
		std::int64_t ModCountTotal = 0;
		/// @brief Mods downloaded today for the game
		std::int64_t ModDownloadsToday = 0;
		/// @brief Total Mods downloaded for the game
		std::int64_t ModDownloadsTotal = 0;
		/// @brief Average mods downloaded on a daily basis
		std::int64_t ModDownloadsDailyAverage = 0;
		/// @brief Number of total users who have subscribed to the mods for the game
		std::int64_t ModSubscribersTotal = 0;
		/// @brief Unix timestamp until this game's statistics are considered stale
		std::int64_t DateExpires = 0;

		/// @docnone
		friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::GameStats& GameStats);
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioGameStats.ipp"
#endif
