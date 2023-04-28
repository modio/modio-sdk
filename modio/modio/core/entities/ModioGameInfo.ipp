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
	#include "modio/core/entities/ModioGameInfo.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	/// @docnone
	void from_json(const nlohmann::json& Json, Modio::HeaderImage& HeaderImage)
	{
		Detail::ParseSafe(Json, HeaderImage.Filename, "filename");
		Detail::ParseSafe(Json, HeaderImage.Original, "original");
	}

	/// @docnone
	void from_json(const nlohmann::json& Json, Modio::OtherUrl& OtherUrl)
	{
		Detail::ParseSafe(Json, OtherUrl.Label, "label");
		Detail::ParseSafe(Json, OtherUrl.Url, "url");
	}

	/// @docnone
	void from_json(const nlohmann::json& Json, Modio::Theme& Theme)
	{
		Detail::ParseSafe(Json, Theme.Primary, "primary");
		Detail::ParseSafe(Json, Theme.Dark, "dark");
		Detail::ParseSafe(Json, Theme.Light, "light");
		Detail::ParseSafe(Json, Theme.Success, "success");
		Detail::ParseSafe(Json, Theme.Warning, "warning");
		Detail::ParseSafe(Json, Theme.Danger, "danger");
	}

	/// @docnone
	void from_json(const nlohmann::json& Json, Modio::GameInfo& GameInfo)
	{
		Detail::ParseSafe(Json, GameInfo.GameID, "id");
		Detail::ParseSafe(Json, GameInfo.DateAdded, "date_added");
		Detail::ParseSafe(Json, GameInfo.DateUpdated, "date_updated");
		Detail::ParseSafe(Json, GameInfo.DateLive, "date_live");
		Detail::ParseSafe(Json, GameInfo.UgcName, "ugc_name");
		Detail::ParseSafe(Json, GameInfo.Name, "name");
		Detail::ParseSafe(Json, GameInfo.Summary, "summary");
		Detail::ParseSafe(Json, GameInfo.Instructions, "instructions");
		Detail::ParseSafe(Json, GameInfo.InstructionsUrl, "instructions_url");
		Detail::ParseSafe(Json, GameInfo.ProfileUrl, "profile_url");
		Detail::ParseSafe(Json, GameInfo.Logo, "logo");
		Detail::ParseSafe(Json, GameInfo.Icon, "icon");
		Detail::ParseSafe(Json, GameInfo.HeaderImage, "header");
		Detail::ParseSafe(Json, GameInfo.Stats, "stats");
		Detail::ParseSafe(Json, GameInfo.Theme, "theme");
		Detail::ParseSafe(Json, GameInfo.OtherUrls, "other_urls");
	}
} // namespace Modio