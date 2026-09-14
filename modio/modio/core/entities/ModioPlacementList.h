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
#include "modio/core/entities/ModioPlacement.h"
#include "modio/core/entities/ModioPagedResult.h"

namespace Modio
{
	/// @docpublic
	/// @brief Class representing a list of placements that may be a page from a larger set of results
	class PlacementList : public Modio::PagedResult, public Modio::List<std::vector, Modio::Placement>
	{
	public:
		/// @docpublic
		/// @brief Insert PlacementsList to the end of this list
		void Append(const Modio::PlacementList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}

		/// @docpublic
		/// @brief Insert a Placements to the end of this list
		void Append(const Modio::Placement& PlacementData)
		{
			InternalList.push_back(PlacementData);
		}
	};
} // namespace Modio