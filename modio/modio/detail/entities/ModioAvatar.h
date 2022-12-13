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
#include <string>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Structure with the file references to the user's avatar
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

			/// @docnone
			friend bool operator==(const Modio::Detail::Avatar& A, const Modio::Detail::Avatar& B)
			{
				return (A.Filename == B.Filename && A.Original == B.Original && A.Thumb50x50 == B.Thumb50x50 &&
						A.Thumb100x100 == B.Thumb100x100);
			}
		};

		/// @docnone
		MODIO_IMPL void from_json(const nlohmann::json& Json, Avatar& Avatar);

		/// @docnone
		MODIO_IMPL void to_json(nlohmann::json& Json, const Avatar& Avatar);

		/// @docpublic
		/// @brief Retrieve the corresponding string according to an avatar size
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

		/// @docpublic
		/// @brief Transform an AvatarSize to an std::string 
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

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/entities/ModioAvatar.ipp"
#endif