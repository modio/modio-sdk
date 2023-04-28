/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioGameStats.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	void from_json(const nlohmann::json& Json, Modio::GameStats& GameStats)
	{
		Detail::ParseSafe(Json, GameStats.GameID, "game_id");
		Detail::ParseSafe(Json, GameStats.ModCountTotal, "mods_count_total");
		Detail::ParseSafe(Json, GameStats.ModDownloadsToday, "mods_downloads_today");
		Detail::ParseSafe(Json, GameStats.ModDownloadsTotal, "mods_downloads_total");
		Detail::ParseSafe(Json, GameStats.ModDownloadsDailyAverage, "mods_downloads_daily_average");
		Detail::ParseSafe(Json, GameStats.ModSubscribersTotal, "mods_subscribers_total");
		Detail::ParseSafe(Json, GameStats.DateExpires, "date_expires");
	}
} // namespace Modio
