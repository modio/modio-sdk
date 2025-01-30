/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
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
		/// @brief Structure with the file references to the icon of a game
		struct Icon
		{
			/** Icon filename including extension. */
			std::string Filename;
			/** URL to the full-sized icon. */
			std::string Original;
			/** URL to the small icon thumbnail. */
			std::string Thumb64x64;
			/** URL to the medium icon thumbnail. */
			std::string Thumb128x128;
			/** URL to the large icon thumbnail. */
			std::string Thumb256x256;

			/// @docnone
			friend bool operator==(const Modio::Detail::Icon& A, const Modio::Detail::Icon& B)
			{
				if ((A.Filename == B.Filename) && (A.Original == B.Original) && (A.Thumb64x64 == B.Thumb64x64) &&
					(A.Thumb128x128 == B.Thumb128x128) && (A.Thumb256x256 == B.Thumb256x256))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::Detail::Icon& Icon);
		};

		/// @docpublic
		/// @brief Retrieve the corresponding string according to a icon size
		inline const std::string& GetIconURL(const Icon& Icon, Modio::IconSize Size)
		{
			switch (Size)
			{
				case Modio::IconSize::Original:
					return Icon.Original;
				case Modio::IconSize::Thumb64:
					return Icon.Thumb64x64;
				case Modio::IconSize::Thumb128:
					return Icon.Thumb128x128;
				case Modio::IconSize::Thumb256:
					return Icon.Thumb256x256;
			}

			// Should never reach this
			assert(false);
			static std::string NoResult;
			return NoResult;
		}

		/// @docpublic
		/// @brief Transform a IconSize to an std::string
		inline std::string ToString(Modio::IconSize IconSize)
		{
			switch (IconSize)
			{
				case Modio::IconSize::Original:
					return "Original";
				case Modio::IconSize::Thumb64:
					return "Thumb64";
				case Modio::IconSize::Thumb128:
					return "Thumb128";
				case Modio::IconSize::Thumb256:
					return "Thumb256";
			}

			assert(false && "Invalid value to ToString(Modio::IconSize)");
			return "Unknown";
		}
	} // namespace Detail
} // namespace Modio
