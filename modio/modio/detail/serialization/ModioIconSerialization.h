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

#include "modio/detail/entities/ModioIcon.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		inline void from_json(const nlohmann::json& Json, Modio::Detail::Icon& Icon)
		{
			Detail::ParseSafe(Json, Icon.Filename, "filename");
			Detail::ParseSafe(Json, Icon.Original, "original");
			Detail::ParseSafe(Json, Icon.Thumb64x64, "thumb_64x64");
			Detail::ParseSafe(Json, Icon.Thumb128x128, "thumb_128x128");
			Detail::ParseSafe(Json, Icon.Thumb256x256, "thumb_256x256");
		}
	} // namespace Detail
} // namespace Modio