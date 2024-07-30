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
		/// @brief Structure with the file references to the images of a mod
		struct Image
		{
			/** Image filename including extension. */
			std::string Filename;
			/** URL to the full-sized image. */
			std::string Original;
			/** URL to the image thumbnail (320x180) */
			std::string Thumb320x180;
			/** URL to the image thumbnail (1280x720) */
			std::string Thumb1280x720;

			/// @docnone
			friend bool operator==(const Modio::Detail::Image& A, const Modio::Detail::Image& B)
			{
				return (A.Filename == B.Filename && A.Original == B.Original && A.Thumb320x180 == B.Thumb320x180);
			}

			/// @docnone
			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Image& Image);

			/// @docnone
			MODIO_IMPL friend void to_json(nlohmann::json& Json, const Image& Image);
		};

		// GetImmageURL() creates false "unused functions" warnings on certain platforms.
		// Suppressing those warnings here.
		MODIO_DIAGNOSTIC_PUSH
		MODIO_ALLOW_UNUSED_FUNCTIONS

		/// @docinternal
		/// @brief Retrieve the corresponding string according to a GallerySize size
		static const std::string& GetImmageURL(const Image& Image, Modio::GallerySize Size)
		{
			switch (Size)
			{
				case Modio::GallerySize::Original:
					return Image.Original;
				case Modio::GallerySize::Thumb320:
					return Image.Thumb320x180;
				case Modio::GallerySize::Thumb1280:
					return Image.Thumb1280x720;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult("");
			return NoResult;
		}

		// Re-allow "unused function" warnings
		MODIO_DIAGNOSTIC_POP

		/// @docinternal
		/// @brief Transform an GallerySize to an std::string 
		inline std::string ToString(Modio::GallerySize Size)
		{
			switch (Size)
			{
				case Modio::GallerySize::Original:
					return "Original";
				case Modio::GallerySize::Thumb320:
					return "Thumb320";
				case Modio::GallerySize::Thumb1280:
					return "Thumb1280";
			}

			assert(false && "Invalid value to ToString(Modio::AvatarSize)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio
