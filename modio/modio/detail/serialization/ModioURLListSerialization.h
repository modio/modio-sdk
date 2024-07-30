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

#include "modio/core/entities/ModioURLList.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::YoutubeURLList& YoutubeURLList)
	{
		Detail::ParseSafe(Json, YoutubeURLList.InternalList, "youtube");
	}

	inline void from_json(const nlohmann::json& Json, Modio::SketchfabURLList& SketchfabURLList)
	{
		Detail::ParseSafe(Json, SketchfabURLList.InternalList, "sketchfab");
	}

	inline void to_json(nlohmann::json& Json, const Modio::YoutubeURLList& YoutubeURLList)
	{
		Json = nlohmann::json {{"youtube", YoutubeURLList.GetRawList()}};
	}

	inline void to_json(nlohmann::json& Json, const Modio::SketchfabURLList& SketchfabURLList)
	{
		Json = nlohmann::json {{"sketchfab", SketchfabURLList.GetRawList()}};
	}
} // namespace Modio
