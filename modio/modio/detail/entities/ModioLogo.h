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
		struct Logo
		{
			/** Logo filename including extension. */
			std::string Filename;
			/*URL to the full - sized logo. */
			std::string Original;
			/** URL to the small logo thumbnail. */
			std::string Thumb320x180;
			/** URL to the medium logo thumbnail. */
			std::string Thumb640x360;
			/** URL to the large logo thumbnail. */
			std::string Thumb1024x720;

			friend bool operator==(const Modio::Detail::Logo& A, const Modio::Detail::Logo& B)
			{
				if ((A.Filename == B.Filename) && (A.Original == B.Original) && (A.Thumb320x180 == B.Thumb320x180) &&
					(A.Thumb640x360 == B.Thumb640x360) && (A.Thumb1024x720 == B.Thumb1024x720))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		};

		static void from_json(const nlohmann::json& Json, Logo& Avatar)
		{
			Detail::ParseSafe(Json, Avatar.Filename, "filename");
			Detail::ParseSafe(Json, Avatar.Original, "original");
			Detail::ParseSafe(Json, Avatar.Thumb320x180, "thumb_320x180");
			Detail::ParseSafe(Json, Avatar.Thumb640x360, "thumb_640x360");
			Detail::ParseSafe(Json, Avatar.Thumb1024x720, "thumb_1280x720");
		}

		static void to_json(nlohmann::json& Json, const Modio::Detail::Logo& ModLogo)
		{
			Json = {{"filename", ModLogo.Filename},
					{"original", ModLogo.Original},
					{"thumb_320x180", ModLogo.Thumb320x180},
					{"thumb_640x360", ModLogo.Thumb640x360},
					{"thumb_1280x720", ModLogo.Thumb1024x720}};
		}

			

		inline const std::string& GetLogoURL(const Logo& Logo, Modio::LogoSize Size)
		{
			switch (Size)
			{
				case Modio::LogoSize::Original:
					return Logo.Original;
				case Modio::LogoSize::Thumb320:
					return Logo.Thumb320x180;
				case Modio::LogoSize::Thumb640:
					return Logo.Thumb640x360;
				case Modio::LogoSize::Thumb1280:
					return Logo.Thumb1024x720;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult("");
			return NoResult;
		}

		inline std::string ToString(Modio::LogoSize LogoSize)
		{
			switch (LogoSize)
			{
				case Modio::LogoSize::Original:
					return "Original";
				case Modio::LogoSize::Thumb320:
					return "Thumb320";
				case Modio::LogoSize::Thumb640:
					return "Thumb640";
				case Modio::LogoSize::Thumb1280:
					return "Thumb1280";
			}

			assert(false && "Invalid value to ToString(Modio::LogoSize)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio
