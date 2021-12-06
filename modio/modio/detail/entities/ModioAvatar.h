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
		struct Avatar
		{
			/** Avatar filename including extension. */
			std::string Filename;
			/** URL to the full-sized avatar. */
			std::string Original;
			/** URL to the small avatar thumbnail. */
			std::string Thumb50x50;
			/** URL to the medium avatar thumbnail. */
			std::string Thumb100x100;

			friend bool operator==(const Modio::Detail::Avatar& A, const Modio::Detail::Avatar& B)
			{
				return (A.Filename == B.Filename && A.Original == B.Original && A.Thumb50x50 == B.Thumb50x50 && A.Thumb100x100 == B.Thumb100x100);
			}
		};

		static void from_json(const nlohmann::json& Json, Avatar& Avatar)
		{
			Detail::ParseSafe(Json, Avatar.Filename, "filename");
			Detail::ParseSafe(Json, Avatar.Original, "original");
			Detail::ParseSafe(Json, Avatar.Thumb50x50, "thumb_50x50");
			Detail::ParseSafe(Json, Avatar.Thumb100x100, "thumb_100x100");
		}

		static void to_json(nlohmann::json& Json, const Avatar& Avatar)
		{
			Json = nlohmann::json {{"filename", Avatar.Filename},
								   {"original", Avatar.Original},
								   {"thumb_50x50", Avatar.Thumb50x50},
								   {"thumb_100x100", Avatar.Thumb100x100}};
		}

		inline const std::string& GetAvatarURL(const Avatar& Avatar, Modio::AvatarSize Size)
		{
			switch (Size)
			{
				case Modio::AvatarSize::Original:
					return Avatar.Original;
				case Modio::AvatarSize::Thumb50:
					return Avatar.Thumb50x50;
				case Modio::AvatarSize::Thumb100:
					return Avatar.Thumb100x100;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult("");
			return NoResult;
		}

		inline std::string ToString(Modio::AvatarSize AvatarSize)
		{
			switch (AvatarSize)
			{
				case Modio::AvatarSize::Original:
					return "Original";
				case Modio::AvatarSize::Thumb50:
					return "Thumb50";
				case Modio::AvatarSize::Thumb100:
					return "Thumb100";
			}

			assert(false && "Invalid value to ToString(Modio::AvatarSize)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio
