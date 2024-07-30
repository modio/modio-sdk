/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/entities/ModioAvatar.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		inline void from_json(const nlohmann::json& Json, Modio::Detail::Avatar& Avatar)
		{
			Detail::ParseSafe(Json, Avatar.Filename, "filename");
			Detail::ParseSafe(Json, Avatar.Original, "original");
			Detail::ParseSafe(Json, Avatar.Thumb50x50, "thumb_50x50");
			Detail::ParseSafe(Json, Avatar.Thumb100x100, "thumb_100x100");
		}

		inline void to_json(nlohmann::json& Json, const Modio::Detail::Avatar& Avatar)
		{
			Json = nlohmann::json {{"filename", Avatar.Filename},
								   {"original", Avatar.Original},
								   {"thumb_50x50", Avatar.Thumb50x50},
								   {"thumb_100x100", Avatar.Thumb100x100}};
		}
	} // namespace Detail
} // namespace Modio