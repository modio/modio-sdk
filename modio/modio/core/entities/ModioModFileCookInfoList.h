/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/entities/ModioList.h"
#include "modio/core/entities/ModioModFileCookInfo.h"
#include "modio/core/entities/ModioPagedResult.h"

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of mod file cook objects that may be a page from a larger set of results
	class ModFileCookInfoList : public Modio::PagedResult, public Modio::List<std::vector, Modio::ModFileCookInfo>
	{
	public:
		/// @docpublic
		/// @brief Insert ModFileCookInfoList to the end of this list
		void Append(const Modio::ModFileCookInfoList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a ModFileCookInfo to the end of this list
		void Append(const Modio::ModFileCookInfo& ModFileCookInfo)
		{
			InternalList.push_back(ModFileCookInfo);
		}
	};
} // namespace Modio
