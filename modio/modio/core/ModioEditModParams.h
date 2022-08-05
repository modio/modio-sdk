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
#include "modio/core/entities/ModioModInfo.h"

namespace Modio
{
	/// @docpublic
	/// @brief Class specifying fields to update for a mod
	class EditModParams
	{
	public:
		
		/// @docpublic
		/// @brief The name of the mod
		Modio::Optional<std::string> Name;
		
		/// @docpublic
		/// @brief A brief summary of what the mod is
		Modio::Optional<std::string> Summary;
		
		/// @docpublic
		/// @brief Optional override for the name 'slug' in the mod's URL
		Modio::Optional<std::string> NamePath;
		
		/// @docpublic
		/// @brief Optional override for the mod's visibility status. Defaults to true (visible)
		Modio::Optional<bool> bVisible;

		/// @docpublic
		/// @brief Optional long description of the mod
		Modio::Optional<std::string> Description;
		
		/// @docpublic
		/// @brief Optional URL to the mod's homepage. Must be a valid URL
		Modio::Optional<std::string> HomepageURL;
		
		/// @docpublic
		/// @brief Bitwise mask of flags indicating mature content
		Modio::Optional<Modio::MaturityOption> MaturityRating;
		
		/// @docpublic
		/// @brief Optional metadata blob for this mod
		Modio::Optional<std::string> MetadataBlob;
		
	};
} // namespace Modio