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
	class CreateModFileParams
	{
	public:
		/// @docpublic
		/// @brief Path to a directory to treat as the root of the mod. All contents of this folder will be added to the
		/// archive, with a relative path based on this directory. This directory will not be created itself, the
		/// contents will exist at the top level of the archive.
		Modio::filesystem::path RootDirectory;

		/// @docpublic
		/// @brief Optional version string for this modfile release
		Modio::Optional<std::string> Version;

		/// @docpublic
		/// @brief Optional changelog string for this modfile release
		Modio::Optional<std::string> Changelog;

		/// @docpublic
		/// @brief Should this mod be set as active/latest release? defaults to true if not set
		Modio::Optional<bool> bSetAsActive;

		/// @docpublic
		/// @brief Optional metadata blob for this mod
		Modio::Optional<std::string> MetadataBlob;
	};
} // namespace Modio