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
	#include "modio/detail/entities/ModioImage.h"
#endif

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		void from_json(const nlohmann::json& Json, Modio::Detail::Image& Image)
		{
			Detail::ParseSafe(Json, Image.Filename, "filename");
			Detail::ParseSafe(Json, Image.Original, "original");
			Detail::ParseSafe(Json, Image.Thumb320x180, "thumb_320x180");
			Detail::ParseSafe(Json, Image.Thumb1280x720, "thumb_1280x720");
		}

		void to_json(nlohmann::json& Json, const Modio::Detail::Image& Image)
		{
			Json = nlohmann::json {{"filename", Image.Filename},
								   {"original", Image.Original},
								   {"thumb_320x180", Image.Thumb320x180},
									{"thumb_1280x720", Image.Thumb1280x720}};
		}
	} // namespace Detail
} // namespace Modio