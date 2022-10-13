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
	#include "modio/detail/entities/ModioLogo.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		void from_json(const nlohmann::json& Json, Modio::Detail::Logo& ModLogo	)
		{
			Detail::ParseSafe(Json, ModLogo.Filename, "filename");
			Detail::ParseSafe(Json, ModLogo.Original, "original");
			Detail::ParseSafe(Json, ModLogo.Thumb320x180, "thumb_320x180");
			Detail::ParseSafe(Json, ModLogo.Thumb640x360, "thumb_640x360");
			Detail::ParseSafe(Json, ModLogo.Thumb1280x720, "thumb_1280x720");
		}

		void to_json(nlohmann::json& Json, const Modio::Detail::Logo& ModLogo)
		{
			Json = {{"filename", ModLogo.Filename},
					{"original", ModLogo.Original},
					{"thumb_320x180", ModLogo.Thumb320x180},
					{"thumb_640x360", ModLogo.Thumb640x360},
					{"thumb_1280x720", ModLogo.Thumb1280x720}};
		}
	} // namespace Detail
} // namespace Modio