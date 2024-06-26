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
#include "modio/detail/ModioDefines.h"

#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/JsonWrapper.h"
#include <vector>

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of mods that may be a page from a larger set of results
	class ModInfoList : public PagedResult, public List<std::vector, Modio::ModInfo>
	{
	public:
		/// @docpublic
		/// @brief Insert ModInfoList to the end of this list
		void Append(const ModInfoList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModInfo to the end of this list
		void Append(const ModInfo& ModInfoData)
		{
			InternalList.push_back(ModInfoData);
		}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModInfoList& OutModInfoList);
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioModInfoList.ipp"
#endif