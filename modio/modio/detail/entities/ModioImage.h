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

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ModioJsonHelpers.h"
#include <string>

namespace Modio
{
	namespace Detail
	{
		struct Image
		{
			/** Image filename including extension. */
			std::string Filename;
			/** URL to the full-sized image. */
			std::string Original;
			/** URL to the image thumbnail (320x180) */
			std::string Thumb320x180;

			friend bool operator==(const Modio::Detail::Image& A, const Modio::Detail::Image& B)
			{
				return (A.Filename == B.Filename && A.Original == B.Original && A.Thumb320x180 == B.Thumb320x180);
			}
		};

		static MODIO_IMPL void from_json(const nlohmann::json& Json, Image& Image)
		{
			Detail::ParseSafe(Json, Image.Filename, "filename");
			Detail::ParseSafe(Json, Image.Original, "original");
			Detail::ParseSafe(Json, Image.Thumb320x180, "thumb_320x180");
		}

		static MODIO_IMPL void to_json(nlohmann::json& Json, const Image& Image)
		{
			Json = nlohmann::json {{"filename", Image.Filename},
								   {"original", Image.Original},
								   {"thumb_320x180", Image.Thumb320x180}};
		}

		static const std::string& GetImmageURL(const Image& Image, Modio::GallerySize Size)
		{
			switch (Size)
			{
				case Modio::GallerySize::Original:
					return Image.Original;
				case Modio::GallerySize::Thumb320:
					return Image.Thumb320x180;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult("");
			return NoResult;
		}

		inline std::string ToString(Modio::GallerySize Size)
		{
			switch (Size)
			{
				case Modio::GallerySize::Original:
					return "Original";
				case Modio::GallerySize::Thumb320:
					return "Thumb320";
			}

			assert(false && "Invalid value to ToString(Modio::AvatarSize)");
			return "Unknown";
		}

	} // namespace Detail
} // namespace Modio
