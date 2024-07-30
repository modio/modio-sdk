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
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/JsonWrapper.h"
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Structure with the file references to the logos of a mod
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
			std::string Thumb1280x720;

			/// @docnone
			friend bool operator==(const Modio::Detail::Logo& A, const Modio::Detail::Logo& B)
			{
				if ((A.Filename == B.Filename) && (A.Original == B.Original) && (A.Thumb320x180 == B.Thumb320x180) &&
					(A.Thumb640x360 == B.Thumb640x360) && (A.Thumb1280x720 == B.Thumb1280x720))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			/// @docnone
			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::Logo& ModLogo);

			/// @docnone
			MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::Detail::Logo& ModLogo);
		};

		/// @docpublic
		/// @brief Retrieve the corresponding string according to a logo size
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
					return Logo.Thumb1280x720;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult("");
			return NoResult;
		}

		/// @docpublic
		/// @brief Transform a LogoSize to an std::string
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
