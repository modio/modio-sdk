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

#include "ModioGeneratedVariables.h"

#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Metadata about a group of tags that can be used for filtering mods
	struct ModTagInfo
	{
		/// @brief The display name for the tag
		std::string TagGroupName;

		/// @brief The valid tags the group can have
		std::vector<std::string> TagGroupValues;

		/// @brief True if multiple tags from the group can be used simultaneously
		bool bAllowMultipleSelection;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModTagInfo& TagInfo);
	};

	/// @docpublic
	/// @brief Container for a collection of ModTagInfo objects
	class ModTagOptions : public PagedResult, public List<std::vector, ModTagInfo>
	{
		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModTagOptions& Options);
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioModTagOptions.ipp"
#endif
